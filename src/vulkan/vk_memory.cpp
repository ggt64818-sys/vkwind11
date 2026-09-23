#include "vk_memory.h"
#include "../common/logging.h"

// ============================================================================
// VulkanMemoryAllocator Implementation
// ============================================================================

VulkanMemoryAllocator::~VulkanMemoryAllocator() {
  shutdown();
}

void VulkanMemoryAllocator::initialize(VkPhysicalDevice physicalDevice, VkDevice device) {
  m_physicalDevice = physicalDevice;
  m_device = device;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_memoryProperties);
  VKWIND11_LOG_INFO("Vulkan memory: %u memory types, %u heaps",
    m_memoryProperties.memoryTypeCount, m_memoryProperties.memoryHeapCount);
}

void VulkanMemoryAllocator::shutdown() {
  m_physicalDevice = VK_NULL_HANDLE;
  m_device = VK_NULL_HANDLE;
}

uint32_t VulkanMemoryAllocator::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
  for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  return UINT32_MAX;
}

bool VulkanMemoryAllocator::isMappable(uint32_t memoryTypeIndex) const {
  if (memoryTypeIndex >= m_memoryProperties.memoryTypeCount) return false;
  return (m_memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
}

VulkanMemoryBlock VulkanMemoryAllocator::allocateDeviceLocal(VkDeviceSize size, VkDeviceSize alignment) {
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = size;
  allocInfo.memoryTypeIndex = findMemoryType(
    0xFFFFFFFF,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
  );

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    VKWIND11_LOG_ERROR("No suitable device-local memory type found");
    return {};
  }

  VulkanMemoryBlock block;
  VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &block.memory);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to allocate device memory: %d", result);
    return {};
  }

  block.size = size;
  block.offset = 0;
  block.memoryTypeIndex = allocInfo.memoryTypeIndex;

  VKWIND11_LOG_INFO("Allocated device memory: %llu bytes (type %u)", size, allocInfo.memoryTypeIndex);
  return block;
}

VulkanMemoryBlock VulkanMemoryAllocator::allocateHostVisible(VkDeviceSize size, VkDeviceSize alignment) {
  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = size;
  allocInfo.memoryTypeIndex = findMemoryType(
    0xFFFFFFFF,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
  );

  if (allocInfo.memoryTypeIndex == UINT32_MAX) {
    VKWIND11_LOG_ERROR("No suitable host-visible memory type found");
    return {};
  }

  VulkanMemoryBlock block;
  VkResult result = vkAllocateMemory(m_device, &allocInfo, nullptr, &block.memory);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to allocate host memory: %d", result);
    return {};
  }

  block.size = size;
  block.offset = 0;
  block.memoryTypeIndex = allocInfo.memoryTypeIndex;

  VKWIND11_LOG_INFO("Allocated host memory: %llu bytes (type %u)", size, allocInfo.memoryTypeIndex);
  return block;
}

void* VulkanMemoryAllocator::mapMemory(VulkanMemoryBlock& block) {
  if (block.mappedData) return block.mappedData;
  if (!isMappable(block.memoryTypeIndex)) {
    VKWIND11_LOG_ERROR("Cannot map non-mappable memory");
    return nullptr;
  }

  void* data = nullptr;
  VkResult result = vkMapMemory(m_device, block.memory, block.offset, block.size, 0, &data);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to map memory: %d", result);
    return nullptr;
  }

  block.mappedData = data;
  return data;
}

void VulkanMemoryAllocator::unmapMemory(VulkanMemoryBlock& block) {
  if (block.mappedData) {
    vkUnmapMemory(m_device, block.memory);
    block.mappedData = nullptr;
  }
}

void VulkanMemoryAllocator::free(VulkanMemoryBlock& block) {
  if (block.memory) {
    vkFreeMemory(m_device, block.memory, nullptr);
    block = {};
  }
}
