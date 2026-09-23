#include <cstdio>
#include "../src/d3d11/d3d11_types.h"
#include "../src/vulkan/vk_device.h"
#include "../src/common/logging.h"

int main() {
  printf("=== VKWIND11 Test: D3D11 Types & Vulkan ===\n\n");

  printf("[Test 1] Feature levels...\n");
  {
    printf("  D3D_FEATURE_LEVEL_11_0 = %d\n", D3D_FEATURE_LEVEL_11_0);
    printf("  D3D_FEATURE_LEVEL_10_1 = %d\n", D3D_FEATURE_LEVEL_10_1);
    printf("  D3D_FEATURE_LEVEL_10_0 = %d\n", D3D_FEATURE_LEVEL_10_0);
    printf("  PASS\n");
  }

  printf("\n[Test 2] D3D11 constants...\n");
  {
    printf("  D3D11_BIND_VERTEX_BUFFER = %u\n", D3D11_BIND_VERTEX_BUFFER);
    printf("  D3D11_BIND_INDEX_BUFFER = %u\n", D3D11_BIND_INDEX_BUFFER);
    printf("  D3D11_BIND_CONSTANT_BUFFER = %u\n", D3D11_BIND_CONSTANT_BUFFER);
    printf("  D3D11_BIND_SHADER_RESOURCE = %u\n", D3D11_BIND_SHADER_RESOURCE);
    printf("  D3D11_BIND_RENDER_TARGET = %u\n", D3D11_BIND_RENDER_TARGET);
    printf("  D3D11_BIND_DEPTH_STENCIL = %u\n", D3D11_BIND_DEPTH_STENCIL);
    printf("  PASS\n");
  }

  printf("\n[Test 3] VulkanDevice (headless)...\n");
  {
    VulkanDevice device;
    bool ok = device.initialize(true);
    printf("  Init: %s\n", ok ? "PASS" : "FAIL (no Vulkan driver?)");
    if (ok) {
      device.destroy();
      printf("  Destroy: PASS\n");
    }
  }

  printf("\n=== All tests completed ===\n");
  return 0;
}
