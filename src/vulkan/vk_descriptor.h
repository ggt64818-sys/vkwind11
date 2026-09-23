#pragma once

#include "../../src/vulkan/vk_types.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <mutex>

// ============================================================================
// Vulkan Descriptor Set Management
// ============================================================================
//
// Manages descriptor pool, layouts, and sets for D3D11 resource binding.
// D3D11 slots map to Vulkan descriptor bindings.
//

struct VulkanDescriptorBinding {
  uint32_t binding;
  VkDescriptorType type;
  VkShaderStageFlags stageFlags;
  uint32_t count;
};

class VulkanDescriptorManager {
public:
  VulkanDescriptorManager() = default;
  ~VulkanDescriptorManager();

  void initialize(VkDevice device, uint32_t maxSets = 256);
  void shutdown();

  // Create descriptor set layout
  VkDescriptorSetLayout createLayout(const std::vector<VulkanDescriptorBinding>& bindings);

  // Allocate a descriptor set from the pool
  VkDescriptorSet allocateSet(VkDescriptorSetLayout layout);

  // Update a descriptor set
  void updateBuffer(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  void updateImage(VkDescriptorSet set, uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  // Reset pool for reuse
  void resetPool();

private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkDescriptorPool m_pool = VK_NULL_HANDLE;
  std::mutex m_mutex;
};
