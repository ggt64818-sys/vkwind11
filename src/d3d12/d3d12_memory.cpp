#include "d3d12_device.h"
#include "../common/logging.h"
#include "../common/dxgi_utils.h"

// ============================================================================
// D3D12 Resource Implementation
// ============================================================================

D3D12ResourceImpl::D3D12ResourceImpl(D3D12Device* device, const D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState)
  : m_device(device), m_desc(*desc), m_currentState(initialState) {
}

D3D12ResourceImpl::~D3D12ResourceImpl() {
  auto& vk = m_device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  if (isBuffer()) {
    vkBuffer.destroy(vkDev);
  } else {
    vkImage.destroy(vkDev);
  }
}

bool D3D12ResourceImpl::allocateVulkanBacking() {
  auto& vk = m_device->getVulkanDevice();

  if (isBuffer()) {
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                               VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!vk.createBuffer(m_desc.Width, usage, memProps, vkBuffer)) {
      VKWIND11_LOG_ERROR("D3D12ResourceImpl: failed to create buffer size=%llu", m_desc.Width);
      return false;
    }
  } else {
    uint32_t width = (uint32_t)m_desc.Width;
    uint32_t height = m_desc.Height;
    uint16_t mipLevels = m_desc.MipLevels > 0 ? m_desc.MipLevels : 1;
    uint16_t arrayLayers = m_desc.DepthOrArraySize > 0 ? m_desc.DepthOrArraySize : 1;
    VkFormat format = convertDxgiToVkFormat(m_desc.Format);

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {
      usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
      usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (m_desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
      usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (!vk.createImage(width, height, mipLevels, arrayLayers, format, VK_SAMPLE_COUNT_1_BIT, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vkImage)) {
      VKWIND11_LOG_ERROR("D3D12ResourceImpl: failed to create image %ux%u", width, height);
      return false;
    }
  }

  VKWIND11_LOG_DEBUG("D3D12ResourceImpl allocated Vulkan backing dim=%d", (int)m_desc.Dimension);
  return true;
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12Resource || riid == IID_ID3D12Pageable || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12Resource*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::GetHeapProperties(void* pHeapProperties, void* pResidencyPriority) {
  if (pHeapProperties) memset(pHeapProperties, 0, sizeof(D3D12_HEAP_PROPERTIES));
  if (pResidencyPriority) *(UINT*)pResidencyPriority = 0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::SetName(const wchar_t* Name) {
  if (!Name) return S_OK;

  auto& vk = m_device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  char nameBuf[256] = {};
  for (int i = 0; i < 255 && Name[i]; i++) {
    nameBuf[i] = (char)Name[i];
  }

  if (isBuffer() && vkBuffer.buffer != VK_NULL_HANDLE) {
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
    nameInfo.objectHandle = (uint64_t)vkBuffer.buffer;
    nameInfo.pObjectName = nameBuf;
    auto pfnSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(vkDev, "vkSetDebugUtilsObjectNameEXT");
    if (pfnSetDebugUtilsObjectName) pfnSetDebugUtilsObjectName(vkDev, &nameInfo);
  } else if (!isBuffer() && vkImage.image != VK_NULL_HANDLE) {
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    nameInfo.objectHandle = (uint64_t)vkImage.image;
    nameInfo.pObjectName = nameBuf;
    auto pfnSetDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(vkDev, "vkSetDebugUtilsObjectNameEXT");
    if (pfnSetDebugUtilsObjectName) pfnSetDebugUtilsObjectName(vkDev, &nameInfo);
  }

  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::Map(UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData) {
  if (!ppData) return E_POINTER;
  if (!isBuffer()) {
    VKWIND11_LOG_WARN("D3D12ResourceImpl::Map on non-buffer resource");
    *ppData = nullptr;
    return E_FAIL;
  }
  if (!vkBuffer.isHostVisible()) {
    VKWIND11_LOG_ERROR("D3D12ResourceImpl::Map buffer is not host-visible");
    return E_FAIL;
  }
  *ppData = vkBuffer.mapped;
  return S_OK;
}

void STDMETHODCALLTYPE D3D12ResourceImpl::Unmap(UINT Subresource, const D3D12_RANGE* pWrittenRange) {
  // No-op for host-coherent memory
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::GetDesc(D3D12_RESOURCE_DESC* pDesc) {
  if (!pDesc) return E_POINTER;
  pDesc->Dimension = m_desc.Dimension;
  pDesc->Alignment = m_desc.Alignment;
  pDesc->Width = m_desc.Width;
  pDesc->Height = m_desc.Height;
  pDesc->DepthOrArraySize = m_desc.DepthOrArraySize;
  pDesc->MipLevels = m_desc.MipLevels;
  pDesc->Format = m_desc.Format;
  pDesc->SampleDesc_Count = m_desc.SampleDesc_Count;
  pDesc->SampleDesc_Quality = m_desc.SampleDesc_Quality;
  pDesc->Layout = m_desc.Layout;
  pDesc->Flags = m_desc.Flags;
  return S_OK;
}

D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE D3D12ResourceImpl::GetGPUVirtualAddress() {
  if (isBuffer() && vkBuffer.buffer != VK_NULL_HANDLE) {
    return (D3D12_GPU_VIRTUAL_ADDRESS)vkBuffer.buffer;
  }
  if (!isBuffer() && vkImage.image != VK_NULL_HANDLE) {
    return (D3D12_GPU_VIRTUAL_ADDRESS)vkImage.image;
  }
  return 0;
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::WriteToSubresource(UINT DstSubresource, const D3D12_RANGE* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) {
  if (!isBuffer() || !vkBuffer.mapped) return E_FAIL;

  uint32_t bpp = 4;
  uint32_t dstX = pDstBox ? (uint32_t)pDstBox->Begin : 0;
  uint32_t width = pDstBox ? (uint32_t)(pDstBox->End - pDstBox->Begin) : (uint32_t)m_desc.Width;
  uint32_t height = m_desc.Height > 0 ? m_desc.Height : 1;

  uint8_t* dst = (uint8_t*)vkBuffer.mapped;
  const uint8_t* src = (const uint8_t*)pSrcData;

  for (uint32_t y = 0; y < height; y++) {
    memcpy(dst + ((uint64_t)y * (uint64_t)SrcRowPitch), src + ((uint64_t)y * (uint64_t)SrcRowPitch), (uint64_t)width * bpp);
  }

  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12ResourceImpl::ReadFromSubresource(void* pDstData, UINT SrcSubresource, const D3D12_RANGE* pSrcBox, UINT SrcRowPitch, UINT SrcDepthPitch) {
  if (!isBuffer() || !vkBuffer.mapped) return E_FAIL;
  memcpy(pDstData, vkBuffer.mapped, vkBuffer.size);
  return S_OK;
}
