#include "d3d11_device.h"
#include "../common/logging.h"

// ============================================================================
// D3D11BufferImpl
// ============================================================================

D3D11BufferImpl::D3D11BufferImpl(D3D11Device* dev, const D3D11_BUFFER_DESC* bufferDesc, const D3D11_SUBRESOURCE_DATA* initialData)
  : device(dev)
  , desc(*bufferDesc) {

  auto& vk = device->getVulkanDevice();

  VkBufferUsageFlags vkUsage = 0;
  if (bufferDesc->BindFlags & D3D11_BIND_VERTEX_BUFFER)    vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  if (bufferDesc->BindFlags & D3D11_BIND_INDEX_BUFFER)     vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  if (bufferDesc->BindFlags & D3D11_BIND_CONSTANT_BUFFER)  vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  if (bufferDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE)  vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (bufferDesc->BindFlags & D3D11_BIND_STREAM_OUTPUT)    vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  if (bufferDesc->BindFlags & D3D11_BIND_UNORDERED_ACCESS) vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

  VkMemoryPropertyFlags memProps = 0;
  switch (bufferDesc->Usage) {
    case D3D11_USAGE_DEFAULT:
      memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case D3D11_USAGE_IMMUTABLE:
      memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      break;
    case D3D11_USAGE_DYNAMIC:
      memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      break;
    case D3D11_USAGE_STAGING:
      memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      break;
  }

  if (bufferDesc->CPUAccessFlags & D3D11_CPU_ACCESS_READ) {
    memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
  }
  if (bufferDesc->CPUAccessFlags & D3D11_CPU_ACCESS_WRITE) {
    memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }

  // Ensure we can transfer to/from for staging
  if (bufferDesc->Usage == D3D11_USAGE_STAGING) {
    vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  if (!vk.createBuffer(bufferDesc->ByteWidth, vkUsage, memProps, vkBuffer)) {
    VKWIND11_LOG_ERROR("Failed to create Vulkan buffer for D3D11 buffer (%u bytes)", bufferDesc->ByteWidth);
    return;
  }

  // Upload initial data
  if (initialData && initialData->pSysMem) {
    if (vkBuffer.mapped) {
      memcpy(vkBuffer.mapped, initialData->pSysMem, bufferDesc->ByteWidth);
    } else {
      // Need staging buffer
      VulkanBuffer staging;
      vk.createBuffer(bufferDesc->ByteWidth, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);

      if (staging.mapped) {
        memcpy(staging.mapped, initialData->pSysMem, bufferDesc->ByteWidth);

        // Copy via command buffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = /* TODO: get from device */ VK_NULL_HANDLE;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        // For now, just copy directly if buffer is host visible
        if (vkBuffer.isHostVisible()) {
          vkMapMemory(vk.getDevice(), vkBuffer.memory, 0, bufferDesc->ByteWidth, 0, &vkBuffer.mapped);
          if (vkBuffer.mapped) {
            memcpy(vkBuffer.mapped, initialData->pSysMem, bufferDesc->ByteWidth);
            vkUnmapMemory(vk.getDevice(), vkBuffer.memory);
            vkBuffer.mapped = nullptr;
          }
        }
      }

      staging.destroy(vk.getDevice());
    }
  }

  VKWIND11_LOG_DEBUG("D3D11Buffer created: %u bytes, usage=0x%x", bufferDesc->ByteWidth, bufferDesc->BindFlags);
}

D3D11BufferImpl::~D3D11BufferImpl() {
  vkBuffer.destroy(device->getVulkanDevice().getDevice());
}

HRESULT D3D11BufferImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11Buffer || riid == IID_ID3D11Resource || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D11Buffer*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

void D3D11BufferImpl::GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) {
  if (pResourceDimension) *pResourceDimension = D3D11_RESOURCE_DIMENSION_BUFFER;
}

void D3D11BufferImpl::SetEvictionPriority(UINT EvictionPriority) {}
UINT D3D11BufferImpl::GetEvictionPriority() { return 0; }

void D3D11BufferImpl::GetDesc(D3D11_BUFFER_DESC* pDesc) {
  if (pDesc) *pDesc = desc;
}

void D3D11BufferImpl::GetDevice(ID3D11Device** ppDevice) {
  if (ppDevice) {
    *ppDevice = device;
    device->AddRef();
  }
}

HRESULT D3D11BufferImpl::GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11BufferImpl::SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11BufferImpl::SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) { return S_OK; }

// ============================================================================
// D3D11Texture2DImpl
// ============================================================================

D3D11Texture2DImpl::D3D11Texture2DImpl(D3D11Device* dev, const D3D11_TEXTURE2D_DESC* textureDesc, const D3D11_SUBRESOURCE_DATA* initialData)
  : device(dev)
  , desc(*textureDesc) {

  auto& vk = device->getVulkanDevice();

  // Convert D3D11 format to Vulkan format
  VkFormat vkFormat = VK_FORMAT_R8G8B8A8_UNORM; // default
  switch (textureDesc->Format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:      vkFormat = VK_FORMAT_R8G8B8A8_UNORM; break;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: vkFormat = VK_FORMAT_R8G8B8A8_SRGB; break;
    case DXGI_FORMAT_R8G8B8A8_UINT:       vkFormat = VK_FORMAT_R8G8B8A8_UINT; break;
    case DXGI_FORMAT_R8G8B8A8_SNORM:      vkFormat = VK_FORMAT_R8G8B8A8_SNORM; break;
    case DXGI_FORMAT_R8G8B8A8_SINT:       vkFormat = VK_FORMAT_R8G8B8A8_SINT; break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:      vkFormat = VK_FORMAT_B8G8R8A8_UNORM; break;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: vkFormat = VK_FORMAT_B8G8R8A8_SRGB; break;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:  vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT; break;
    case DXGI_FORMAT_R32_FLOAT:           vkFormat = VK_FORMAT_R32_SFLOAT; break;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:  vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT; break;
    case DXGI_FORMAT_D32_FLOAT:           vkFormat = VK_FORMAT_D32_SFLOAT; break;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:   vkFormat = VK_FORMAT_D24_UNORM_S8_UINT; break;
    case DXGI_FORMAT_R8_UNORM:            vkFormat = VK_FORMAT_R8_UNORM; break;
    case DXGI_FORMAT_BC1_UNORM:           vkFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK; break;
    case DXGI_FORMAT_BC1_UNORM_SRGB:      vkFormat = VK_FORMAT_BC1_RGBA_SRGB_BLOCK; break;
    case DXGI_FORMAT_BC3_UNORM:           vkFormat = VK_FORMAT_BC3_UNORM_BLOCK; break;
    case DXGI_FORMAT_BC3_UNORM_SRGB:      vkFormat = VK_FORMAT_BC3_SRGB_BLOCK; break;
    case DXGI_FORMAT_BC5_UNORM:           vkFormat = VK_FORMAT_BC5_UNORM_BLOCK; break;
    case DXGI_FORMAT_BC7_UNORM:           vkFormat = VK_FORMAT_BC7_UNORM_BLOCK; break;
    default:
      VKWIND11_LOG_WARN("Unhandled DXGI format %d, defaulting to R8G8B8A8_UNORM", textureDesc->Format);
      vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
      break;
  }

  VkImageUsageFlags vkUsage = VK_IMAGE_USAGE_SAMPLED_BIT;
  if (textureDesc->BindFlags & D3D11_BIND_RENDER_TARGET) vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (textureDesc->BindFlags & D3D11_BIND_DEPTH_STENCIL) vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  if (textureDesc->BindFlags & D3D11_BIND_UNORDERED_ACCESS) vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
  vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
  if (textureDesc->SampleDescCount > 1) {
    switch (textureDesc->SampleDescCount) {
      case 2: samples = VK_SAMPLE_COUNT_2_BIT; break;
      case 4: samples = VK_SAMPLE_COUNT_4_BIT; break;
      case 8: samples = VK_SAMPLE_COUNT_8_BIT; break;
      default: samples = VK_SAMPLE_COUNT_1_BIT; break;
    }
  }

  VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  if (textureDesc->Usage == D3D11_USAGE_STAGING) {
    memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }

  if (!vk.createImage(textureDesc->Width, textureDesc->Height, textureDesc->MipLevels, textureDesc->ArraySize,
                      vkFormat, samples, vkUsage,
                      memProps, vkImage)) {
    VKWIND11_LOG_ERROR("Failed to create Vulkan image for D3D11 texture");
    return;
  }

  // Upload initial data if provided
  if (initialData && initialData->pSysMem && vkImage.memory) {
    // TODO: staging buffer upload for textures
    // For now, only works if initial data is already in GPU memory
  }

  VKWIND11_LOG_DEBUG("D3D11Texture2D created: %ux%u, format=%d, mip=%u, array=%u",
    textureDesc->Width, textureDesc->Height, textureDesc->Format, textureDesc->MipLevels, textureDesc->ArraySize);
}

D3D11Texture2DImpl::~D3D11Texture2DImpl() {
  vkImage.destroy(device->getVulkanDevice().getDevice());
}

HRESULT D3D11Texture2DImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11Texture2D || riid == IID_ID3D11Resource || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D11Texture2D*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

void D3D11Texture2DImpl::GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) {
  if (pResourceDimension) *pResourceDimension = D3D11_RESOURCE_DIMENSION_TEXTURE2D;
}

void D3D11Texture2DImpl::SetEvictionPriority(UINT EvictionPriority) {}
UINT D3D11Texture2DImpl::GetEvictionPriority() { return 0; }

void D3D11Texture2DImpl::GetDesc(D3D11_TEXTURE2D_DESC* pDesc) {
  if (pDesc) *pDesc = desc;
}

void D3D11Texture2DImpl::GetDevice(ID3D11Device** ppDevice) {
  if (ppDevice) {
    *ppDevice = device;
    device->AddRef();
  }
}

HRESULT D3D11Texture2DImpl::GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11Texture2DImpl::SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11Texture2DImpl::SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) { return S_OK; }
