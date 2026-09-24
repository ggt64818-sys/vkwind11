#include "vk_device.h"
#include "../common/logging.h"
#include <cstring>
#include <set>
#include <algorithm>
#include <vector>

#if defined(_WIN32) && !defined(VK_USE_PLATFORM_WIN32_KHR_DEFINED)
// We need VK_KHR_WIN32_SURFACE_EXTENSION_NAME but can't include <windows.h>
// Define the extension name directly
#ifndef VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"
#endif
#define VK_USE_PLATFORM_WIN32_KHR_DEFINED
#elif defined(__linux__) && !defined(__ANDROID__)
#define VK_USE_PLATFORM_XLIB_KHR
#include <vulkan/vulkan_xlib.h>
#elif defined(__ANDROID__)
#include <vulkan/vulkan_android.h>
#endif

// ============================================================================
// VulkanDevice
// ============================================================================

VulkanDevice::VulkanDevice() = default;

VulkanDevice::~VulkanDevice() {
  destroy();
}

bool VulkanDevice::initialize(bool headless) {
  // Create VkInstance
  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "VKWIND11";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.pEngineName = "VKWIND11";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  appInfo.apiVersion = VK_API_VERSION_1_1;

  const char* extensions[3] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef __ANDROID__
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(__linux__)
    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif
  };

  uint32_t extensionCount = 0;
  extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
  extensionCount = 1;
#ifdef __ANDROID__
  extensions[extensionCount++] = VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif defined(_WIN32)
  extensions[extensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(__linux__)
  extensions[extensionCount++] = VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#endif

  const char* layers[1] = {};
  uint32_t layerCount = 0;

  if (g_config.strict_validation) {
    layers[layerCount++] = "VK_LAYER_KHRONOS_validation";
  }

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = headless ? 0 : extensionCount;
  createInfo.ppEnabledExtensionNames = headless ? nullptr : extensions;
  createInfo.enabledLayerCount = layerCount;
  createInfo.ppEnabledLayerNames = layers;

  VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create VkInstance: %d", result);
    return false;
  }

  VKWIND11_LOG_INFO("VkInstance created");

  // Enumerate physical devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    VKWIND11_LOG_ERROR("No Vulkan physical devices found");
    return false;
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

  // Pick first discrete GPU, or first available
  m_physicalDevice = devices[0];
  for (auto& dev : devices) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(dev, &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      m_physicalDevice = dev;
      break;
    }
  }

  VkPhysicalDeviceProperties physProps;
  vkGetPhysicalDeviceProperties(m_physicalDevice, &physProps);
  m_debugName = physProps.deviceName;
  VKWIND11_LOG_INFO("Using GPU: %s (API %d.%d.%d)", m_debugName.c_str(),
    VK_VERSION_MAJOR(physProps.apiVersion),
    VK_VERSION_MINOR(physProps.apiVersion),
    VK_VERSION_PATCH(physProps.apiVersion));

  // Find queue families
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

  bool foundGraphics = false;
  bool foundPresent = false;

  for (uint32_t i = 0; i < queueFamilyCount; i++) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      m_graphicsQueueFamily = i;
      foundGraphics = true;
    }
    if (!headless) {
      // For Android, present queue is same as graphics
      m_presentQueueFamily = i;
      foundPresent = true;
    }
    if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      m_computeQueueFamily = i;
    }
  }

  if (!foundGraphics) {
    VKWIND11_LOG_ERROR("No graphics queue family found");
    return false;
  }

  // Create logical device
  float queuePriority = 1.0f;
  std::vector<VkDeviceQueueCreateInfo> queueInfos;

  VkDeviceQueueCreateInfo graphicsQueueInfo{};
  graphicsQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  graphicsQueueInfo.queueFamilyIndex = m_graphicsQueueFamily;
  graphicsQueueInfo.queueCount = 1;
  graphicsQueueInfo.pQueuePriorities = &queuePriority;
  queueInfos.push_back(graphicsQueueInfo);

  if (m_presentQueueFamily != m_graphicsQueueFamily) {
    VkDeviceQueueCreateInfo presentQueueInfo{};
    presentQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    presentQueueInfo.queueFamilyIndex = m_presentQueueFamily;
    presentQueueInfo.queueCount = 1;
    presentQueueInfo.pQueuePriorities = &queuePriority;
    queueInfos.push_back(presentQueueInfo);
  }

  const char* deviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  };

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;
  deviceFeatures.fillModeNonSolid = VK_TRUE;
  deviceFeatures.wideLines = VK_TRUE;
  deviceFeatures.depthClamp = VK_TRUE;
  deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;
  deviceFeatures.multiDrawIndirect = VK_TRUE;

  VkDeviceCreateInfo deviceCreateInfo{};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueInfos.data();
  deviceCreateInfo.enabledExtensionCount = headless ? 0 : 1;
  deviceCreateInfo.ppEnabledExtensionNames = headless ? nullptr : deviceExtensions;
  deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

  result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create VkDevice: %d", result);
    return false;
  }

  VKWIND11_LOG_INFO("VkDevice created");

  // Get queues
  vkGetDeviceQueue(m_device, m_graphicsQueueFamily, 0, &m_graphicsQueue);
  if (m_presentQueueFamily != m_graphicsQueueFamily) {
    vkGetDeviceQueue(m_device, m_presentQueueFamily, 0, &m_presentQueue);
  } else {
    m_presentQueue = m_graphicsQueue;
  }

  // Create immediate sync objects
  m_immCommandPool = createCommandPool(m_graphicsQueueFamily);
  m_immCommandBuffer = allocateCommandBuffer(m_immCommandPool);
  m_immFence = createFence(true);

  VKWIND11_LOG_INFO("VulkanDevice initialized successfully");
  return true;
}

void VulkanDevice::destroy() {
  if (!m_device) return;

  vkDeviceWaitIdle(m_device);

  if (m_immFence) vkDestroyFence(m_device, m_immFence, nullptr);
  if (m_immCommandPool) vkDestroyCommandPool(m_device, m_immCommandPool, nullptr);

  vkDestroyDevice(m_device, nullptr);
  m_device = VK_NULL_HANDLE;

  if (m_instance) {
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
  }
}

uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties memProps;
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProps);

  for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  return 0xFFFFFFFF;
}

bool VulkanDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProps, VulkanBuffer& out) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &out.buffer);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create buffer: %d", result);
    return false;
  }

  VkMemoryRequirements memReqs;
  vkGetBufferMemoryRequirements(m_device, out.buffer, &memReqs);

  VkPhysicalDeviceMemoryProperties memProps2;
  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProps2);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, memProps);

  result = vkAllocateMemory(m_device, &allocInfo, nullptr, &out.memory);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to allocate buffer memory: %d", result);
    vkDestroyBuffer(m_device, out.buffer, nullptr);
    out.buffer = VK_NULL_HANDLE;
    return false;
  }

  vkBindBufferMemory(m_device, out.buffer, out.memory, 0);

  out.size = size;
  out.usage = usage;
  out.memProps = memProps;

  if (memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
    vkMapMemory(m_device, out.memory, 0, size, 0, &out.mapped);
  }

  return true;
}

bool VulkanDevice::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, uint32_t arrayLayers,
                               VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage,
                               VkMemoryPropertyFlags memProps, VulkanImage& out) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent = {width, height, 1};
  imageInfo.mipLevels = mipLevels;
  imageInfo.arrayLayers = arrayLayers;
  imageInfo.format = format;
  // LINEAR tiling is required for HOST_VISIBLE memory (staging textures)
  imageInfo.tiling = (memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                     ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = samples;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult result = vkCreateImage(m_device, &imageInfo, nullptr, &out.image);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create image: %d", result);
    return false;
  }

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(m_device, out.image, &memReqs);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memReqs.size;
  allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, memProps);

  result = vkAllocateMemory(m_device, &allocInfo, nullptr, &out.memory);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to allocate image memory: %d", result);
    vkDestroyImage(m_device, out.image, nullptr);
    out.image = VK_NULL_HANDLE;
    return false;
  }

  vkBindImageMemory(m_device, out.image, out.memory, 0);

  out.format = format;
  out.width = width;
  out.height = height;
  out.mipLevels = mipLevels;
  out.arrayLayers = arrayLayers;
  out.samples = samples;
  out.usage = usage;

  // Create default image view
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = out.image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = mipLevels;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = arrayLayers;

  if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
      viewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
  }

  result = vkCreateImageView(m_device, &viewInfo, nullptr, &out.view);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create image view: %d", result);
  }

  return true;
}

VkSemaphore VulkanDevice::createSemaphore() {
  VkSemaphoreCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  VkSemaphore sem;
  vkCreateSemaphore(m_device, &info, nullptr, &sem);
  return sem;
}

VkFence VulkanDevice::createFence(bool signaled) {
  VkFenceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  if (signaled) info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VkFence fence;
  vkCreateFence(m_device, &info, nullptr, &fence);
  return fence;
}

VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex) {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  info.queueFamilyIndex = queueFamilyIndex;
  VkCommandPool pool;
  vkCreateCommandPool(m_device, &info, nullptr, &pool);
  return pool;
}

VkCommandBuffer VulkanDevice::allocateCommandBuffer(VkCommandPool pool, bool primary) {
  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandPool = pool;
  info.level = primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
  info.commandBufferCount = 1;
  VkCommandBuffer cmd;
  vkAllocateCommandBuffers(m_device, &info, &cmd);
  return cmd;
}

VkDescriptorSetLayout VulkanDevice::createDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
  VkDescriptorSetLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  info.bindingCount = static_cast<uint32_t>(bindings.size());
  info.pBindings = bindings.data();
  VkDescriptorSetLayout layout;
  vkCreateDescriptorSetLayout(m_device, &info, nullptr, &layout);
  return layout;
}

VkDescriptorPool VulkanDevice::createDescriptorPool(uint32_t maxSets, const std::vector<VkDescriptorPoolSize>& poolSizes) {
  VkDescriptorPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  info.maxSets = maxSets;
  info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  info.pPoolSizes = poolSizes.data();
  VkDescriptorPool pool;
  vkCreateDescriptorPool(m_device, &info, nullptr, &pool);
  return pool;
}

VkDescriptorSet VulkanDevice::allocateDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout) {
  VkDescriptorSetAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  info.descriptorPool = pool;
  info.descriptorSetCount = 1;
  info.pSetLayouts = &layout;
  VkDescriptorSet set;
  vkAllocateDescriptorSets(m_device, &info, &set);
  return set;
}

VkPipelineLayout VulkanDevice::createPipelineLayout(VkDescriptorSetLayout layout, uint32_t pushConstantSize) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  info.setLayoutCount = 1;
  info.pSetLayouts = &layout;

  VkPushConstantRange pushRange{};
  if (pushConstantSize > 0) {
    pushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushRange.offset = 0;
    pushRange.size = pushConstantSize;
    info.pushConstantRangeCount = 1;
    info.pPushConstantRanges = &pushRange;
  }

  VkPipelineLayout pipelineLayout;
  vkCreatePipelineLayout(m_device, &info, nullptr, &pipelineLayout);
  return pipelineLayout;
}

VkPipeline VulkanDevice::createGraphicsPipeline(VkPipelineLayout layout, VkRenderPass renderPass, const VkGraphicsPipelineCreateInfo& info) {
  VkGraphicsPipelineCreateInfo createInfo = info;
  createInfo.layout = layout;
  createInfo.renderPass = renderPass;

  VkPipeline pipeline;
  VkResult result = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create graphics pipeline: %d", result);
    return VK_NULL_HANDLE;
  }
  return pipeline;
}

VkPipeline VulkanDevice::createComputePipeline(VkPipelineLayout layout, const VkComputePipelineCreateInfo& info) {
  VkComputePipelineCreateInfo createInfo = info;
  createInfo.layout = layout;

  VkPipeline pipeline;
  VkResult result = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create compute pipeline: %d", result);
    return VK_NULL_HANDLE;
  }
  return pipeline;
}

VkRenderPass VulkanDevice::createRenderPass(const VkRenderPassCreateInfo& info) {
  VkRenderPass renderPass;
  vkCreateRenderPass(m_device, &info, nullptr, &renderPass);
  return renderPass;
}

VkFramebuffer VulkanDevice::createFramebuffer(VkRenderPass renderPass, const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height, uint32_t layers) {
  VkFramebufferCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  info.renderPass = renderPass;
  info.attachmentCount = static_cast<uint32_t>(attachments.size());
  info.pAttachments = attachments.data();
  info.width = width;
  info.height = height;
  info.layers = layers;

  VkFramebuffer fb;
  vkCreateFramebuffer(m_device, &info, nullptr, &fb);
  return fb;
}

VkShaderModule VulkanDevice::createShaderModule(const uint32_t* code, size_t size) {
  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = size;
  info.pCode = code;

  VkShaderModule module;
  VkResult result = vkCreateShaderModule(m_device, &info, nullptr, &module);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create shader module: %d", result);
    return VK_NULL_HANDLE;
  }
  return module;
}

bool VulkanDevice::createSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height, VulkanSwapchain& out) {
  // Query surface capabilities
  VkSurfaceCapabilitiesKHR caps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, surface, &caps);

  // Pick surface format
  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &formatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> formats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &formatCount, formats.data());

  VkSurfaceFormatKHR chosenFormat = formats[0];
  for (auto& f : formats) {
    if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      chosenFormat = f;
      break;
    }
  }

  // Pick present mode
  uint32_t presentModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, presentModes.data());

  VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (auto& m : presentModes) {
    if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
      chosenPresentMode = m;
      break;
    }
  }

  // Clamp extent
  VkExtent2D extent = caps.currentExtent;
  if (extent.width == 0xFFFFFFFF) {
    extent.width = std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, width));
    extent.height = std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, height));
  }

  uint32_t imageCount = caps.minImageCount + 1;
  if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
    imageCount = caps.maxImageCount;
  }

  // Create swapchain
  VkSwapchainCreateInfoKHR scInfo{};
  scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  scInfo.surface = surface;
  scInfo.minImageCount = imageCount;
  scInfo.imageFormat = chosenFormat.format;
  scInfo.imageColorSpace = chosenFormat.colorSpace;
  scInfo.imageExtent = extent;
  scInfo.imageArrayLayers = 1;
  scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {m_graphicsQueueFamily, m_presentQueueFamily};
  if (m_graphicsQueueFamily != m_presentQueueFamily) {
    scInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    scInfo.queueFamilyIndexCount = 2;
    scInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    scInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  scInfo.preTransform = caps.currentTransform;
  scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  scInfo.presentMode = chosenPresentMode;
  scInfo.clipped = VK_TRUE;
  scInfo.oldSwapchain = VK_NULL_HANDLE;

  VkResult result = vkCreateSwapchainKHR(m_device, &scInfo, nullptr, &out.swapchain);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create swapchain: %d", result);
    return false;
  }

  out.format = chosenFormat.format;
  out.extent = extent;

  // Get images
  uint32_t swapImageCount = 0;
  vkGetSwapchainImagesKHR(m_device, out.swapchain, &swapImageCount, nullptr);
  out.images.resize(swapImageCount);
  vkGetSwapchainImagesKHR(m_device, out.swapchain, &swapImageCount, out.images.data());

  // Create image views
  out.views.resize(swapImageCount);
  for (uint32_t i = 0; i < swapImageCount; i++) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = out.images[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = chosenFormat.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    result = vkCreateImageView(m_device, &viewInfo, nullptr, &out.views[i]);
    if (result != VK_SUCCESS) {
      VKWIND11_LOG_ERROR("Failed to create swapchain image view %u: %d", i, result);
      return false;
    }
  }

  return true;
}

uint32_t VulkanDevice::acquireNextImage(VkSemaphore signalSemaphore, VulkanSwapchain& swapchain) {
  uint32_t imageIndex = 0;
  VkResult result = vkAcquireNextImageKHR(
    m_device, swapchain.swapchain, UINT64_MAX, signalSemaphore, VK_NULL_HANDLE, &imageIndex);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("vkAcquireNextImageKHR failed: %d", result);
    return 0;
  }
  swapchain.currentImage = imageIndex;
  return imageIndex;
}

bool VulkanDevice::submitImmediate(VkCommandBuffer cmd) {
  vkResetFences(m_device, 1, &m_immFence);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;

  vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_immFence);
  vkWaitForFences(m_device, 1, &m_immFence, VK_TRUE, UINT64_MAX);

  return true;
}

// ============================================================================
// VulkanBuffer
// ============================================================================

void VulkanBuffer::destroy(VkDevice device) {
  if (buffer) vkDestroyBuffer(device, buffer, nullptr);
  if (memory) vkFreeMemory(device, memory, nullptr);
  buffer = VK_NULL_HANDLE;
  memory = VK_NULL_HANDLE;
  mapped = nullptr;
}

// ============================================================================
// VulkanImage
// ============================================================================

void VulkanImage::destroy(VkDevice device) {
  if (view) vkDestroyImageView(device, view, nullptr);
  if (image) vkDestroyImage(device, image, nullptr);
  if (memory) vkFreeMemory(device, memory, nullptr);
  image = VK_NULL_HANDLE;
  view = VK_NULL_HANDLE;
  memory = VK_NULL_HANDLE;
}

// ============================================================================
// VulkanSampler
// ============================================================================

void VulkanSampler::destroy(VkDevice device) {
  if (sampler) vkDestroySampler(device, sampler, nullptr);
  sampler = VK_NULL_HANDLE;
}

// ============================================================================
// VulkanPipeline
// ============================================================================

void VulkanPipeline::destroy(VkDevice device) {
  if (pipeline) vkDestroyPipeline(device, pipeline, nullptr);
  if (layout) vkDestroyPipelineLayout(device, layout, nullptr);
  pipeline = VK_NULL_HANDLE;
  layout = VK_NULL_HANDLE;
}

// ============================================================================
// VulkanRenderPass
// ============================================================================

void VulkanRenderPass::destroy(VkDevice device) {
  if (renderPass) vkDestroyRenderPass(device, renderPass, nullptr);
  renderPass = VK_NULL_HANDLE;
}

// ============================================================================
// VulkanFramebuffer
// ============================================================================

void VulkanFramebuffer::destroy(VkDevice device) {
  if (framebuffer) vkDestroyFramebuffer(device, framebuffer, nullptr);
  framebuffer = VK_NULL_HANDLE;
}

// ============================================================================
// VulkanSwapchain
// ============================================================================

void VulkanSwapchain::destroy(VkDevice device) {
  for (auto fb : framebuffers) vkDestroyFramebuffer(device, fb, nullptr);
  for (auto v : views) vkDestroyImageView(device, v, nullptr);
  if (swapchain) vkDestroySwapchainKHR(device, swapchain, nullptr);
  framebuffers.clear();
  views.clear();
  images.clear();
  swapchain = VK_NULL_HANDLE;
}

// ============================================================================
// VulkanQueryPool
// ============================================================================

void VulkanQueryPool::destroy(VkDevice device) {
  if (queryPool) vkDestroyQueryPool(device, queryPool, nullptr);
  queryPool = VK_NULL_HANDLE;
}
