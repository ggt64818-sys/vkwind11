#pragma once

#include "../../src/vulkan/vk_types.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <mutex>

// ============================================================================
// Vulkan Memory Allocator (Manual, no VMA for now)
// ============================================================================
//
// Simple memory allocation wrapper around Vulkan memory types.
// For Phase 1 we use vkAllocateMemory directly.
// Phase 3+ can switch to VMA or keep this.
//

struct VulkanMemoryBlock {
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkDeviceSize size = 0;
  VkDeviceSize offset = 0;
  uint32_t memoryTypeIndex = 0;
  void* mappedData = nullptr;
};

class VulkanMemoryAllocator {
public:
  VulkanMemoryAllocator() = default;
  ~VulkanMemoryAllocator();

  void initialize(VkPhysicalDevice physicalDevice, VkDevice device);
  void shutdown();

  // Allocate a block of device-local memory
  VulkanMemoryBlock allocateDeviceLocal(VkDeviceSize size, VkDeviceSize alignment = 256);

  // Allocate a block of host-visible memory (for staging)
  VulkanMemoryBlock allocateHostVisible(VkDeviceSize size, VkDeviceSize alignment = 256);

  // Map host-visible memory for CPU access
  void* mapMemory(VulkanMemoryBlock& block);
  void unmapMemory(VulkanMemoryBlock& block);

  // Free a block
  void free(VulkanMemoryBlock& block);

  // Get memory properties
  VkPhysicalDeviceMemoryProperties getMemoryProperties() const { return m_memoryProperties; }
  bool isMappable(uint32_t memoryTypeIndex) const;

private:
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkPhysicalDeviceMemoryProperties m_memoryProperties = {};

  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
};
