#pragma once

#include "../../src/vulkan/vk_types.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <mutex>

// ============================================================================
// Vulkan Command Buffer Management
// ============================================================================
//
// Manages command buffer allocation, recording, and submission.
// Used by the D3D11 context for draw commands.
//

struct VulkanCommandBuffer {
  VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
  VkCommandBufferBeginInfo beginInfo = {};
  bool recording = false;
};

class VulkanCommandBufferManager {
public:
  VulkanCommandBufferManager() = default;
  ~VulkanCommandBufferManager();

  void initialize(VkDevice device, VkCommandPool commandPool);
  void shutdown();

  // Get a command buffer for recording
  VulkanCommandBuffer* acquireBuffer();

  // Submit recorded commands
  VkResult submit(VkQueue queue, VulkanCommandBuffer* buffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence);

  // Reset all command buffers for reuse
  void reset();

  VkCommandPool getCommandPool() const { return m_commandPool; }

private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkCommandPool m_commandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> m_commandBuffers;
  std::mutex m_mutex;
  uint32_t m_nextIndex = 0;
};
