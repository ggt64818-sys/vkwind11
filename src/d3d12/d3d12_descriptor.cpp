#include "d3d12_device.h"
#include "../common/logging.h"

// ============================================================================
// D3D12 Descriptor Heap
// ============================================================================

D3D12DescriptorHeapImpl::D3D12DescriptorHeapImpl(D3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC* desc)
  : m_device(device), m_desc(*desc) {
  auto& vk = device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  m_maxSets = desc->NumDescriptors > 0 ? desc->NumDescriptors : 256;
  m_handleIncrement = device->GetDescriptorHandleIncrementSize(desc->Type);

  // Create descriptor pool
  VkDescriptorPoolSize poolSize = {};
  switch (desc->Type) {
    case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
      poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
      poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
      break;
    case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
    case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
      poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      break;
    default:
      poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      break;
  }
  poolSize.descriptorCount = m_maxSets;

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = m_maxSets;
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes = &poolSize;

  VkResult result = vkCreateDescriptorPool(vkDev, &poolInfo, nullptr, &m_pool);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12DescriptorHeapImpl: vkCreateDescriptorPool failed %d", result);
  }

  VKWIND11_LOG_INFO("D3D12DescriptorHeapImpl created type=%d maxSets=%d", (int)desc->Type, m_maxSets);
}

D3D12DescriptorHeapImpl::~D3D12DescriptorHeapImpl() {
  if (m_pool != VK_NULL_HANDLE) {
    auto& vk = m_device->getVulkanDevice();
    vkDestroyDescriptorPool(vk.getDevice(), m_pool, nullptr);
    m_pool = VK_NULL_HANDLE;
  }
  VKWIND11_LOG_INFO("D3D12DescriptorHeapImpl destroyed");
}

HRESULT STDMETHODCALLTYPE D3D12DescriptorHeapImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12DescriptorHeap || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12DescriptorHeap*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12DescriptorHeapImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE D3D12DescriptorHeapImpl::GetCPUDescriptorHandleForHeapStart() {
  return {reinterpret_cast<uint64_t>(m_pool)};
}

D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE D3D12DescriptorHeapImpl::GetGPUDescriptorHandleForHeapStart() {
  return {reinterpret_cast<uint64_t>(m_pool)};
}

D3D12_DESCRIPTOR_HEAP_DESC STDMETHODCALLTYPE D3D12DescriptorHeapImpl::GetDesc() {
  return m_desc;
}

VkDescriptorSet D3D12DescriptorHeapImpl::allocateDescriptor(VkDescriptorSetLayout layout) {
  auto& vk = m_device->getVulkanDevice();
  return vk.allocateDescriptorSet(m_pool, layout);
}

void D3D12DescriptorHeapImpl::updateBufferDescriptor(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type) {
  auto& vk = m_device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = buffer;
  bufferInfo.offset = offset;
  bufferInfo.range = range;

  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.descriptorCount = 1;
  write.descriptorType = type;
  write.pBufferInfo = &bufferInfo;

  vkUpdateDescriptorSets(vkDev, 1, &write, 0, nullptr);
}

void D3D12DescriptorHeapImpl::updateImageDescriptor(VkDescriptorSet set, uint32_t binding, VkImageView view, VkSampler sampler, VkImageLayout layout) {
  auto& vk = m_device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  VkDescriptorImageInfo imageInfo = {};
  imageInfo.imageView = view;
  imageInfo.sampler = sampler;
  imageInfo.imageLayout = layout;

  VkWriteDescriptorSet write = {};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = set;
  write.dstBinding = binding;
  write.descriptorCount = 1;
  write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  write.pImageInfo = &imageInfo;

  vkUpdateDescriptorSets(vkDev, 1, &write, 0, nullptr);
}

VkDevice D3D12DescriptorHeapImpl::getVkDevice() const {
  return m_device->getVulkanDevice().getDevice();
}
