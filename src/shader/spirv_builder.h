#pragma once

#include <cstdint>
#include <vector>
#include <string>

// ============================================================================
// SPIR-V Builder — low-level SPIR-V emission
// ============================================================================
//
// Helper class for building valid SPIR-V modules programmatically.
// Used by the SM4 translator.
//
// SPIR-V word layout:
//   opcode | (wordCount << 16) | operand0 | operand1 | ...
//

// SPIR-V opcodes
enum SpvOpcode : uint32_t {
  SPV_OP_NAME = 5,
  SPV_OP_EXT_INST_IMPORT = 11,
  SPV_OP_EXT_INST = 12,
  SPV_OP_MEMORY_MODEL = 14,
  SPV_OP_ENTRY_POINT = 15,
  SPV_OP_CAPABILITY = 17,
  SPV_OP_TYPE_VOID = 19,
  SPV_OP_TYPE_INT = 21,
  SPV_OP_TYPE_FLOAT = 22,
  SPV_OP_TYPE_VECTOR = 23,
  SPV_OP_TYPE_MATRIX = 24,
  SPV_OP_TYPE_IMAGE = 25,
  SPV_OP_TYPE_SAMPLER = 27,
  SPV_OP_TYPE_SAMPLED_IMAGE = 28,
  SPV_OP_TYPE_ARRAY = 29,
  SPV_OP_TYPE_RUNTIME_ARRAY = 30,
  SPV_OP_TYPE_STRUCT = 31,
  SPV_OP_TYPE_POINTER = 32,
  SPV_OP_TYPE_BOOL = 36,
  SPV_OP_FMIN = 40,
  SPV_OP_FMAX = 41,
  SPV_OP_FUNCTION = 54,
  SPV_OP_FUNCTION_END = 56,
  SPV_OP_VARIABLE = 59,
  SPV_OP_LOAD = 61,
  SPV_OP_STORE = 62,
  SPV_OP_ACCESS_CHAIN = 65,
  SPV_OP_DECORATE = 71,
  SPV_OP_VECTOR_SHUFFLE = 79,
  SPV_OP_COMPOSITE_CONSTRUCT = 81,
  SPV_OP_COMPOSITE_EXTRACT = 82,
  SPV_OP_IMAGE_SAMPLE_IMPLICIT_LOD = 88,
  SPV_OP_IMAGE_SAMPLEPLICIT_LOD = 89,
  SPV_OP_FNEGATE = 127,
  SPV_OP_FADD = 129,
  SPV_OP_FSUB = 130,
  SPV_OP_FMUL = 131,
  SPV_OP_FDIV = 136,
  SPV_OP_DP3 = 138,
  SPV_OP_DP4 = 139,
  SPV_OP_DOT = 150,
  SPV_OP_SELECTION_MERGE = 232,
  SPV_OP_LABEL = 248,
  SPV_OP_BRANCH = 249,
  SPV_OP_BRANCH_CONDITIONAL = 250,
  SPV_OP_RET = 253,

  // Placeholder for cross product (will be properly emitted as OpExtInst GLSL.std.450)
  SPV_OP_CROSS = 0xFFFF,
  SPV_GLSL_STD450_EXT_INST_SET = 1,
};

// Decoration values
enum SpvDecoration : uint32_t {
  SPV_DECORATION_BLOCK = 2,
  SPV_DECORATION_BUFFER_BLOCK = 3,
  SPV_DECORATION_BINDING = 33,
  SPV_DECORATION_DESCRIPTOR_SET = 34,
  SPV_DECORATION_LOCATION = 30,
  SPV_DECORATION_BUILTIN = 11,
};

// Built-in values
enum SpvBuiltIn : uint32_t {
  SPV_BUILTIN_POSITION = 0,
  SPV_BUILTIN_VERTEX_INDEX = 42,
  SPV_BUILTIN_INSTANCE_INDEX = 43,
  SPV_BUILTIN_FRAG_COORD = 15,
  SPV_BUILTIN_FRAG_DEPTH = 22,
};

class SpirvBuilder {
public:
  SpirvBuilder();

  void reset();

  // Header
  void setBound(uint32_t bound);

  // Capabilities
  void capability(uint32_t cap);
  void memoryModel(uint32_t addressingModel, uint32_t memoryModel);

  // Types
  uint32_t typeVoid();
  uint32_t typeFloat(uint32_t width = 32);
  uint32_t typeInt(uint32_t width = 32, bool signedness = true);
  uint32_t typeBool();
  uint32_t typeVector(uint32_t componentType, uint32_t componentCount);
  uint32_t typeMatrix(uint32_t columnType, uint32_t columnCount);
  uint32_t typeArray(uint32_t elementType, uint32_t length);
  uint32_t typeRuntimeArray(uint32_t elementType);
  uint32_t typeStruct(const std::vector<uint32_t>& memberTypes);
  uint32_t typePointer(uint32_t storageClass, uint32_t pointeeType);
  uint32_t typeImage(uint32_t sampledType, uint32_t dimension, uint32_t depth = 0, uint32_t arrayed = 0, uint32_t multiSampled = 0, uint32_t sampled = 1, uint32_t format = 0);
  uint32_t typeSampler();
  uint32_t typeSampledImage(uint32_t imageType);

  // Constants
  uint32_t constantFloat(uint32_t floatType, float value);
  uint32_t constantInt(uint32_t intType, int32_t value);
  uint32_t constantUint(uint32_t intType, uint32_t value);
  uint32_t constantComposite(uint32_t resultType, const std::vector<uint32_t>& constituents);

  // Variables
  uint32_t variable(uint32_t resultType, uint32_t storageClass);

  // Decorations
  void decorate(uint32_t target, uint32_t decoration, const std::vector<uint32_t>& extraOperands = {});
  void memberDecorate(uint32_t structType, uint32_t memberIndex, uint32_t decoration, const std::vector<uint32_t>& extraOperands = {});
  void name(uint32_t target, const std::string& name);
  void memberName(uint32_t structType, uint32_t memberIndex, const std::string& name);

  // Entry point
  void entryPoint(uint32_t executionModel, uint32_t entryPointId, const std::string& name, const std::vector<uint32_t>& interfaces = {});

  // Function
  uint32_t function(uint32_t resultType, uint32_t functionControl, uint32_t functionType);
  void functionEnd();
  void label(uint32_t labelId);

  // Instructions
  uint32_t load(uint32_t resultType, uint32_t pointer, uint32_t memoryAccess = 0);
  void store(uint32_t pointer, uint32_t object, uint32_t memoryAccess = 0);
  uint32_t accessChain(uint32_t resultType, uint32_t base, const std::vector<uint32_t>& indexes);
  uint32_t compositeConstruct(uint32_t resultType, const std::vector<uint32_t>& constituents);
  uint32_t compositeExtract(uint32_t resultType, uint32_t composite, const std::vector<uint32_t>& indexes);
  uint32_t vectorShuffle(uint32_t resultType, uint32_t vector1, uint32_t vector2, const std::vector<uint32_t>& components);

  // Arithmetic
  uint32_t fadd(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t fsub(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t fmul(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t fdiv(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t fnegate(uint32_t resultType, uint32_t operand);
  uint32_t fmin(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t fmax(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t dotProduct(uint32_t resultType, uint32_t operand1, uint32_t operand2);
  uint32_t crossProduct(uint32_t resultType, uint32_t operand1, uint32_t operand2);

  // Texture sampling
  uint32_t imageSampleImplicitLod(uint32_t resultType, uint32_t sampledImage, uint32_t coordinate);
  uint32_t imageSampleExplicitLod(uint32_t resultType, uint32_t sampledImage, uint32_t coordinate, uint32_t lod);
  uint32_t imageSampleGrad(uint32_t resultType, uint32_t sampledImage, uint32_t coordinate, uint32_t dx, uint32_t dy);

  // Control flow
  void ret();
  uint32_t iF(uint32_t condition);
  void iElse(uint32_t mergeBlock);
  void endIf(uint32_t mergeBlock);
  uint32_t iBranch(uint32_t targetLabel);
  uint32_t iBranchConditional(uint32_t condition, uint32_t trueLabel, uint32_t falseLabel);

  // Output
  const std::vector<uint32_t>& getSpirv() const { return m_spirv; }
  std::vector<uint32_t> takeSpirv() { return std::move(m_spirv); }

private:
  std::vector<uint32_t> m_spirv;
  uint32_t m_nextId = 1;
  uint32_t m_bound = 1;

  // Cached type IDs
  uint32_t m_voidTypeId = 0;
  uint32_t m_floatTypeId = 0;
  uint32_t m_intTypeId = 0;
  uint32_t m_boolTypeId = 0;
  uint32_t m_vec2TypeId = 0;
  uint32_t m_vec3TypeId = 0;
  uint32_t m_vec4TypeId = 0;
  uint32_t m_mat4TypeId = 0;

  uint32_t m_extInstImportId = 0;

  uint32_t createId() { return m_nextId++; }
  void emitOp(uint32_t opcode, uint32_t wordCount, const std::vector<uint32_t>& operands = {});
};
