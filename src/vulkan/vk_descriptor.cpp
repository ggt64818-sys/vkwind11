#include "vk_descriptor.h"
#include "../common/logging.h"

// ============================================================================
// VulkanDescriptorManager Implementation
// ============================================================================

VulkanDescriptorManager::~VulkanDescriptorManager() {
  shutdown();
}

void VulkanDescriptorManager::initialize(VkDevice device, uint32_t maxSets) {
  m_device = device;

  VkDescriptorPoolSize poolSizes[] = {
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSets * 4 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSets * 8 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxSets * 2 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, maxSets * 2 },
  };

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = maxSets;
  poolInfo.poolSizeCount = 4;
  poolInfo.pPoolSizes = poolSizes;

  VkResult result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_pool);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create descriptor pool: %d", result);
  } else {
    VKWIND11_LOG_INFO("Descriptor pool created: maxSets=%u", maxSets);
  }
}

void VulkanDescriptorManager::shutdown() {
  if (m_device && m_pool) {
    vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    m_pool = VK_NULL_HANDLE;
  }
  m_device = VK_NULL_HANDLE;
}

VkDescriptorSetLayout VulkanDescriptorManager::createLayout(const std::vector<VulkanDescriptorBinding>& bindings) {
  std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
  layoutBindings.reserve(bindings.size());

  for (auto& b : bindings) {
    VkDescriptorSetLayoutBinding lb = {};
    lb.binding = b.binding;
    lb.descriptorType = b.type;
    lb.descriptorCount = b.count > 0 ? b.count : 1;
    lb.stageFlags = b.stageFlags;
    layoutBindings.push_back(lb);
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = (uint32_t)layoutBindings.size();
  layoutInfo.pBindings = layoutBindings.data();

  VkDescriptorSetLayout layout;
  VkResult result = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &layout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create descriptor set layout: %d", result);
    return VK_NULL_HANDLE;
  }

  return layout;
}

VkDescriptorSet VulkanDescriptorManager::allocateSet(VkDescriptorSetLayout layout) {
  std::lock_guard lock(m_mutex);

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_pool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &layout;

  VkDescriptorSet set;
  VkResult result = vkAllocateDescriptorSets(m_device, &allocInfo, &set);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to allocate descriptor set: %d", result);
    return VK_NULL_HANDLE;
  }

  return set;
}

void VulkanDescriptorManager::updateBuffer(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type) {
  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = buffer;
  bufferInfo.offset = offset;
  bufferInfo.range = range;

  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.dstArrayElement = 0;
  write.descriptorType = type;
  write.descriptorCount = 1;
  write.pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}

void VulkanDescriptorManager::updateImage(VkDescriptorSet set, uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout) {
  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageLayout = layout;
  imageInfo.imageView = imageView;
  imageInfo.sampler = sampler;

  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.dstArrayElement = 0;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.descriptorCount = 1;
  write.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}

void VulkanDescriptorManager::resetPool() {
  std::lock_guard lock(m_mutex);
  vkResetDescriptorPool(m_device, m_pool, 0);
}
