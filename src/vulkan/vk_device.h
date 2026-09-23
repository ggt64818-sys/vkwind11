#pragma once

#include "vk_types.h"
#include "../common/config.h"
#include <vector>
#include <memory>
#include <mutex>

// ============================================================================
// Vulkan Device — wraps VkDevice + VkPhysicalDevice
// ============================================================================

class VulkanDevice {
public:
  VulkanDevice();
  ~VulkanDevice();

  bool initialize(bool headless = false);
  void destroy();

  // Device info
  VkDevice getDevice() const { return m_device; }
  VkPhysicalDevice getPhysicalDevice() const { return m_physicalDevice; }
  VkInstance getInstance() const { return m_instance; }
  VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
  VkQueue getPresentQueue() const { return m_presentQueue; }
  uint32_t getGraphicsQueueFamily() const { return m_graphicsQueueFamily; }
  uint32_t getPresentQueueFamily() const { return m_presentQueueFamily; }

  // Memory
  uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
  bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VulkanBuffer& out);
  bool createImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers,
                   VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags memProps, VulkanImage& out);

  // Sync objects
  VkSemaphore createSemaphore();
  VkFence createFence(bool signaled = true);

  // Command buffers
  VkCommandPool createCommandPool(uint32_t queueFamilyIndex);
  VkCommandBuffer allocateCommandBuffer(VkCommandPool pool, bool primary = true);

  // Descriptors
  VkDescriptorSetLayout createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);
  VkDescriptorPool createDescriptorPool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes);
  VkDescriptorSet allocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);

  // Pipelines
  VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout layout, uint32_t pushConstantSize = 0);
  VkPipeline createGraphicsPipeline(VkPipelineLayout layout, VkRenderPass renderPass, const VkGraphicsPipelineCreateInfo& info);
  VkPipeline createComputePipeline(VkPipelineLayout layout, const VkComputePipelineCreateInfo& info);

  // Render pass
  VkRenderPass createRenderPass(const VkRenderPassCreateInfo& info);

  // Framebuffer
  VkFramebuffer createFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height, uint32_t layers = 1);

  // Shader modules
  VkShaderModule createShaderModule(const uint32_t* code, size_t size);

  // Swapchain
  bool createSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height, VulkanSwapchain& out);
  uint32_t acquireNextImage(VkSemaphore signalSemaphore, VulkanSwapchain& swapchain);

  // Immediate submit
  bool submitImmediate(VkCommandBuffer cmd);

  // Features
  bool supportsCompute() const { return m_computeQueue != VK_NULL_HANDLE; }

private:
  VkInstance m_instance = VK_NULL_HANDLE;
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;

  uint32_t m_graphicsQueueFamily = 0;
  uint32_t m_presentQueueFamily = 0;
  uint32_t m_computeQueueFamily = 0;
  VkQueue m_graphicsQueue = VK_NULL_HANDLE;
  VkQueue m_presentQueue = VK_NULL_HANDLE;
  VkQueue m_computeQueue = VK_NULL_HANDLE;

  VkCommandPool m_immCommandPool = VK_NULL_HANDLE;
  VkCommandBuffer m_immCommandBuffer = VK_NULL_HANDLE;
  VkFence m_immFence = VK_NULL_HANDLE;

  std::string m_debugName;
  bool m_validationEnabled = false;
};
