#include "vk_swapchain.h"
#include "../common/logging.h"
#include <algorithm>

// ============================================================================
// SwapchainManager Implementation
// ============================================================================

SwapchainManager::~SwapchainManager() {
  shutdown();
}

void SwapchainManager::initialize(VkPhysicalDevice physicalDevice, VkDevice device) {
  m_physicalDevice = physicalDevice;
  m_device = device;
}

void SwapchainManager::shutdown() {
  destroy();
  m_physicalDevice = VK_NULL_HANDLE;
  m_device = VK_NULL_HANDLE;
}

void SwapchainManager::cleanup() {
  for (auto imageView : m_imageViews) {
    if (imageView) vkDestroyImageView(m_device, imageView, nullptr);
  }
  m_imageViews.clear();
  m_images.clear();

  if (m_renderPass) {
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;
  }

  if (m_swapchain) {
    vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
    m_swapchain = VK_NULL_HANDLE;
  }
}

void SwapchainManager::destroy() {
  cleanup();
}

bool SwapchainManager::create(const VulkanSwapchainCreateOptions& options) {
  cleanup();

  m_surface = options.surface;
  m_extent = { options.width, options.height };

  // Query swapchain support
  VulkanSwapchainSupport support = querySupport(options.surface);

  // Choose surface format
  VkSurfaceFormatKHR chosenFormat = support.formats[0];
  for (auto& fmt : support.formats) {
    if (fmt.format == options.preferredFormat && fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      chosenFormat = fmt;
      break;
    }
  }
  m_format = chosenFormat.format;

  // Choose present mode
  VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (auto& mode : support.presentModes) {
    if (mode == options.preferredPresentMode) {
      chosenPresentMode = mode;
      break;
    }
  }

  // Create swapchain
  VkSwapchainCreateInfoKHR swapchainInfo = {};
  swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainInfo.surface = m_surface;
  swapchainInfo.minImageCount = 3;
  swapchainInfo.imageFormat = m_format;
  swapchainInfo.imageColorSpace = chosenFormat.colorSpace;
  swapchainInfo.imageExtent = m_extent;
  swapchainInfo.imageArrayLayers = 1;
  swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapchainInfo.presentMode = chosenPresentMode;
  swapchainInfo.clipped = VK_TRUE;
  swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(m_device, &swapchainInfo, nullptr, &m_swapchain);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create swapchain: %d", result);
    return false;
  }

  // Get images
  uint32_t imageCount = 0;
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
  m_images.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

  // Create image views
  m_imageViews.resize(imageCount);
  for (uint32_t i = 0; i < imageCount; i++) {
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_images[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(m_device, &viewInfo, nullptr, &m_imageViews[i]);
    if (result != VK_SUCCESS) {
      VKWIND11_LOG_ERROR("Failed to create swapchain image view %u: %d", i, result);
      cleanup();
      return false;
    }
  }

  createRenderPass();

  VKWIND11_LOG_INFO("Swapchain created: %ux%u, %u images", m_extent.width, m_extent.height, imageCount);
  return true;
}

void SwapchainManager::createRenderPass() {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = m_format;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorRef = {};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments = &colorAttachment;
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  VkResult result = vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create render pass: %d", result);
  }
}

VulkanSwapchainSupport SwapchainManager::querySupport(VkSurfaceKHR surface) {
  VulkanSwapchainSupport support;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface, &support.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &formatCount, nullptr);
  if (formatCount > 0) {
    support.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &formatCount, support.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, nullptr);
  if (presentModeCount > 0) {
    support.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, support.presentModes.data());
  }

  return support;
}

VkResult SwapchainManager::acquireNextImage(VkSemaphore semaphore, uint32_t* imageIndex) {
  return vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, imageIndex);
}

VkResult SwapchainManager::present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore) {
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &waitSemaphore;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &m_swapchain;
  presentInfo.pImageIndices = &imageIndex;

  return vkQueuePresentKHR(queue, &presentInfo);
}
