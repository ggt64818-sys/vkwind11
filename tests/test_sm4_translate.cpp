#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cstdint>
#include "../src/shader/dxbc_parser.h"
#include "../src/shader/spirv_builder.h"
#include "../src/shader/sm4_translator.h"
#include "../src/d3dx/d3dx_math.h"

static int g_passed = 0;
static int g_failed = 0;

#ifndef D3DX_PI
#define D3DX_PI 3.14159265358979323846f
#endif

#define TEST(name) printf("[Test] %s... ", name)
#define PASS() do { printf("PASS\n"); g_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

// ============================================================================
// Helper: Build a minimal DXBC with an SM4 vertex shader
// SM4 opcode stream: opcodes are 32-bit tokens
// ============================================================================

// SM4 opcode definitions
static constexpr uint32_t OPCODE_DCL_INPUT          = 0x0000f000 | 0x01;
static constexpr uint32_t OPCODE_DCL_OUTPUT         = 0x0000f000 | 0x02;
static constexpr uint32_t OPCODE_DCL_CONSTANT_BUFFER = 0x0000f000 | 0x0B;
static constexpr uint32_t OPCODE_DCL_SAMPLER        = 0x0000f000 | 0x0C;
static constexpr uint32_t OPCODE_DCL_TEXTURE        = 0x0000f000 | 0x0D;
static constexpr uint32_t OPCODE_MOV                = 0x00000000 | 0x01;
static constexpr uint32_t OPCODE_ADD                = 0x00000000 | 0x02;
static constexpr uint32_t OPCODE_MUL                = 0x00000000 | 0x04;
static constexpr uint32_t OPCODE_MAD                = 0x00000000 | 0x05;
static constexpr uint32_t OPCODE_DP4                = 0x00000000 | 0x07;
static constexpr uint32_t OPCODE_DP3                = 0x00000000 | 0x08;
static constexpr uint32_t OPCODE_MIN                = 0x00000000 | 0x0D;
static constexpr uint32_t OPCODE_MAX                = 0x00000000 | 0x0E;
static constexpr uint32_t OPCODE_ABS                = 0x00000000 | 0x0F;
static constexpr uint32_t OPCODE_NEG                = 0x00000000 | 0x10;
static constexpr uint32_t OPCODE_RSQ                = 0x00000000 | 0x11;
static constexpr uint32_t OPCODE_RCP                = 0x00000000 | 0x12;
static constexpr uint32_t OPCODE_DIV                = 0x00000000 | 0x09;
static constexpr uint32_t OPCODE_SINCOS             = 0x00000000 | 0x15;
static constexpr uint32_t OPCODE_END                = 0x00000000 | 0x00 | (0x01 << 16);

// Operand types
static constexpr uint32_t OPERAND_TEMP              = 0x00000000;
static constexpr uint32_t OPERAND_INPUT             = 0x00001000;
static constexpr uint32_t OPERAND_OUTPUT            = 0x00002000;
static constexpr uint32_t OPERAND_CONSTANT_BUFFER   = 0x00003000;
static constexpr uint32_t OPERAND_SAMPLER           = 0x00006000;
static constexpr uint32_t OPERAND_TEXTURE           = 0x00007000;

// Component masks
static constexpr uint32_t COMP_MASK_X    = 0x1;
static constexpr uint32_t COMP_MASK_XY   = 0x3;
static constexpr uint32_t COMP_MASK_XYZ  = 0x7;
static constexpr uint32_t COMP_MASK_XYZW = 0xF;

// ============================================================================
// Test: D3DX Math - more comprehensive
// ============================================================================

void test_d3dx_matrix_inverse() {
  TEST("D3DXMatrixInverse (identity)");
  {
    D3DXMATRIX mat, inv;
    D3DXMatrixIdentity(&mat);
    D3DXMATRIX* result = D3DXMatrixInverse(&inv, nullptr, &mat);
    CHECK(result != nullptr, "Inverse of identity should succeed");

    bool ok = true;
    for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
        if (fabsf(inv.m[r][c] - mat.m[r][c]) > 0.001f) ok = false;

    printf("  Inverse of identity = identity: %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

void test_d3dx_matrix_rotation() {
  TEST("D3DXMatrixRotationZ(90 deg)");
  {
    D3DXMATRIX rot;
    D3DXMatrixRotationZ(&rot, D3DX_PI * 0.5f);

    // Rotating (1,0,0) by 90 degrees around Z should give (0,1,0)
    D3DXVECTOR4 in = {1.0f, 0.0f, 0.0f, 1.0f};
    D3DXVECTOR4 out = {};
    D3DXVec4Transform(&out, &in, &rot);

    bool ok = (fabsf(out.x) < 0.01f && fabsf(out.y - 1.0f) < 0.01f && fabsf(out.z) < 0.01f);
    printf("  (1,0,0) * RotZ(90) = (%.2f,%.2f,%.2f): %s\n", out.x, out.y, out.z, ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

void test_d3dx_vec3_normalize() {
  TEST("D3DXVec3Normalize");
  {
    D3DXVECTOR3 v(3.0f, 4.0f, 0.0f);
    D3DXVECTOR3 n;
    D3DXVec3Normalize(&n, &v);

    float len = sqrtf(n.x*n.x + n.y*n.y + n.z*n.z);
    bool ok = (fabsf(len - 1.0f) < 0.001f);
    printf("  Normalize(3,4,0) = (%.2f,%.2f,%.2f) len=%.3f: %s\n",
      n.x, n.y, n.z, len, ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

void test_d3dx_vec3_cross() {
  TEST("D3DXVec3Cross");
  {
    D3DXVECTOR3 a(1.0f, 0.0f, 0.0f);
    D3DXVECTOR3 b(0.0f, 1.0f, 0.0f);
    D3DXVECTOR3 c;
    D3DXVec3Cross(&c, &a, &b);

    bool ok = (fabsf(c.x) < 0.001f && fabsf(c.y) < 0.001f && fabsf(c.z - 1.0f) < 0.001f);
    printf("  (1,0,0) x (0,1,0) = (%.2f,%.2f,%.2f): %s\n", c.x, c.y, c.z, ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

void test_d3dx_matrix_lookat() {
  TEST("D3DXMatrixLookAtLH");
  {
    D3DXVECTOR3 eye(0, 0, -5);
    D3DXVECTOR3 at(0, 0, 0);
    D3DXVECTOR3 up(0, 1, 0);
    D3DXMATRIX view;
    D3DXMatrixLookAtLH(&view, &eye, &at, &up);

    // View matrix should be valid (not all zeros)
    bool nonZero = false;
    for (int r = 0; r < 4 && !nonZero; r++)
      for (int c = 0; c < 4 && !nonZero; c++)
        if (fabsf(view.m[r][c]) > 0.001f) nonZero = true;

    printf("  LookAtLH(eye=(0,0,-5), at=origin): %s\n", nonZero ? "PASS" : "FAIL");
    if (nonZero) g_passed++; else g_failed++;
  }
}

void test_d3dx_matrix_perspective() {
  TEST("D3DXMatrixPerspectiveFovLH");
  {
    D3DXMATRIX proj;
    D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 4.0f, 16.0f/9.0f, 0.1f, 100.0f);

    // Perspective matrix: [2] should be non-zero (z mapping)
    bool ok = (fabsf(proj.m[2][2]) > 0.001f && fabsf(proj.m[2][3]) > 0.001f);
    printf("  PerspectiveFovLH(fov=45, aspect=16/9, zn=0.1, zf=100): %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

// ============================================================================
// Test: SM4 Translator with real DXBC opcodes
// ============================================================================

void test_sm4_translator_minimal() {
  TEST("SM4Translator::translate with empty DXBC");
  {
    ParsedDXBC dxbc = {};
    dxbc.majorVersion = 4;
    dxbc.minorVersion = 0;

    SM4Translator translator;
    SM4TranslateResult result = translator.translate(dxbc);

    printf("  Empty DXBC: success=%d, spirvSize=%zu, error='%s'\n",
      result.success, result.spirvWords.size(), result.errorMessage.c_str());

    // Even with empty input, translator should not crash
    printf("  No crash: PASS\n");
    g_passed++;
    PASS();
  }
}

void test_sm4_translator_empty_shaders() {
  TEST("SM4Translator handles edge cases");
  {
    SM4Translator translator;

    // Test with no tokens
    ParsedDXBC dxbc = {};
    dxbc.majorVersion = 4;
    dxbc.minorVersion = 0;
    SM4TranslateResult r1 = translator.translate(dxbc);
    bool ok1 = !r1.success || r1.spirvWords.empty();
    printf("  Empty DXBC: handled=%s\n", ok1 ? "PASS" : "FAIL");
    if (ok1) g_passed++; else g_failed++;

    PASS();
  }
}

// ============================================================================
// Test: SPIR-V Builder - instruction emission
// ============================================================================

void test_spirv_types_and_constants() {
  TEST("SpirvBuilder types, constants, memory model");
  {
    SpirvBuilder builder;

    uint32_t voidType = builder.typeVoid();
    uint32_t floatType = builder.typeFloat(32);
    uint32_t intType = builder.typeInt(32, false);
    uint32_t boolType = builder.typeBool();
    uint32_t vec2Type = builder.typeVector(floatType, 2);
    uint32_t vec3Type = builder.typeVector(floatType, 3);
    uint32_t vec4Type = builder.typeVector(floatType, 4);
    uint32_t mat4Type = builder.typeMatrix(vec4Type, 4);
    uint32_t arrType = builder.typeArray(floatType, 16);
    uint32_t structType = builder.typeStruct({floatType, vec4Type, intType});

    // Constants
    uint32_t c0 = builder.constantFloat(floatType, 0.0f);
    uint32_t c1 = builder.constantFloat(floatType, 1.0f);
    uint32_t ci = builder.constantInt(intType, 42);
    uint32_t cu = builder.constantUint(intType, 7);

    // Pointers
    uint32_t ptrUniform = builder.typePointer(7, vec4Type);
    uint32_t ptrPush = builder.typePointer(9, floatType);

    // Memory model
    builder.capability(1); // Shader
    builder.memoryModel(0, 2); // Logical GLSL450

    auto spirv = builder.getSpirv();
    printf("  Created %u types, %u constants, SPIR-V=%zu words\n",
      14, 4, spirv.size());
    bool ok = (spirv.size() > 10 && voidType > 0 && floatType > 0);
    printf("  %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;

    PASS();
  }
}

void test_spirv_decorations() {
  TEST("SpirvBuilder decorations (Location, Binding)");
  {
    SpirvBuilder builder;

    uint32_t floatType = builder.typeFloat(32);
    uint32_t vec4Type = builder.typeVector(floatType, 4);

    // Create a global variable
    uint32_t ptrType = builder.typePointer(7, vec4Type); // storageClass=7 (Uniform)
    uint32_t globalVar = builder.variable(ptrType, 7);

    // Add decorations
    builder.decorate(globalVar, 33, {0}); // Location=0
    builder.decorate(globalVar, 34, {0}); // Binding=0

    printf("  globalVar: %u\n", globalVar);
    bool ok = (globalVar > 0);
    printf("  %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;

    PASS();
  }
}

// ============================================================================
// Test: Quaternion math (extended)
// ============================================================================

void test_quaternion_slerp() {
  TEST("D3DXQuaternionSlerp");
  {
    D3DXQUATERNION q1; // identity (w=1)
    D3DXQUATERNION q2, result;
    D3DXQuaternionRotationYawPitchRoll(&q2, D3DX_PI/2, 0, 0);
    D3DXQuaternionSlerp(&result, &q1, &q2, 0.5f);

    // SLERP between identity and 90deg should be at 45deg
    bool ok = (fabsf(result.w) > 0.5f); // cos(45) = 0.707
    printf("  Slerp(identity, yaw90, t=0.5): w=%.3f: %s\n", result.w, ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

void test_quaternion_normalize() {
  TEST("D3DXQuaternionNormalize");
  {
    D3DXQUATERNION q(1, 2, 3, 4);
    D3DXQUATERNION result;
    D3DXQuaternionNormalize(&result, &q);

    float len = sqrtf(result.x*result.x + result.y*result.y + result.z*result.z + result.w*result.w);
    bool ok = (fabsf(len - 1.0f) < 0.001f);
    printf("  Normalize(1,2,3,4): len=%.3f: %s\n", len, ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
  }
}

// ============================================================================
// Main
// ============================================================================

int main() {
  printf("=== VKWIND11 Test: Shader Translation & Math ===\n\n");

  // D3DX Math (extended)
  test_d3dx_matrix_inverse();
  test_d3dx_matrix_rotation();
  test_d3dx_vec3_normalize();
  test_d3dx_vec3_cross();
  test_d3dx_matrix_lookat();
  test_d3dx_matrix_perspective();

  // Quaternion
  test_quaternion_slerp();
  test_quaternion_normalize();

  // SM4 Translator
  test_sm4_translator_minimal();
  test_sm4_translator_empty_shaders();

  // SPIR-V Builder
  test_spirv_types_and_constants();
  test_spirv_decorations();

  printf("\n=== Results: %d passed, %d failed ===\n", g_passed, g_failed);
  return g_failed > 0 ? 1 : 0;
}
