#include "vk_cmd_buffer.h"
#include "../common/logging.h"

// ============================================================================
// VulkanCommandBufferManager Implementation
// ============================================================================

VulkanCommandBufferManager::~VulkanCommandBufferManager() {
  shutdown();
}

void VulkanCommandBufferManager::initialize(VkDevice device, VkCommandPool commandPool) {
  m_device = device;
  m_commandPool = commandPool;
  m_nextIndex = 0;

  // Pre-allocate command buffers
  const uint32_t BUFFER_COUNT = 16;
  m_commandBuffers.resize(BUFFER_COUNT);

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = BUFFER_COUNT;

  VkResult result = vkAllocateCommandBuffers(device, &allocInfo, m_commandBuffers.data());
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to allocate command buffers: %d", result);
  } else {
    VKWIND11_LOG_INFO("Allocated %u command buffers", BUFFER_COUNT);
  }
}

void VulkanCommandBufferManager::shutdown() {
  if (m_device && m_commandPool && !m_commandBuffers.empty()) {
    vkFreeCommandBuffers(m_device, m_commandPool, (uint32_t)m_commandBuffers.size(), m_commandBuffers.data());
    m_commandBuffers.clear();
  }
  m_device = VK_NULL_HANDLE;
  m_commandPool = VK_NULL_HANDLE;
}

VulkanCommandBuffer* VulkanCommandBufferManager::acquireBuffer() {
  // TODO: proper ring buffer with fences for async submission
  static thread_local VulkanCommandBuffer s_buffer;

  if (m_nextIndex >= m_commandBuffers.size()) {
    m_nextIndex = 0;
  }

  s_buffer.cmdBuffer = m_commandBuffers[m_nextIndex++];
  s_buffer.recording = false;

  return &s_buffer;
}

VkResult VulkanCommandBufferManager::submit(
  VkQueue queue,
  VulkanCommandBuffer* buffer,
  VkSemaphore waitSemaphore,
  VkSemaphore signalSemaphore,
  VkFence fence
) {
  if (!buffer || !buffer->cmdBuffer) return VK_ERROR_INITIALIZATION_FAILED;

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &buffer->cmdBuffer;

  if (waitSemaphore) {
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submitInfo.pWaitDstStageMask = &waitStage;
  }

  if (signalSemaphore) {
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;
  }

  VkResult result = vkQueueSubmit(queue, 1, &submitInfo, fence);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("vkQueueSubmit failed: %d", result);
  }

  return result;
}

void VulkanCommandBufferManager::reset() {
  if (m_commandPool) {
    vkResetCommandPool(m_device, m_commandPool, 0);
    m_nextIndex = 0;
  }
}
