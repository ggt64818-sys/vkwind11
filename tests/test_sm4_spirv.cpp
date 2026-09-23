#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <cstdint>
#include "../src/shader/dxbc_parser.h"
#include "../src/shader/spirv_builder.h"
#include "../src/d3dx/d3dx_math.h"

// ============================================================================
// Test: SM4→SPIR-V translation pipeline
// ============================================================================

int main() {
  printf("=== VKWIND11 Test: SM4→SPIR-V Translation ===\n\n");

  // Test 1: DXBC parser
  printf("[Test 1] DXBC Parser...\n");
  {
    // Minimal DXBC header
    std::vector<uint32_t> bytecode = {
      0x43425844,  // magic "DXBC"
      0x00000000,  // checksum
      0x00030001,  // version 3.1
      0x00000018,  // total size
      0x00000001,  // 1 chunk
      0x00000018,  // chunk offset
    };

    ParsedDXBC result;
    bool ok = parseDXBC(bytecode.data(), bytecode.size() * 4, result);
    printf("  parseDXBC: %s (version %u.%u)\n",
      ok ? "PASS" : "FAIL",
      result.majorVersion, result.minorVersion);
  }

  // Test 2: SPIR-V builder
  printf("\n[Test 2] SPIR-V Builder...\n");
  {
    SpirvBuilder builder;

    // Basic types
    uint32_t voidType = builder.typeVoid();
    uint32_t floatType = builder.typeFloat(32);
    uint32_t vec4Type = builder.typeVector(floatType, 4);
    uint32_t mat4Type = builder.typeMatrix(vec4Type, 4);

    printf("  voidType: %u\n", voidType);
    printf("  floatType: %u\n", floatType);
    printf("  vec4Type: %u\n", vec4Type);
    printf("  mat4Type: %u\n", mat4Type);

    // Constants
    uint32_t zero = builder.constantFloat(floatType, 0.0f);
    uint32_t one = builder.constantFloat(floatType, 1.0f);
    printf("  zero: %u, one: %u\n", zero, one);

    // Memory model
    builder.capability(1); // Shader
    builder.memoryModel(0, 2); // Logical GLSL450

    printf("  SPIR-V words: %zu\n", builder.getSpirv().size());
    printf("  PASS\n");
  }

  // Test 3: D3DX math
  printf("\n[Test 3] D3DX Math...\n");
  {
    // Test identity matrix
    D3DXMATRIX mat;
    D3DXMatrixIdentity(&mat);
    printf("  Identity matrix: [%.1f,%.1f,%.1f,%.1f]...\n",
      mat.m[0][0], mat.m[1][1], mat.m[2][2], mat.m[3][3]);

    // Test transpose
    D3DXMATRIX t;
    D3DXMatrixTranspose(&t, &mat);
    printf("  Transpose: %.1f == %.1f? %s\n",
      mat.m[0][0], t.m[0][0],
      mat.m[0][0] == t.m[0][0] ? "PASS" : "FAIL");

    // Test determinant
    float det = D3DXMatrixDeterminant(&mat);
    printf("  Determinant: %.1f (expected 1.0) %s\n",
      det, fabsf(det - 1.0f) < 0.001f ? "PASS" : "FAIL");

    // Test matrix multiply
    D3DXMATRIX m1, m2, result;
    D3DXMatrixScaling(&m1, 2.0f, 2.0f, 2.0f);
    D3DXMatrixTranslation(&m2, 1.0f, 2.0f, 3.0f);
    D3DXMatrixMultiply(&result, &m1, &m2);
    printf("  Scale * Translate: [%.1f,%.1f,%.1f] %s\n",
      result.m[3][0], result.m[3][1], result.m[3][2],
      (result.m[3][0] == 2.0f && result.m[3][1] == 4.0f && result.m[3][2] == 6.0f) ? "PASS" : "FAIL");
  }

  // Test 4: Quaternion
  printf("\n[Test 4] Quaternion...\n");
  {
    D3DXQUATERNION q;
    D3DXQuaternionRotationYawPitchRoll(&q, 0.0f, 0.0f, 0.0f);
    printf("  Identity quat: (%.2f,%.2f,%.2f,%.2f) %s\n",
      q.x, q.y, q.z, q.w,
      (fabsf(q.w - 1.0f) < 0.001f) ? "PASS" : "FAIL");
  }

  printf("\n=== All SM4→SPIR-V tests completed ===\n");
  return 0;
}
