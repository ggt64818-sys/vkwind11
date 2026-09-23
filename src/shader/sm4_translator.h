#pragma once

#include "dxbc_parser.h"
#include <vector>
#include <cstdint>
#include <string>
#include <map>

// ============================================================================
// SM4/SM5 → SPIR-V Translator
// ============================================================================

struct SM4ConstantBuffer {
  std::string name;
  uint32_t slot;
  uint32_t size;
  std::vector<uint8_t> data;
  uint32_t spirvTypeId = 0;
};

struct SM4TextureBinding {
  std::string name;
  uint32_t slot;
  uint32_t samplerSlot;
  uint32_t dimension;
};

struct SM4TranslateResult {
  std::vector<uint32_t> spirvWords;
  std::string entryPointName;
  bool success = false;
  std::string errorMessage;
};

class SM4Translator {
public:
  SM4Translator();
  SM4TranslateResult translate(const ParsedDXBC& dxbc);
  void setInputSignature(const void* isgn, uint32_t size);
  void setOutputSignature(const void* osgn, uint32_t size);

private:
  // Token stream
  const uint32_t* m_tokens = nullptr;
  uint32_t m_tokenCount = 0;
  uint32_t m_currentToken = 0;

  // Shader info
  bool m_isVertexShader = false;
  bool m_isPixelShader = false;

  // SPIR-V output
  std::vector<uint32_t> m_spirv;

  // ID tracking
  uint32_t m_nextId = 1;

  // Built-in types
  uint32_t m_voidType = 0;
  uint32_t m_floatType = 0;
  uint32_t m_vec2Type = 0;
  uint32_t m_vec3Type = 0;
  uint32_t m_vec4Type = 0;
  uint32_t m_intType = 0;
  uint32_t m_uintType = 0;
  uint32_t m_boolType = 0;
  uint32_t m_mat4Type = 0;
  uint32_t m_extInstImport = 0;

  // Entry point
  uint32_t m_entryPoint = 0;
  uint32_t m_mainFunction = 0;

  // Resources
  std::map<uint32_t, SM4ConstantBuffer> m_cbuffers;
  std::map<uint32_t, SM4TextureBinding> m_textures;

  // Register → SPIR-V ID maps
  std::map<uint32_t, uint32_t> m_inputVars;
  std::map<uint32_t, uint32_t> m_outputVars;
  uint32_t m_tempRegisters[32] = {};

  // Operand decode
  uint32_t peekToken() const;
  uint32_t nextToken();
  uint32_t getOperandType(uint32_t token) const;
  uint32_t getOperandIndex(uint32_t token) const;

  // Pass 1: scan declarations
  void scanDeclarations();

  // SPIR-V emission
  void emitCapability(uint32_t cap);
  void emitExtInstImport();
  void emitMemoryModel();
  void emitEntryPoint();
  void emitDecorations();
  void emitTypeDeclarations();
  uint32_t emitTypeFunction(uint32_t returnType, const std::vector<uint32_t>& paramTypes);
  void emitFunction(uint32_t functionType, uint32_t functionId);
  void emitFunctionEnd();
  void emitLabel(uint32_t labelId);
  void emitReturn();
  void emitDecoration(uint32_t target, uint32_t decoration, const std::vector<uint32_t>& args);
  void emitAccessChain(uint32_t resultType, uint32_t resultId, uint32_t baseId, const std::vector<uint32_t>& indexes);
  void emitLoad(uint32_t resultType, uint32_t resultId, uint32_t pointerId);
  void emitStore(uint32_t pointerId, uint32_t objectId);
  void emitCompositeExtract(uint32_t resultType, uint32_t resultId, uint32_t sourceId, const std::vector<uint32_t>& indices);
  void emitCompositeConstruct(uint32_t resultType, uint32_t resultId, const std::vector<uint32_t>& components);
  void emitInstExtended(uint32_t opcode, uint32_t resultType, uint32_t resultId, const std::vector<uint32_t>& operands);
  void emitIAdd(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitFAdd(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitFSub(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitFMul(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitFMin(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitFMax(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitFAbs(uint32_t resultType, uint32_t resultId, uint32_t op1);
  void emitFNegate(uint32_t resultType, uint32_t resultId, uint32_t op1);
  void emitDot(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitInverseSqrt(uint32_t resultType, uint32_t resultId, uint32_t op1);
  void emitFDiv(uint32_t resultType, uint32_t resultId, uint32_t op1, uint32_t op2);
  void emitConstant(uint32_t type, uint32_t id, uint32_t value);

  // Instruction translation
  void translateInstruction(uint32_t opcode);
  void translateDcl();
  void translateMov();
  void translateAdd();
  void translateMul();
  void translateDP4();
  void translateDP3();
  void translateMin();
  void translateMax();
  void translateAbs();
  void translateNeg();
  void translateRSQ();
  void translateRCP();
  void translateDiv();
  void translateMAD();
  void translateSincos();
  void skipInstruction();

  // Helpers
  uint32_t createId();
  uint32_t getOrCreateTypeId(uint32_t componentType, uint32_t componentCount);
  uint32_t resolveOperandId(uint32_t token, bool isDst);
};
