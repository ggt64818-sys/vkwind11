#include <cstdio>
#include "../src/vulkan/vk_device.h"
#include "../src/vulkan/vk_pipeline.h"
#include "../src/vulkan/vk_descriptor.h"
#include "../src/vulkan/vk_memory.h"

// ============================================================================
// Test: Pipeline Integration
// ============================================================================

int main() {
  printf("=== VKWIND11 Test: Pipeline Integration ===\n\n");

  // Test 1: VulkanDevice
  printf("[Test 1] VulkanDevice...\n");
  {
    VulkanDevice device;
    bool ok = device.initialize(false);
    printf("  Init: %s\n", ok ? "PASS" : "FAIL (no Vulkan?)");
    if (ok) {
      printf("  Device created successfully\n");
      device.destroy();
    }
  }

  // Test 2: PipelineManager
  printf("\n[Test 2] PipelineManager...\n");
  {
    VulkanDevice device;
    if (device.initialize(false)) {
      VulkanPipelineManager pipelineManager;
      pipelineManager.initialize(device.getDevice());

      PipelineKey key;
      VkPipeline pipeline = pipelineManager.getOrCreateGraphicsPipeline(key);
      printf("  Pipeline: %p (stub returns VK_NULL_HANDLE)\n", (void*)pipeline);

      pipelineManager.shutdown();
      device.destroy();
    } else {
      printf("  SKIP (no Vulkan)\n");
    }
  }

  // Test 3: DescriptorManager
  printf("\n[Test 3] DescriptorManager...\n");
  {
    VulkanDevice device;
    if (device.initialize(false)) {
      VulkanDescriptorManager descriptorManager;
      descriptorManager.initialize(device.getDevice());

      // Create a layout with one uniform buffer
      std::vector<VulkanDescriptorBinding> bindings = {
        { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1 }
      };
      VkDescriptorSetLayout layout = descriptorManager.createLayout(bindings);
      printf("  Layout: %p %s\n", (void*)layout, layout ? "PASS" : "FAIL");

      if (layout) {
        VkDescriptorSet set = descriptorManager.allocateSet(layout);
        printf("  Set: %p %s\n", (void*)set, set ? "PASS" : "FAIL");
      }

      descriptorManager.shutdown();
      device.destroy();
    } else {
      printf("  SKIP (no Vulkan)\n");
    }
  }

  // Test 4: Memory allocator
  printf("\n[Test 4] MemoryAllocator...\n");
  {
    VulkanDevice device;
    if (device.initialize(false)) {
      VulkanMemoryAllocator memAlloc;
      memAlloc.initialize(device.getPhysicalDevice(), device.getDevice());

      auto block = memAlloc.allocateHostVisible(4096);
      printf("  Host memory: %p size=%llu %s\n",
        (void*)block.memory, block.size, block.memory ? "PASS" : "FAIL");

      if (block.memory) {
        void* mapped = memAlloc.mapMemory(block);
        printf("  Mapped: %p %s\n", mapped, mapped ? "PASS" : "FAIL");
        memAlloc.unmapMemory(block);
        memAlloc.free(block);
      }

      memAlloc.shutdown();
      device.destroy();
    } else {
      printf("  SKIP (no Vulkan)\n");
    }
  }

  printf("\n=== All pipeline integration tests completed ===\n");
  return 0;
}
