#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>

// ============================================================================
// Vulkan Buffer
// ============================================================================

struct VulkanBuffer {
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkDeviceSize size = 0;
  VkBufferUsageFlags usage = 0;
  VkMemoryPropertyFlags memProps = 0;
  void* mapped = nullptr;

  void destroy(VkDevice device);
  bool isHostVisible() const { return (memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }
};

// ============================================================================
// Vulkan Image
// ============================================================================

struct VulkanImage {
  VkImage image = VK_NULL_HANDLE;
  VkImageView view = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
  VkFormat format = VK_FORMAT_UNDEFINED;
  uint32_t width = 0;
  uint32_t height = 0;
  uint32_t mipLevels = 1;
  uint32_t arrayLayers = 1;
  VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
  VkImageUsageFlags usage = 0;

  void destroy(VkDevice device);
};

// ============================================================================
// Vulkan Sampler
// ============================================================================

struct VulkanSampler {
  VkSampler sampler = VK_NULL_HANDLE;

  void destroy(VkDevice device);
};

// ============================================================================
// Vulkan Pipeline
// ============================================================================

struct VulkanPipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout layout = VK_NULL_HANDLE;

  void destroy(VkDevice device);
};

// ============================================================================
// Vulkan Render Pass
// ============================================================================

struct VulkanRenderPass {
  VkRenderPass renderPass = VK_NULL_HANDLE;

  void destroy(VkDevice device);
};

// ============================================================================
// Vulkan Framebuffer
// ============================================================================

struct VulkanFramebuffer {
  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  VkRenderPass renderPass = VK_NULL_HANDLE;

  void destroy(VkDevice device);
};

// ============================================================================
// Vulkan Swapchain
// ============================================================================

struct VulkanSwapchain {
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkFormat format = VK_FORMAT_UNDEFINED;
  VkExtent2D extent = {0, 0};
  std::vector<VkImage> images;
  std::vector<VkImageView> views;
  std::vector<VkFramebuffer> framebuffers;
  uint32_t currentImage = 0;

  void destroy(VkDevice device);
};

// ============================================================================
// Vulkan Descriptor Set
// ============================================================================

struct VulkanDescriptorSet {
  VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
  VkDescriptorPool pool = VK_NULL_HANDLE;
};

// ============================================================================
// Vulkan Query Pool
// ============================================================================

struct VulkanQueryPool {
  VkQueryPool queryPool = VK_NULL_HANDLE;
  VkQueryType type = VK_QUERY_TYPE_OCCLUSION;
  uint32_t count = 0;

  void destroy(VkDevice device);
};
