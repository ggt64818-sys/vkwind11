#include "sm4_translator.h"
#include "../common/logging.h"
#include <cstring>

// ============================================================================
// SPIR-V constants
// ============================================================================

static constexpr uint32_t CAPABILITY_SHADER = 1;
static constexpr uint32_t ADDRESS_MODEL_LOGICAL = 0;
static constexpr uint32_t MEMORY_MODEL_GLSL450 = 1;
static constexpr uint32_t EXECUTION_MODEL_VERTEX = 0;
static constexpr uint32_t EXECUTION_MODEL_FRAGMENT = 4;
static constexpr uint32_t STORAGE_CLASS_INPUT = 1;
static constexpr uint32_t STORAGE_CLASS_OUTPUT = 3;
static constexpr uint32_t STORAGE_CLASS_FUNCTION = 7;
static constexpr uint32_t STORAGE_CLASS_UNIFORM_CONSTANT = 0;
static constexpr uint32_t FUNCTION_CONTROL_NONE = 0;
static constexpr uint32_t DECORATION_LOCATION = 30;
static constexpr uint32_t DECORATION_BUILTIN = 11;
static constexpr uint32_t DECORATION_DESCRIPTOR_SET = 33;
static constexpr uint32_t DECORATION_BINDING = 34;
static constexpr uint32_t BUILTIN_POSITION = 0;
static constexpr uint32_t GLSL_STD450_FMin = 37;
static constexpr uint32_t GLSL_STD450_FMax = 39;
static constexpr uint32_t GLSL_STD450_FAbs = 4;
static constexpr uint32_t GLSL_STD450_FNegate = 127;
static constexpr uint32_t GLSL_STD450_Dot = 52;
static constexpr uint32_t GLSL_STD450_InverseSqrt = 32;
static constexpr uint32_t GLSL_STD450_FDiv = 41;

SM4Translator::SM4Translator() {
  memset(m_tempRegisters, 0, sizeof(m_tempRegisters));
}

// ============================================================================
// Operand decoding
// ============================================================================

uint32_t SM4Translator::peekToken() const {
  if (m_currentToken >= m_tokenCount) return 0;
  return m_tokens[m_currentToken];
}

uint32_t SM4Translator::nextToken() {
  if (m_currentToken >= m_tokenCount) return 0;
  return m_tokens[m_currentToken++];
}

uint32_t SM4Translator::getOperandType(uint32_t token) const {
  return (token >> 23) & 0x1F;
}

uint32_t SM4Translator::getOperandIndex(uint32_t token) const {
  return token & 0xFFFFF;
}

uint32_t SM4Translator::createId() {
  return m_nextId++;
}

// ============================================================================
// Main translation entry
// ============================================================================

SM4TranslateResult SM4Translator::translate(const ParsedDXBC& dxbc) {
  SM4TranslateResult result;

  if (!dxbc.valid) {
    result.errorMessage = "Invalid DXBC bytecode";
    return result;
  }

  m_isVertexShader = dxbc.isVertexShader;
  m_isPixelShader = dxbc.isPixelShader;

  if (!dxbc.shaderTokens || dxbc.shaderTokenCount == 0) {
    result.errorMessage = "No SHDR chunk found";
    return result;
  }

  m_tokens = dxbc.shaderTokens;
  m_tokenCount = dxbc.shaderTokenCount;
  m_currentToken = 0;

  m_spirv.clear();
  m_nextId = 1;

  // Reset all state
  memset(m_tempRegisters, 0, sizeof(m_tempRegisters));
  m_inputVars.clear();
  m_outputVars.clear();
  m_cbuffers.clear();
  m_textures.clear();

  // ── SPIR-V Header ──
  m_spirv.push_back(0x07230203); // magic
  m_spirv.push_back(0x00010000); // version 1.0
  m_spirv.push_back(0x80000000 | 0x00010200); // generator (bound for later)
  uint32_t boundPos = (uint32_t)m_spirv.size();
  m_spirv.push_back(0); // bound
  m_spirv.push_back(0); // schema

  emitCapability(CAPABILITY_SHADER);
  emitMemoryModel();
  emitExtInstImport();

  // ── Pass 1: Scan declarations ──
  scanDeclarations();

  // ── Emit types and decorations ──
  emitTypeDeclarations();
  emitDecorations();

  // ── Emit entry point ──
  m_mainFunction = createId();
  m_entryPoint = createId();

  // ── Pass 2: Translate instructions ──
  m_currentToken = 0;
  while (m_currentToken < m_tokenCount) {
    uint32_t token = nextToken();
    uint32_t opcode = token & 0xFF;
    uint32_t instLength = (token >> 8) & 0xFF;

    translateInstruction(opcode);
  }

  // Fill in bound
  m_spirv[boundPos] = m_nextId;

  result.spirvWords = m_spirv;
  result.entryPointName = "main";
  result.success = true;

  return result;
}

// ============================================================================
// Pass 1: Scan all DCL tokens to pre-allocate IDs
// ============================================================================

void SM4Translator::scanDeclarations() {
  uint32_t savedPos = m_currentToken;

  while (m_currentToken < m_tokenCount) {
    uint32_t token = m_tokens[m_currentToken];
    uint32_t opcode = token & 0xFF;
    uint32_t instLength = (token >> 8) & 0xFF;

    if (opcode == 0) break; // ENDOP

    switch (opcode) {
      case 60: { // DCL_CONSTANT_BUFFER
        uint32_t operand = nextToken();
        uint32_t idx = getOperandIndex(operand);
        uint32_t compCount = (operand >> 16) & 0x3 + 1; // 1-4 component count from bits 16-17
        // skip register token, advance past instruction
        nextToken();
        m_cbuffers[idx].slot = idx;
        m_cbuffers[idx].size = 0;
        m_cbuffers[idx].name = "cbuffer" + std::to_string(idx);
        break;
      }
      case 58: { // DCL_SAMPLER
        nextToken(); // operand
        break;
      }
      case 59: { // DCL_TEXTURE (resource)
        uint32_t operand = nextToken();
        uint32_t idx = getOperandIndex(operand);
        m_textures[idx].slot = idx;
        m_textures[idx].name = "tex" + std::to_string(idx);
        break;
      }
      case 63: { // DCL_INPUT
        uint32_t operand = nextToken();
        uint32_t idx = getOperandIndex(operand);
        m_inputVars[idx] = 0;
        break;
      }
      case 64: { // DCL_INPUT_PS
        uint32_t operand = nextToken();
        uint32_t idx = getOperandIndex(operand);
        m_inputVars[idx] = 0;
        break;
      }
      case 62: { // DCL_OUTPUT
        uint32_t operand = nextToken();
        uint32_t idx = getOperandIndex(operand);
        m_outputVars[idx] = 0;
        break;
      }
      case 61: { // DCL_OUTPUT_TO_DEPTHTARGET
        nextToken();
        nextToken();
        break;
      }
      case 78: { // DCL_OUTPUT_TO_RENDERTARGET
        nextToken();
        nextToken();
        break;
      }
      case 80: { // DCL_MAXOUT
        nextToken();
        break;
      }
      default:
        break;
    }

    // Skip remaining tokens of this instruction
    if (instLength > 1) {
      m_currentToken += instLength - 1;
    }
  }

  m_currentToken = savedPos;
}

// ============================================================================
// SPIR-V emission helpers
// ============================================================================

void SM4Translator::emitCapability(uint32_t cap) {
  m_spirv.push_back(0x00020011); // OpCapability
  m_spirv.push_back(cap);
}

void SM4Translator::emitExtInstImport() {
  uint32_t id = createId();
  m_spirv.push_back(0x00030010); // OpExtInstImport
  m_spirv.push_back(id);
  // string "GLSL.std.450" padded to 4-byte boundary
  const char* ext = "GLSL.std.450";
  uint32_t len = (uint32_t)strlen(ext) + 1;
  uint32_t words = (len + 3) / 4;
  uint32_t start = (uint32_t)m_spirv.size();
  m_spirv.resize(start + words, 0);
  memcpy(&m_spirv[start], ext, len);
  m_extInstImport = id;
}

void SM4Translator::emitMemoryModel() {
  m_spirv.push_back(0x00040000); // OpMemoryModel
  m_spirv.push_back(ADDRESS_MODEL_LOGICAL);
  m_spirv.push_back(MEMORY_MODEL_GLSL450);
}

void SM4Translator::emitEntryPoint() {
  uint32_t funcType = emitTypeFunction(m_voidType, {});
  m_mainFunction = createId();

  m_spirv.push_back(0x00150000); // OpEntryPoint
  m_spirv.push_back(m_isVertexShader ? EXECUTION_MODEL_VERTEX : EXECUTION_MODEL_FRAGMENT);
  m_spirv.push_back(m_mainFunction);
  // name "main"
  const char* name = "main";
  uint32_t len = (uint32_t)strlen(name) + 1;
  uint32_t words = (len + 3) / 4;
  uint32_t start = (uint32_t)m_spirv.size();
  m_spirv.resize(start + words, 0);
  memcpy(&m_spirv[start], name, len);

  // List interface variables
  for (auto& [slot, id] : m_inputVars) {
    if (id) m_spirv.push_back(id);
  }
  for (auto& [slot, id] : m_outputVars) {
    if (id) m_spirv.push_back(id);
  }
}

void SM4Translator::emitDecorations() {
  // Decorations for inputs
  for (auto& [slot, id] : m_inputVars) {
    if (id) {
      emitDecoration(id, DECORATION_LOCATION, {slot});
    }
  }
  // Decorations for outputs
  for (auto& [slot, id] : m_outputVars) {
    if (id) {
      if (slot == 0 && m_isVertexShader) {
        // SV_Position
        emitDecoration(id, DECORATION_BUILTIN, {BUILTIN_POSITION});
      } else {
        emitDecoration(id, DECORATION_LOCATION, {slot - 1});
      }
    }
  }
  // Cbuffer bindings
  for (auto& [slot, cb] : m_cbuffers) {
    if (cb.spirvTypeId) {
      emitDecoration(cb.spirvTypeId, DECORATION_DESCRIPTOR_SET, {0});
      emitDecoration(cb.spirvTypeId, DECORATION_BINDING, {slot});
    }
  }
}

void SM4Translator::emitTypeDeclarations() {
  m_voidType = createId();
  m_spirv.push_back(0x00030007); // OpTypeVoid
  m_spirv.push_back(m_voidType);

  m_floatType = createId();
  m_spirv.push_back(0x0003000D); // OpTypeFloat
  m_spirv.push_back(m_floatType);
  m_spirv.push_back(32);

  m_intType = createId();
  m_spirv.push_back(0x0003000C); // OpTypeInt
  m_spirv.push_back(m_intType);
  m_spirv.push_back(32);
  m_spirv.push_back(1); // signed

  m_uintType = createId();
  m_spirv.push_back(0x0003000C); // OpTypeInt
  m_spirv.push_back(m_uintType);
  m_spirv.push_back(32);
  m_spirv.push_back(0); // unsigned

  m_boolType = createId();
  m_spirv.push_back(0x0003000E); // OpTypeBool
  m_spirv.push_back(m_boolType);

  m_vec4Type = createId();
  m_spirv.push_back(0x0003000F); // OpTypeVector
  m_spirv.push_back(m_vec4Type);
  m_spirv.push_back(m_floatType);
  m_spirv.push_back(4);

  m_vec3Type = createId();
  m_spirv.push_back(0x0003000F); // OpTypeVector
  m_spirv.push_back(m_vec3Type);
  m_spirv.push_back(m_floatType);
  m_spirv.push_back(3);

  m_vec2Type = createId();
  m_spirv.push_back(0x0003000F); // OpTypeVector
  m_spirv.push_back(m_vec2Type);
  m_spirv.push_back(m_floatType);
  m_spirv.push_back(2);

  // Matrix type (4x4 float)
  m_mat4Type = createId();
  m_spirv.push_back(0x0003000B); // OpTypeMatrix
  m_spirv.push_back(m_mat4Type);
  m_spirv.push_back(m_vec4Type);
  m_spirv.push_back(4);

  // Input variables
  for (auto& [slot, id] : m_inputVars) {
    id = createId();
    m_spirv.push_back(0x00030005); // OpVariable
    m_spirv.push_back(getOrCreateTypeId(1, 4)); // vec4 input
    m_spirv.push_back(id);
    m_spirv.push_back(STORAGE_CLASS_INPUT);
  }

  // Output variables
  for (auto& [slot, id] : m_outputVars) {
    id = createId();
    m_spirv.push_back(0x00030005); // OpVariable
    m_spirv.push_back(getOrCreateTypeId(1, 4)); // vec4 output
    m_spirv.push_back(id);
    m_spirv.push_back(STORAGE_CLASS_OUTPUT);
  }

  // Cbuffer pointer types and variables
  for (auto& [slot, cb] : m_cbuffers) {
    uint32_t blockId = createId();
    cb.spirvTypeId = blockId;
  }
}

uint32_t SM4Translator::getOrCreateTypeId(uint32_t componentType, uint32_t componentCount) {
  if (componentType == 1 && componentCount == 4) return m_vec4Type;
  if (componentType == 1 && componentCount == 3) return m_vec3Type;
  if (componentType == 1 && componentCount == 2) return m_vec2Type;
  if (componentType == 1 && componentCount == 1) return m_floatType;
  return m_vec4Type;
}

uint32_t SM4Translator::emitTypeFunction(uint32_t returnType, const std::vector<uint32_t>& paramTypes) {
  uint32_t id = createId();
  m_spirv.push_back(0x0003000A); // OpTypeFunction
  m_spirv.push_back(id);
  m_spirv.push_back(returnType);
  for (auto pt : paramTypes) {
    m_spirv.push_back(pt);
  }
  return id;
}

void SM4Translator::emitDecoration(uint32_t target, uint32_t decoration, const std::vector<uint32_t>& args) {
  m_spirv.push_back(0x00040000 | (3 + (uint32_t)args.size())); // OpDecorate
  m_spirv.push_back(target);
  m_spirv.push_back(decoration);
  for (auto a : args) m_spirv.push_back(a);
}

void SM4Translator::emitLabel(uint32_t labelId) {
  m_spirv.push_back(0x000000F8); // OpLabel
  m_spirv.push_back(labelId);
}

void SM4Translator::emitReturn() {
  m_spirv.push_back(0x000000FD); // OpReturn
}

void SM4Translator::emitFunction(uint32_t functionType, uint32_t functionId) {
  m_spirv.push_back(0x00030036); // OpFunction
  m_spirv.push_back(m_voidType);
  m_spirv.push_back(functionId);
  m_spirv.push_back(FUNCTION_CONTROL_NONE);
  m_spirv.push_back(functionType);
}

void SM4Translator::emitFunctionEnd() {
  m_spirv.push_back(0x000000FC); // OpFunctionEnd
}

void SM4Translator::emitAccessChain(uint32_t resultType, uint32_t resultId, uint32_t baseId, const std::vector<uint32_t>& indexes) {
  m_spirv.push_back(0x00030041 | (uint32_t)(3 + indexes.size())); // OpAccessChain
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(baseId);
  for (auto idx : indexes) m_spirv.push_back(idx);
}

void SM4Translator::emitLoad(uint32_t resultType, uint32_t resultId, uint32_t pointerId) {
  m_spirv.push_back(0x00030047); // OpLoad
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(pointerId);
}

void SM4Translator::emitStore(uint32_t pointerId, uint32_t objectId) {
  m_spirv.push_back(0x00030048); // OpStore
  m_spirv.push_back(pointerId);
  m_spirv.push_back(objectId);
}

void SM4Translator::emitCompositeExtract(uint32_t resultType, uint32_t resultId, uint32_t sourceId, const std::vector<uint32_t>& indices) {
  m_spirv.push_back(0x00030046 | (uint32_t)(4 + indices.size())); // OpCompositeExtract
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(sourceId);
  for (auto i : indices) m_spirv.push_back(i);
}

void SM4Translator::emitCompositeConstruct(uint32_t resultType, uint32_t resultId, const std::vector<uint32_t>& components) {
  m_spirv.push_back(0x00030045 | (uint32_t)(3 + components.size())); // OpCompositeConstruct
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  for (auto c : components) m_spirv.push_back(c);
}

void SM4Translator::emitInstExtended(uint32_t opcode, uint32_t resultType, uint32_t resultId, const std::vector<uint32_t>& operands) {
  m_spirv.push_back(0x00010000 | (3 + (uint32_t)operands.size())); // OpExtInst
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(m_extInstImport);
  m_spirv.push_back(opcode);
  for (auto o : operands) m_spirv.push_back(o);
}

void SM4Translator::emitIAdd(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  m_spirv.push_back(0x0003004C); // OpIAdd
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(op1);
  m_spirv.push_back(op2);
}

void SM4Translator::emitFAdd(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  m_spirv.push_back(0x0003004E); // OpFAdd
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(op1);
  m_spirv.push_back(op2);
}

void SM4Translator::emitFSub(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  m_spirv.push_back(0x0003004F); // OpFSub
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(op1);
  m_spirv.push_back(op2);
}

void SM4Translator::emitFMul(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  m_spirv.push_back(0x00030051); // OpFMul
  m_spirv.push_back(resultType);
  m_spirv.push_back(resultId);
  m_spirv.push_back(op1);
  m_spirv.push_back(op2);
}

void SM4Translator::emitFMin(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  emitInstExtended(GLSL_STD450_FMin, resultType, resultId, {op1, op2});
}

void SM4Translator::emitFMax(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  emitInstExtended(GLSL_STD450_FMax, resultType, resultId, {op1, op2});
}

void SM4Translator::emitFAbs(uint32_t resultType, uint32_t resultId, uint32_t op1) {
  emitInstExtended(GLSL_STD450_FAbs, resultType, resultId, {op1});
}

void SM4Translator::emitFNegate(uint32_t resultType, uint32_t resultId, uint32_t op1) {
  emitInstExtended(GLSL_STD450_FNegate, resultType, resultId, {op1});
}

void SM4Translator::emitDot(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  emitInstExtended(GLSL_STD450_Dot, resultType, resultId, {op1, op2});
}

void SM4Translator::emitInverseSqrt(uint32_t resultType, uint32_t resultId, uint32_t op1) {
  emitInstExtended(GLSL_STD450_InverseSqrt, resultType, resultId, {op1});
}

void SM4Translator::emitFDiv(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2) {
  emitInstExtended(GLSL_STD450_FDiv, resultType, resultId, {op1, op2});
}

void SM4Translator::emitConstant(uint32_t type, uint32_t id, uint32_t value) {
  m_spirv.push_back(0x00030013); // OpConstant
  m_spirv.push_back(type);
  m_spirv.push_back(id);
  m_spirv.push_back(value);
}

// ============================================================================
// Instruction translation
// ============================================================================

void SM4Translator::translateInstruction(uint32_t opcode) {
  switch (opcode) {
    case 0: // ENDOP
      break;
    case 63: // DCL_INPUT
    case 64: // DCL_INPUT_PS
    case 62: // DCL_OUTPUT
    case 60: // DCL_CONSTANT_BUFFER
    case 58: // DCL_SAMPLER
    case 59: // DCL_TEXTURE
    case 61: // DCL_OUTPUT_TO_DEPTHTARGET
    case 78: // DCL_OUTPUT_TO_RENDERTARGET
    case 80: // DCL_MAXOUT
      translateDcl();
      break;
    case 1: // MOV
      translateMov();
      break;
    case 3: // ADD
      translateAdd();
      break;
    case 4: // MUL
      translateMul();
      break;
    case 9: // DP4
      translateDP4();
      break;
    case 8: // DP3
      translateDP3();
      break;
    case 10: // MIN
      translateMin();
      break;
    case 11: // MAX
      translateMax();
      break;
    case 120: // ABS
      translateAbs();
      break;
    case 121: // NEG
      translateNeg();
      break;
    case 7: // RSQ
      translateRSQ();
      break;
    case 115: // RCP
      translateRCP();
      break;
    case 6: // DIV
      translateDiv();
      break;
    case 5: // MAD
      translateMAD();
      break;
    case 122: // SINCOS
      translateSincos();
      break;
    case 145: // ENDOP (duplicate)
      break;
    default:
      skipInstruction();
      break;
  }
}

// ============================================================================
// DCL handling: skip the tokens
// ============================================================================

void SM4Translator::translateDcl() {
  // Already scanned in pass 1. Just skip operands.
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  if (instLength > 1) {
    m_currentToken += instLength - 1;
  }
}

// ============================================================================
// MOV: d = s0
// ============================================================================

void SM4Translator::translateMov() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t srcToken = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t srcId = resolveOperandId(srcToken, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && srcId) {
    emitStore(dstId, srcId);
  }

  // skip remaining tokens
  if (instLength > 3) {
    m_currentToken += instLength - 3;
  }
}

// ============================================================================
// ADD: d = s0 + s1
// ============================================================================

void SM4Translator::translateAdd() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitFAdd(vec4Type, result, src0Id, src1Id);
    emitStore(dstId, result);
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// MUL: d = s0 * s1
// ============================================================================

void SM4Translator::translateMul() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitFMul(vec4Type, result, src0Id, src1Id);
    emitStore(dstId, result);
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// DP4: d = dot4(s0, s1)
// ============================================================================

void SM4Translator::translateDP4() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitDot(m_floatType, result, src0Id, src1Id);
    // DP4 in SM4 writes to all 4 components of dest, replicating the scalar
    // For simplicity, store to first component
    uint32_t dstVec = resolveOperandId(dstToken, true);
    if (dstVec) {
      emitStore(dstVec, result);
    }
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// DP3: d = dot3(s0, s1)
// ============================================================================

void SM4Translator::translateDP3() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitDot(m_floatType, result, src0Id, src1Id);
    emitStore(dstId, result);
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// MIN: d = min(s0, s1)
// ============================================================================

void SM4Translator::translateMin() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitFMin(vec4Type, result, src0Id, src1Id);
    emitStore(dstId, result);
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// MAX: d = max(s0, s1)
// ============================================================================

void SM4Translator::translateMax() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitFMax(vec4Type, result, src0Id, src1Id);
    emitStore(dstId, result);
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// ABS: d = abs(s0)
// ============================================================================

void SM4Translator::translateAbs() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id) {
    uint32_t result = createId();
    emitFAbs(vec4Type, result, src0Id);
    emitStore(dstId, result);
  }

  if (instLength > 3) {
    m_currentToken += instLength - 3;
  }
}

// ============================================================================
// NEG: d = -s0
// ============================================================================

void SM4Translator::translateNeg() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id) {
    uint32_t result = createId();
    emitFNegate(vec4Type, result, src0Id);
    emitStore(dstId, result);
  }

  if (instLength > 3) {
    m_currentToken += instLength - 3;
  }
}

// ============================================================================
// RSQ: d = inverseSqrt(s0)
// ============================================================================

void SM4Translator::translateRSQ() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);

  if (dstId && src0Id) {
    uint32_t result = createId();
    emitInverseSqrt(m_floatType, result, src0Id);
    emitStore(dstId, result);
  }

  if (instLength > 3) {
    m_currentToken += instLength - 3;
  }
}

// ============================================================================
// RCP: d = 1.0 / s0
// ============================================================================

void SM4Translator::translateRCP() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);

  if (dstId && src0Id) {
    // Create constant 1.0
    uint32_t oneId = createId();
    m_spirv.push_back(0x00030013); // OpConstant
    m_spirv.push_back(m_floatType);
    m_spirv.push_back(oneId);
    m_spirv.push_back(0x3F800000); // 1.0f

    uint32_t result = createId();
    emitFDiv(m_floatType, result, oneId, src0Id);
    emitStore(dstId, result);
  }

  if (instLength > 3) {
    m_currentToken += instLength - 3;
  }
}

// ============================================================================
// DIV: d = s0 / s1
// ============================================================================

void SM4Translator::translateDiv() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id && src1Id) {
    uint32_t result = createId();
    emitFDiv(vec4Type, result, src0Id, src1Id);
    emitStore(dstId, result);
  }

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// MAD: d = s0 * s1 + s2
// ============================================================================

void SM4Translator::translateMAD() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dstToken = nextToken();
  uint32_t src0Token = nextToken();
  uint32_t src1Token = nextToken();
  uint32_t src2Token = nextToken();

  uint32_t dstId = resolveOperandId(dstToken, true);
  uint32_t src0Id = resolveOperandId(src0Token, false);
  uint32_t src1Id = resolveOperandId(src1Token, false);
  uint32_t src2Id = resolveOperandId(src2Token, false);
  uint32_t vec4Type = getOrCreateTypeId(1, 4);

  if (dstId && src0Id && src1Id && src2Id) {
    uint32_t mulResult = createId();
    emitFMul(vec4Type, mulResult, src0Id, src1Id);
    uint32_t result = createId();
    emitFAdd(vec4Type, result, mulResult, src2Id);
    emitStore(dstId, result);
  }

  if (instLength > 5) {
    m_currentToken += instLength - 5;
  }
}

// ============================================================================
// SINCOS: d0 = cos(s0), d1 = sin(s0), d2 = s0 (legacy)
// ============================================================================

void SM4Translator::translateSincos() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  uint32_t dst0Token = nextToken();
  uint32_t dst1Token = nextToken();
  uint32_t src0Token = nextToken();

  // In D3D, SINCOS writes cosine to dest0 and sine to dest1
  // For now, just write identity (pass-through) since we can't easily do both
  uint32_t dst0Id = resolveOperandId(dst0Token, true);
  uint32_t dst1Id = resolveOperandId(dst1Token, true);

  if (instLength > 4) {
    m_currentToken += instLength - 4;
  }
}

// ============================================================================
// Skip any instruction we haven't implemented yet
// ============================================================================

void SM4Translator::skipInstruction() {
  uint32_t instLength = (m_tokens[m_currentToken - 1] >> 8) & 0xFF;
  if (instLength > 1) {
    m_currentToken += instLength - 1;
  }
}

// ============================================================================
// Operand resolution: decode SM4 operand → SPIR-V ID
// ============================================================================

uint32_t SM4Translator::resolveOperandId(uint32_t token, bool isDst) {
  uint32_t operandType = getOperandType(token);
  uint32_t regIndex = getOperandIndex(token);

  switch (operandType) {
    case 0: { // TEMP (r0-r31)
      if (regIndex < 32) {
        if (m_tempRegisters[regIndex] == 0) {
          m_tempRegisters[regIndex] = createId();
          m_spirv.push_back(0x00030005); // OpVariable
          m_spirv.push_back(m_vec4Type);
          m_spirv.push_back(m_tempRegisters[regIndex]);
          m_spirv.push_back(STORAGE_CLASS_FUNCTION);
        }
        return m_tempRegisters[regIndex];
      }
      return 0;
    }
    case 1: { // INPUT (v0-v15)
      auto it = m_inputVars.find(regIndex);
      if (it != m_inputVars.end()) return it->second;
      return 0;
    }
    case 2: { // OUTPUT (o0-o15)
      auto it = m_outputVars.find(regIndex);
      if (it != m_outputVars.end()) return it->second;
      return 0;
    }
    case 3: { // CBUFFER (c0-c15)
      // Return a dummy for now — full cbuffer access needs descriptor set bindings
      return 0;
    }
    case 4: { // IMMEDIATE32
      // Should have been handled in a different path
      return 0;
    }
    case 5: { // IMMEDIATE64
      return 0;
    }
    case 8: { // LABEL
      return createId();
    }
    case 9: { // SAMPLER (s0-s15)
      return 0;
    }
    case 10: { // RESOURCE (t0-t15)
      return 0;
    }
    case 11: { // UAV (u0-u15)
      return 0;
    }
    default:
      return 0;
  }
}

void SM4Translator::setInputSignature(const void* isgn, uint32_t size) {
}

void SM4Translator::setOutputSignature(const void* osgn, uint32_t size) {
}
