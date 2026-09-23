#include "d3d12_device.h"
#include "../common/logging.h"
#include "../common/config.h"

// ============================================================================
// D3D12Device — COM IUnknown + ID3D12Device
// ============================================================================

D3D12Device::D3D12Device() {
  m_vkDevice = std::make_unique<VulkanDevice>();
  m_descriptorHandleSize[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = 64;
  m_descriptorHandleSize[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = 32;
  m_descriptorHandleSize[D3D12_DESCRIPTOR_HEAP_TYPE_RTV] = 64;
  m_descriptorHandleSize[D3D12_DESCRIPTOR_HEAP_TYPE_DSV] = 64;

  m_options.ResourceBindingTier = D3D12_RESOURCE_BINDING_TIER_3;
  VKWIND11_LOG_INFO("D3D12Device created");
}

D3D12Device::~D3D12Device() {
  if (m_vkDevice) {
    m_vkDevice->destroy();
  }
  VKWIND11_LOG_INFO("D3D12Device destroyed");
}

HRESULT STDMETHODCALLTYPE D3D12Device::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12Device || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12Device*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12Device::GetPrivateData(REFGUID, UINT*, void*) { return S_OK; }
HRESULT STDMETHODCALLTYPE D3D12Device::SetPrivateData(REFGUID, UINT, const void*) { return S_OK; }
HRESULT STDMETHODCALLTYPE D3D12Device::SetPrivateDataInterface(REFGUID, const IUnknown*) { return S_OK; }

HRESULT STDMETHODCALLTYPE D3D12Device::SetName(const wchar_t* Name) {
  if (Name) {
    VKWIND11_LOG_INFO("D3D12Device::SetName set");
  }
  return S_OK;
}

// ============================================================================
// ID3D12Device
// ============================================================================

HRESULT STDMETHODCALLTYPE D3D12Device::GetNodeCount(UINT* pNodeCount) {
  if (!pNodeCount) return E_POINTER;
  *pNodeCount = 1;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue) {
  if (!ppCommandQueue) return E_POINTER;
  if (!m_vkDevice) return E_FAIL;

  D3D12_COMMAND_QUEUE_DESC desc = pDesc ? *pDesc : D3D12_COMMAND_QUEUE_DESC{};
  auto queue = new D3D12CommandQueueImpl(this, &desc);
  *ppCommandQueue = queue;
  VKWIND11_LOG_INFO("D3D12Device::CreateCommandQueue type=%d", (int)desc.Type);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** ppCommandAllocator) {
  if (!ppCommandAllocator) return E_POINTER;

  auto allocator = new D3D12CommandAllocatorImpl(this, type);
  *ppCommandAllocator = allocator;
  VKWIND11_LOG_INFO("D3D12Device::CreateCommandAllocator type=%d", (int)type);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList) {
  if (!ppCommandList || !pCommandAllocator) return E_POINTER;

  auto* allocator = static_cast<D3D12CommandAllocatorImpl*>(pCommandAllocator);
  D3D12PipelineStateImpl* initialPSO = pInitialState ? static_cast<D3D12PipelineStateImpl*>(pInitialState) : nullptr;

  auto cmdList = new D3D12GraphicsCommandListImpl(this, type, allocator, initialPSO);
  *ppCommandList = cmdList;
  VKWIND11_LOG_INFO("D3D12Device::CreateCommandList type=%d", (int)type);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) {
  if (!pFeatureSupportData) return E_POINTER;

  switch (Feature) {
    case D3D12_FEATURE_D3D12_OPTIONS: {
      auto* opts = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS*>(pFeatureSupportData);
      opts->DoublePrecisionFloatShaderOps = FALSE;
      opts->OutputMergerLogicOp = FALSE;
      opts->PSSpecifiedStencilRefSupported = FALSE;
      opts->TypedUAVLoadAdditionalFormats = FALSE;
      opts->ROVsSupported = FALSE;
      opts->ConservativeRasterizationTier = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
      opts->ResourceBindingTier = D3D12_RESOURCE_BINDING_TIER_3;
      opts->PSampleCustomEvalSupported = FALSE;
      return S_OK;
    }
    case D3D12_FEATURE_FEATURE_LEVELS: {
      auto* fl = static_cast<D3D12_FEATURE_DATA_FEATURE_LEVELS*>(pFeatureSupportData);
      if (fl->pFeatureLevelsRequested && fl->NumFeatureLevels > 0) {
        fl->MaxSupportedFeatureLevel = fl->pFeatureLevelsRequested[0];
      }
      return S_OK;
    }
    case D3D12_FEATURE_SHADER_MODEL: {
      auto* sm = static_cast<D3D12_FEATURE_DATA_SHADER_MODEL*>(pFeatureSupportData);
      sm->HighestShaderModel = D3D12_SHADER_MODEL_5_1;
      return S_OK;
    }
    case D3D12_FEATURE_ARCHITECTURE1: {
      auto* arch = static_cast<D3D12_FEATURE_DATA_ARCHITECTURE1*>(pFeatureSupportData);
      arch->TileBasedRenderer = TRUE;
      arch->UMA = TRUE;
      arch->CacheCoherentUMA = TRUE;
      arch->IsolatedMRTBlendConsistent = TRUE;
      return S_OK;
    }
    case D3D12_FEATURE_D3D12_OPTIONS1: {
      auto* opts = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS1*>(pFeatureSupportData);
      opts->WaveOps = FALSE;
      opts->WaveLaneCountMin = 0;
      opts->WaveLaneCountMax = 0;
      opts->TotalWaveSize = 0;
      opts->Int64ShaderOps = FALSE;
      return S_OK;
    }
    case D3D12_FEATURE_D3D12_OPTIONS2: {
      auto* opts = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS2*>(pFeatureSupportData);
      opts->DepthBoundsTestSupported = FALSE;
      opts->GpuUploadHeapSupported = FALSE;
      return S_OK;
    }
    case D3D12_FEATURE_D3D12_OPTIONS3: {
      auto* opts = static_cast<D3D12_FEATURE_DATA_D3D12_OPTIONS3*>(pFeatureSupportData);
      opts->TimestampQuery = FALSE;
      opts->TimestampQueryPipelines = FALSE;
      opts->CopyQueueTimestampQueries = FALSE;
      opts->PipelineStatisticsQuery = FALSE;
      opts->OcclusionQuery = FALSE;
      opts->OcclusionQueryPrecise = FALSE;
      return S_OK;
    }
    case D3D12_FEATURE_FORMAT_SUPPORT: {
      auto* fs = static_cast<D3D12_FEATURE_DATA_FORMAT_SUPPORT*>(pFeatureSupportData);
      fs->Support1 = D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_RENDER_TARGET;
      fs->Support2 = D3D12_FORMAT_SUPPORT2_SHADER_SAMPLE;
      return S_OK;
    }
    default:
      VKWIND11_LOG_WARN("D3D12Device::CheckFeatureSupport feature=%d not implemented", (int)Feature);
      memset(pFeatureSupportData, 0, FeatureSupportDataSize);
      return S_OK;
  }
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap) {
  if (!ppvHeap || !pDescriptorHeapDesc) return E_POINTER;

  auto heap = new D3D12DescriptorHeapImpl(this, pDescriptorHeapDesc);
  *ppvHeap = heap;
  VKWIND11_LOG_INFO("D3D12Device::CreateDescriptorHeap type=%d num=%d", (int)pDescriptorHeapDesc->Type, pDescriptorHeapDesc->NumDescriptors);
  return S_OK;
}

UINT STDMETHODCALLTYPE D3D12Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType) {
  if (DescriptorHeapType < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES) {
    return m_descriptorHandleSize[DescriptorHeapType];
  }
  return 64;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void** ppvRootSignature) {
  if (!ppvRootSignature) return E_POINTER;
  if (!pBlobWithRootSignature || blobLengthInBytes < sizeof(D3D12_ROOT_SIGNATURE_DESC)) return E_INVALIDARG;

  const auto* desc = static_cast<const D3D12_ROOT_SIGNATURE_DESC*>(pBlobWithRootSignature);
  auto rootSig = new D3D12RootSignatureImpl(this, desc);
  *ppvRootSignature = rootSig;
  VKWIND11_LOG_INFO("D3D12Device::CreateRootSignature params=%d staticSamplers=%d", desc->NumParameters, desc->NumStaticSamplers);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateGraphicsPipelineState(const void* pDesc, REFIID riid, void** ppPipelineState) {
  if (!ppPipelineState || !pDesc) return E_POINTER;
  *ppPipelineState = nullptr;

  const auto* desc = static_cast<const D3D12_GRAPHICS_PIPELINE_STATE_DESC*>(pDesc);
  auto* pso = new D3D12PipelineStateImpl();
  if (!pso->createGraphics(this, desc)) {
    delete pso;
    return E_FAIL;
  }
  *ppPipelineState = pso;
  VKWIND11_LOG_INFO("D3D12Device::CreateGraphicsPipelineState OK");
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateComputePipelineState(const void* pDesc, REFIID riid, void** ppPipelineState) {
  if (!ppPipelineState || !pDesc) return E_POINTER;
  *ppPipelineState = nullptr;

  const auto* desc = static_cast<const D3D12_COMPUTE_PIPELINE_STATE_DESC*>(pDesc);
  auto* pso = new D3D12PipelineStateImpl();
  if (!pso->createCompute(this, desc)) {
    delete pso;
    return E_FAIL;
  }
  *ppPipelineState = pso;
  VKWIND11_LOG_INFO("D3D12Device::CreateComputePipelineState OK");
  return S_OK;
}

void STDMETHODCALLTYPE D3D12Device::CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
  VKWIND11_LOG_TRACE("D3D12Device::CreateConstantBufferView");
}

void STDMETHODCALLTYPE D3D12Device::CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
  VKWIND11_LOG_TRACE("D3D12Device::CreateShaderResourceView");
}

void STDMETHODCALLTYPE D3D12Device::CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
  VKWIND11_LOG_TRACE("D3D12Device::CreateUnorderedAccessView");
}

void STDMETHODCALLTYPE D3D12Device::CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
  VKWIND11_LOG_TRACE("D3D12Device::CreateRenderTargetView");
}

void STDMETHODCALLTYPE D3D12Device::CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
  VKWIND11_LOG_TRACE("D3D12Device::CreateDepthStencilView");
}

void STDMETHODCALLTYPE D3D12Device::CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) {
  VKWIND11_LOG_TRACE("D3D12Device::CreateSampler");
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource) {
  if (!ppvResource || !pHeapProperties || !pDesc) return E_POINTER;

  auto resource = new D3D12ResourceImpl(this, pDesc, InitialResourceState);
  if (!resource->allocateVulkanBacking()) {
    delete resource;
    return E_OUTOFMEMORY;
  }

  *ppvResource = resource;
  VKWIND11_LOG_INFO("D3D12Device::CreateCommittedResource dim=%d w=%llu h=%u", (int)pDesc->Dimension, pDesc->Width, pDesc->Height);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateHeap(const D3D12_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap) {
  if (!ppvHeap || !pDesc) return E_POINTER;
  VKWIND11_LOG_WARN("D3D12Device::CreateHeap not fully implemented");
  auto heap = new D3D12HeapImpl(this, pDesc);
  *ppvHeap = heap;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreatePlacedResource(ID3D12Heap* pHeap, uint64_t HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) {
  if (!ppvResource || !pDesc) return E_POINTER;
  VKWIND11_LOG_WARN("D3D12Device::CreatePlacedResource — falling back to committed resource");
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
  return CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, pDesc, InitialResourceState, pOptimizedClearValue, riid, ppvResource);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) {
  if (!ppvResource) return E_POINTER;
  *ppvResource = nullptr;
  VKWIND11_LOG_WARN("D3D12Device::CreateReservedResource — falling back to committed");
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
  return CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, pDesc, InitialResourceState, pOptimizedClearValue, riid, ppvResource);
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateSharedHandle(ID3D12DeviceChild*, const void*, DWORD, const wchar_t*, HANDLE* pHandle) {
  VKWIND11_LOG_WARN("D3D12Device::CreateSharedHandle not implemented");
  if (pHandle) *pHandle = nullptr;
  return E_FAIL;
}

HRESULT STDMETHODCALLTYPE D3D12Device::OpenSharedHandle(HANDLE, REFIID, void**) {
  VKWIND11_LOG_WARN("D3D12Device::OpenSharedHandle not implemented");
  return E_FAIL;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateFence(uint64_t InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void** ppFence) {
  if (!ppFence) return E_POINTER;

  auto fence = new D3D12FenceImpl(this, InitialValue, Flags);
  *ppFence = fence;
  VKWIND11_LOG_INFO("D3D12Device::CreateFence initialValue=%llu", InitialValue);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::GetDeviceRemovedReason() {
  return S_OK;
}

void STDMETHODCALLTYPE D3D12Device::GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, uint64_t BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, uint64_t* pRowSizeInBytes, uint64_t* pTotalBytes) {
  if (!pResourceDesc) return;

  uint64_t totalBytes = 0;
  uint32_t bpp = 4;
  if (pResourceDesc->Format != DXGI_FORMAT_UNKNOWN) {
    bpp = 4;
  }

  for (UINT i = 0; i < NumSubresources; i++) {
    if (pLayouts) {
      pLayouts[i].Offset = totalBytes + BaseOffset;
      pLayouts[i].Footprint_Width = (uint32_t)pResourceDesc->Width;
      pLayouts[i].Footprint_Height = pResourceDesc->Height;
      pLayouts[i].Footprint_Depth = (pResourceDesc->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D) ? pResourceDesc->DepthOrArraySize : 1;
      pLayouts[i].Footprint_RowPitch = ((uint64_t)pLayouts[i].Footprint_Width * bpp + 255) & ~256ULL;
      totalBytes += (uint64_t)pLayouts[i].Footprint_RowPitch * pLayouts[i].Footprint_Height * pLayouts[i].Footprint_Depth;
    }
    if (pNumRows) pNumRows[i] = pResourceDesc->Height;
    if (pRowSizeInBytes) pRowSizeInBytes[i] = (uint64_t)pResourceDesc->Width * bpp;
  }
  if (pTotalBytes) *pTotalBytes = totalBytes;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateQueryHeap(const void* pDesc, REFIID riid, void** ppvHeap) {
  if (!ppvHeap) return E_POINTER;
  *ppvHeap = nullptr;
  VKWIND11_LOG_WARN("D3D12Device::CreateQueryHeap — stub returning dummy");
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12Device::CreateCommandSignature(const void* pDesc, ID3D12RootSignature* pRootSignature, REFIID riid, void** ppvCommandSignature) {
  if (!ppvCommandSignature) return E_POINTER;
  *ppvCommandSignature = nullptr;
  VKWIND11_LOG_WARN("D3D12Device::CreateCommandSignature — stub returning dummy");
  return S_OK;
}

void STDMETHODCALLTYPE D3D12Device::GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO* pResourceAllocationInfo) {
  if (!pResourceAllocationInfo || !pResourceDescs) return;

  uint64_t totalSize = 0;
  uint64_t maxAlignment = 256;

  for (UINT i = 0; i < numResourceDescs; i++) {
    const auto& desc = pResourceDescs[i];
    uint64_t size = 0;
    uint32_t bpp = 4;

    switch (desc.Dimension) {
      case D3D12_RESOURCE_DIMENSION_BUFFER:
        size = (desc.Width + 255) & ~255ULL;
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
      case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        size = (uint64_t)desc.Width * desc.Height * bpp * desc.DepthOrArraySize;
        break;
      case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        size = (uint64_t)desc.Width * desc.Height * desc.DepthOrArraySize * bpp;
        break;
      default:
        break;
    }
    totalSize += size;
  }

  pResourceAllocationInfo->SizeInBytes = (totalSize + 65535) & ~65535ULL;
  pResourceAllocationInfo->Alignment = maxAlignment;
}

HRESULT STDMETHODCALLTYPE D3D12Device::GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE HeapType, D3D12_HEAP_PROPERTIES* pHeapProperties) {
  if (!pHeapProperties) return E_POINTER;
  pHeapProperties->Type = HeapType;
  pHeapProperties->CPUPageProperty = 0;
  pHeapProperties->MemoryPoolPreference = 0;
  pHeapProperties->CreationNodeMask = 1;
  pHeapProperties->VisibleNodeMask = 1;
  return S_OK;
}

D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE D3D12Device::GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1) {
  D3D12_RESOURCE_ALLOCATION_INFO info = {};
  GetResourceAllocationInfo(visibleMask, numResourceDescs, pResourceDescs, &info);
  return info;
}
