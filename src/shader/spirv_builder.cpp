#include "spirv_builder.h"
#include "../common/logging.h"
#include <cstring>
#include <algorithm>

// ============================================================================
// SPIR-V Builder Implementation
// ============================================================================

SpirvBuilder::SpirvBuilder() {
  reset();
}

void SpirvBuilder::reset() {
  m_spirv.clear();
  m_nextId = 1;
  m_bound = 1;

  // Header placeholder
  m_spirv.push_back(0x07230203); // magic
  m_spirv.push_back(0x00010000); // version 1.0
  m_spirv.push_back(0);          // generator
  m_spirv.push_back(0);          // bound (set later)
  m_spirv.push_back(0);          // schema

  // Cache common types
  m_voidTypeId = 0;
  m_floatTypeId = 0;
  m_intTypeId = 0;
  m_boolTypeId = 0;
  m_extInstImportId = 0;
}

void SpirvBuilder::setBound(uint32_t bound) {
  m_bound = bound;
  if (m_spirv.size() > 3) {
    m_spirv[3] = bound;
  }
}

void SpirvBuilder::capability(uint32_t cap) {
  emitOp(SPV_OP_CAPABILITY, 3, {cap});
}

void SpirvBuilder::memoryModel(uint32_t addressingModel, uint32_t memoryModel) {
  emitOp(SPV_OP_MEMORY_MODEL, 4, {addressingModel, memoryModel});
}

// --- Types ---

uint32_t SpirvBuilder::typeVoid() {
  if (m_voidTypeId) return m_voidTypeId;
  m_voidTypeId = createId();
  emitOp(SPV_OP_TYPE_VOID, 2, {m_voidTypeId});
  return m_voidTypeId;
}

uint32_t SpirvBuilder::typeFloat(uint32_t width) {
  if (width == 32 && m_floatTypeId) return m_floatTypeId;
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_FLOAT, 3, {id, width});
  if (width == 32) m_floatTypeId = id;
  return id;
}

uint32_t SpirvBuilder::typeInt(uint32_t width, bool signedness) {
  if (width == 32 && m_intTypeId) return m_intTypeId;
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_INT, 4, {id, width, signedness ? 1u : 0u});
  if (width == 32) m_intTypeId = id;
  return id;
}

uint32_t SpirvBuilder::typeBool() {
  if (m_boolTypeId) return m_boolTypeId;
  m_boolTypeId = createId();
  emitOp(SPV_OP_TYPE_BOOL, 2, {m_boolTypeId});
  return m_boolTypeId;
}

uint32_t SpirvBuilder::typeVector(uint32_t componentType, uint32_t componentCount) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_VECTOR, 4, {id, componentType, componentCount});
  if (componentType == m_floatTypeId && componentCount == 2) m_vec2TypeId = id;
  if (componentType == m_floatTypeId && componentCount == 3) m_vec3TypeId = id;
  if (componentType == m_floatTypeId && componentCount == 4) m_vec4TypeId = id;
  return id;
}

uint32_t SpirvBuilder::typeMatrix(uint32_t columnType, uint32_t columnCount) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_MATRIX, 4, {id, columnType, columnCount});
  if (columnType == m_vec4TypeId && columnCount == 4) m_mat4TypeId = id;
  return id;
}

uint32_t SpirvBuilder::typeArray(uint32_t elementType, uint32_t length) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_ARRAY, 4, {id, elementType, length});
  return id;
}

uint32_t SpirvBuilder::typeRuntimeArray(uint32_t elementType) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_RUNTIME_ARRAY, 3, {id, elementType});
  return id;
}

uint32_t SpirvBuilder::typeStruct(const std::vector<uint32_t>& memberTypes) {
  uint32_t id = createId();
  std::vector<uint32_t> operands = {id};
  operands.insert(operands.end(), memberTypes.begin(), memberTypes.end());
  emitOp(SPV_OP_TYPE_STRUCT, operands.size(), operands);
  return id;
}

uint32_t SpirvBuilder::typePointer(uint32_t storageClass, uint32_t pointeeType) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_POINTER, 4, {id, storageClass, pointeeType});
  return id;
}

uint32_t SpirvBuilder::typeImage(uint32_t sampledType, uint32_t dimension, uint32_t depth, uint32_t arrayed, uint32_t multiSampled, uint32_t sampled, uint32_t format) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_IMAGE, 8, {id, sampledType, depth, arrayed, multiSampled, sampled, format, dimension});
  return id;
}

uint32_t SpirvBuilder::typeSampler() {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_SAMPLER, 2, {id});
  return id;
}

uint32_t SpirvBuilder::typeSampledImage(uint32_t imageType) {
  uint32_t id = createId();
  emitOp(SPV_OP_TYPE_SAMPLED_IMAGE, 3, {id, imageType});
  return id;
}

// --- Constants ---

uint32_t SpirvBuilder::constantFloat(uint32_t floatType, float value) {
  uint32_t id = createId();
  uint32_t valueBits;
  memcpy(&valueBits, &value, 4);
  emitOp(59, 4, {floatType, id, valueBits}); // OpConstant
  return id;
}

uint32_t SpirvBuilder::constantInt(uint32_t intType, int32_t value) {
  uint32_t id = createId();
  emitOp(59, 4, {intType, id, static_cast<uint32_t>(value)});
  return id;
}

uint32_t SpirvBuilder::constantUint(uint32_t intType, uint32_t value) {
  uint32_t id = createId();
  emitOp(59, 4, {intType, id, value});
  return id;
}

uint32_t SpirvBuilder::constantComposite(uint32_t resultType, const std::vector<uint32_t>& constituents) {
  uint32_t id = createId();
  std::vector<uint32_t> operands = {resultType, id};
  operands.insert(operands.end(), constituents.begin(), constituents.end());
  emitOp(SPV_OP_COMPOSITE_CONSTRUCT, operands.size(), operands);
  return id;
}

// --- Variables ---

uint32_t SpirvBuilder::variable(uint32_t resultType, uint32_t storageClass) {
  uint32_t id = createId();
  emitOp(SPV_OP_VARIABLE, 4, {resultType, id, storageClass});
  return id;
}

// --- Decorations ---

void SpirvBuilder::decorate(uint32_t target, uint32_t decoration, const std::vector<uint32_t>& extraOperands) {
  std::vector<uint32_t> operands = {target, decoration};
  operands.insert(operands.end(), extraOperands.begin(), extraOperands.end());
  emitOp(SPV_OP_DECORATE, operands.size(), operands);
}

void SpirvBuilder::memberDecorate(uint32_t structType, uint32_t memberIndex, uint32_t decoration, const std::vector<uint32_t>& extraOperands) {
  std::vector<uint32_t> operands = {structType, memberIndex, decoration};
  operands.insert(operands.end(), extraOperands.begin(), extraOperands.end());
  emitOp(72, operands.size(), operands); // OpMemberDecorate
}

void SpirvBuilder::name(uint32_t target, const std::string& name) {
  std::vector<uint32_t> operands = {target};
  // Encode string as uint32 words
  size_t len = name.size();
  size_t wordLen = (len + 4) / 4;
  std::vector<uint32_t> nameWords(wordLen, 0);
  memcpy(nameWords.data(), name.c_str(), len);
  operands.insert(operands.end(), nameWords.begin(), nameWords.end());
  emitOp(SPV_OP_NAME, operands.size(), operands);
}

void SpirvBuilder::memberName(uint32_t structType, uint32_t memberIndex, const std::string& name) {
  std::vector<uint32_t> operands = {structType, memberIndex};
  size_t len = name.size();
  size_t wordLen = (len + 4) / 4;
  std::vector<uint32_t> nameWords(wordLen, 0);
  memcpy(nameWords.data(), name.c_str(), len);
  operands.insert(operands.end(), nameWords.begin(), nameWords.end());
  emitOp(70, operands.size(), operands); // OpMemberName
}

// --- Entry point ---

void SpirvBuilder::entryPoint(uint32_t executionModel, uint32_t entryPointId, const std::string& name, const std::vector<uint32_t>& interfaces) {
  std::vector<uint32_t> operands = {executionModel, entryPointId};
  size_t len = name.size();
  size_t wordLen = (len + 4) / 4;
  std::vector<uint32_t> nameWords(wordLen, 0);
  memcpy(nameWords.data(), name.c_str(), len);
  operands.insert(operands.end(), nameWords.begin(), nameWords.end());
  operands.insert(operands.end(), interfaces.begin(), interfaces.end());
  emitOp(SPV_OP_ENTRY_POINT, operands.size(), operands);
}

// --- Function ---

uint32_t SpirvBuilder::function(uint32_t resultType, uint32_t functionControl, uint32_t functionType) {
  uint32_t id = createId();
  emitOp(SPV_OP_FUNCTION, 5, {resultType, id, functionControl, functionType});
  return id;
}

void SpirvBuilder::functionEnd() {
  emitOp(SPV_OP_FUNCTION_END, 1);
}

void SpirvBuilder::label(uint32_t labelId) {
  emitOp(SPV_OP_LABEL, 2, {labelId});
}

// --- Instructions ---

uint32_t SpirvBuilder::load(uint32_t resultType, uint32_t pointer, uint32_t memoryAccess) {
  uint32_t id = createId();
  if (memoryAccess) {
    emitOp(SPV_OP_LOAD, 5, {resultType, id, pointer, memoryAccess});
  } else {
    emitOp(SPV_OP_LOAD, 4, {resultType, id, pointer});
  }
  return id;
}

void SpirvBuilder::store(uint32_t pointer, uint32_t object, uint32_t memoryAccess) {
  if (memoryAccess) {
    emitOp(SPV_OP_STORE, 4, {pointer, object, memoryAccess});
  } else {
    emitOp(SPV_OP_STORE, 3, {pointer, object});
  }
}

uint32_t SpirvBuilder::accessChain(uint32_t resultType, uint32_t base, const std::vector<uint32_t>& indexes) {
  uint32_t id = createId();
  std::vector<uint32_t> operands = {resultType, id, base};
  operands.insert(operands.end(), indexes.begin(), indexes.end());
  emitOp(SPV_OP_ACCESS_CHAIN, operands.size(), operands);
  return id;
}

uint32_t SpirvBuilder::compositeConstruct(uint32_t resultType, const std::vector<uint32_t>& constituents) {
  uint32_t id = createId();
  std::vector<uint32_t> operands = {resultType, id};
  operands.insert(operands.end(), constituents.begin(), constituents.end());
  emitOp(SPV_OP_COMPOSITE_CONSTRUCT, operands.size(), operands);
  return id;
}

uint32_t SpirvBuilder::compositeExtract(uint32_t resultType, uint32_t composite, const std::vector<uint32_t>& indexes) {
  uint32_t id = createId();
  std::vector<uint32_t> operands = {resultType, id, composite};
  operands.insert(operands.end(), indexes.begin(), indexes.end());
  emitOp(SPV_OP_COMPOSITE_EXTRACT, operands.size(), operands);
  return id;
}

uint32_t SpirvBuilder::vectorShuffle(uint32_t resultType, uint32_t vector1, uint32_t vector2, const std::vector<uint32_t>& components) {
  uint32_t id = createId();
  std::vector<uint32_t> operands = {resultType, id, vector1, vector2};
  operands.insert(operands.end(), components.begin(), components.end());
  emitOp(SPV_OP_VECTOR_SHUFFLE, operands.size(), operands);
  return id;
}

// --- Arithmetic ---

uint32_t SpirvBuilder::fadd(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_FADD, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::fsub(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_FSUB, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::fmul(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_FMUL, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::fdiv(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_FDIV, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::fnegate(uint32_t resultType, uint32_t operand) {
  uint32_t id = createId();
  emitOp(SPV_OP_FNEGATE, 4, {resultType, id, operand});
  return id;
}

uint32_t SpirvBuilder::fmin(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_FMIN, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::fmax(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_FMAX, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::dotProduct(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  uint32_t id = createId();
  emitOp(SPV_OP_DOT, 5, {resultType, id, operand1, operand2});
  return id;
}

uint32_t SpirvBuilder::crossProduct(uint32_t resultType, uint32_t operand1, uint32_t operand2) {
  if (!m_extInstImportId) {
    m_extInstImportId = createId();
    const char* glsl = "GLSL.std.450";
    size_t len = strlen(glsl);
    size_t wordLen = (len + 4) / 4;
    std::vector<uint32_t> operands;
    operands.push_back(m_extInstImportId);
    for (size_t i = 0; i < wordLen; i++) {
      uint32_t word = 0;
      memcpy(&word, glsl + i * 4, std::min((size_t)4, len - i * 4));
      operands.push_back(word);
    }
    emitOp(SPV_OP_EXT_INST_IMPORT, 1 + operands.size(), operands);
  }
  uint32_t id = createId();
  // OpExtInst: ResultType, Result, Set, Instruction, Operand1, Operand2
  // GLSL.std.450 Cross = opcode 46
  emitOp(SPV_OP_EXT_INST, 6, {resultType, id, m_extInstImportId, 46, operand1, operand2});
  return id;
}

// --- Texture Sampling ---

uint32_t SpirvBuilder::imageSampleImplicitLod(uint32_t resultType, uint32_t sampledImage, uint32_t coordinate) {
  uint32_t id = createId();
  emitOp(SPV_OP_IMAGE_SAMPLE_IMPLICIT_LOD, 5, {resultType, id, sampledImage, coordinate});
  return id;
}

uint32_t SpirvBuilder::imageSampleExplicitLod(uint32_t resultType, uint32_t sampledImage, uint32_t coordinate, uint32_t lod) {
  uint32_t id = createId();
  emitOp(SPV_OP_IMAGE_SAMPLEPLICIT_LOD, 7, {resultType, id, sampledImage, coordinate, 2, lod}); // LoD image operand
  return id;
}

uint32_t SpirvBuilder::imageSampleGrad(uint32_t resultType, uint32_t sampledImage, uint32_t coordinate, uint32_t dx, uint32_t dy) {
  uint32_t id = createId();
  emitOp(114, 7, {resultType, id, sampledImage, coordinate, dx, dy}); // OpImageSampleGrad
  return id;
}

// --- Control Flow ---

void SpirvBuilder::ret() {
  emitOp(SPV_OP_RET, 1);
}

uint32_t SpirvBuilder::iF(uint32_t condition) {
  uint32_t mergeBlock = createId();
  uint32_t trueBlock = createId();
  emitOp(SPV_OP_SELECTION_MERGE, 3, {mergeBlock, 0}); // selection merge
  emitOp(247, 4, {condition, trueBlock, mergeBlock}); // OpBranchConditional (pre-label)
  return mergeBlock;
}

void SpirvBuilder::iElse(uint32_t mergeBlock) {
  uint32_t falseBlock = createId();
  emitOp(SPV_OP_BRANCH, 2, {falseBlock}); // branch to false block
  label(falseBlock); // false block label
}

void SpirvBuilder::endIf(uint32_t mergeBlock) {
  emitOp(SPV_OP_BRANCH, 2, {mergeBlock}); // branch to merge
  label(mergeBlock); // merge label
}

uint32_t SpirvBuilder::iBranch(uint32_t targetLabel) {
  emitOp(SPV_OP_BRANCH, 2, {targetLabel});
  return 0;
}

uint32_t SpirvBuilder::iBranchConditional(uint32_t condition, uint32_t trueLabel, uint32_t falseLabel) {
  emitOp(SPV_OP_BRANCH_CONDITIONAL, 4, {condition, trueLabel, falseLabel});
  return 0;
}

// --- Internal ---

void SpirvBuilder::emitOp(uint32_t opcode, uint32_t wordCount, const std::vector<uint32_t>& operands) {
  m_spirv.push_back(opcode | (wordCount << 16));
  m_spirv.insert(m_spirv.end(), operands.begin(), operands.end());
  m_bound = m_nextId;
  m_spirv[3] = m_bound;
}
