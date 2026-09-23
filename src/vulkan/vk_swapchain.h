#pragma once

#include "../../src/vulkan/vk_types.h"
#include <vulkan/vulkan.h>
#include <vector>

// ============================================================================
// Vulkan Swapchain Management
// ============================================================================
//
// Manages the swapchain for D3D11 presentation.
// Maps D3D11 swapchain creation to Vulkan surface/swapchain.
//

struct VulkanSwapchainSupport {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanSwapchainCreateOptions {
  uint32_t width = 800;
  uint32_t height = 600;
  VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_UNORM;
  VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
};

class SwapchainManager {
public:
  SwapchainManager() = default;
  ~SwapchainManager();

  void initialize(VkPhysicalDevice physicalDevice, VkDevice device);
  void shutdown();

  bool create(const VulkanSwapchainCreateOptions& options);
  void destroy();

  VkResult acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex);
  VkResult present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);

  // Getters
  VkSwapchainKHR getSwapchain() const { return m_swapchain; }
  VkFormat getImageFormat() const { return m_format; }
  VkExtent2D getExtent() const { return m_extent; }
  const std::vector<VkImage>& getImages() const { return m_images; }
  const std::vector<VkImageView>& getImageViews() const { return m_imageViews; }
  VkRenderPass getRenderPass() const { return m_renderPass; }

  // Query support
  VulkanSwapchainSupport querySupport(VkSurfaceKHR surface);

private:
  VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
  VkDevice m_device = VK_NULL_HANDLE;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
  VkSurfaceKHR m_surface = VK_NULL_HANDLE;
  VkFormat m_format = VK_FORMAT_UNDEFINED;
  VkExtent2D m_extent = {};
  VkRenderPass m_renderPass = VK_NULL_HANDLE;

  std::vector<VkImage> m_images;
  std::vector<VkImageView> m_imageViews;

  void createRenderPass();
  void cleanup();
};
