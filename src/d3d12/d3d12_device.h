#pragma once

#include "d3d12_interfaces.h"
#include "d3d12_types.h"
#include "../vulkan/vk_device.h"
#include "../vulkan/vk_descriptor.h"
#include "../common/config.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

// ============================================================================
// D3D12 Device Implementation
// ============================================================================

class D3D12Device : public ID3D12Device {
public:
  D3D12Device();
  ~D3D12Device() override;

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

  // ID3D12Object
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) override;
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t* Name) override;

  // ID3D12Device
  HRESULT STDMETHODCALLTYPE GetNodeCount(UINT* pNodeCount) override;
  HRESULT STDMETHODCALLTYPE CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue) override;
  HRESULT STDMETHODCALLTYPE CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** ppCommandAllocator) override;
  HRESULT STDMETHODCALLTYPE CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList) override;
  HRESULT STDMETHODCALLTYPE CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) override;
  HRESULT STDMETHODCALLTYPE CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap) override;
  UINT STDMETHODCALLTYPE GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType) override;
  HRESULT STDMETHODCALLTYPE CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void** ppvRootSignature) override;
  HRESULT STDMETHODCALLTYPE CreateGraphicsPipelineState(const void* pDesc, REFIID riid, void** ppPipelineState) override;
  HRESULT STDMETHODCALLTYPE CreateComputePipelineState(const void* pDesc, REFIID riid, void** ppPipelineState) override;
  void STDMETHODCALLTYPE CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
  void STDMETHODCALLTYPE CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
  void STDMETHODCALLTYPE CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
  void STDMETHODCALLTYPE CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
  void STDMETHODCALLTYPE CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
  void STDMETHODCALLTYPE CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) override;
  HRESULT STDMETHODCALLTYPE CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource) override;
  HRESULT STDMETHODCALLTYPE CreateHeap(const D3D12_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap) override;
  HRESULT STDMETHODCALLTYPE CreatePlacedResource(ID3D12Heap* pHeap, uint64_t HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) override;
  HRESULT STDMETHODCALLTYPE CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) override;
  HRESULT STDMETHODCALLTYPE CreateSharedHandle(ID3D12DeviceChild* pObject, const void* pAttributes, DWORD Access, const wchar_t* Name, HANDLE* pHandle) override;
  HRESULT STDMETHODCALLTYPE OpenSharedHandle(HANDLE NTHandle, REFIID riid, void** ppvObj) override;
  HRESULT STDMETHODCALLTYPE CreateFence(uint64_t InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void** ppFence) override;
  HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason() override;
  void STDMETHODCALLTYPE GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, uint64_t BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, uint64_t* pRowSizeInBytes, uint64_t* pTotalBytes) override;
  HRESULT STDMETHODCALLTYPE CreateQueryHeap(const void* pDesc, REFIID riid, void** ppvHeap) override;
  HRESULT STDMETHODCALLTYPE CreateCommandSignature(const void* pDesc, ID3D12RootSignature* pRootSignature, REFIID riid, void** ppvCommandSignature) override;
  void STDMETHODCALLTYPE GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO* pResourceAllocationInfo) override;
  HRESULT STDMETHODCALLTYPE GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE HeapType, D3D12_HEAP_PROPERTIES* pHeapProperties) override;
  D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1) override;

  // Vulkan access
  VulkanDevice& getVulkanDevice() { return *m_vkDevice; }

private:
  std::unique_ptr<VulkanDevice> m_vkDevice;
  std::string m_debugName;
  uint32_t m_descriptorHandleSize[4] = {}; // CBV_SRV_UAV, Sampler, RTV, DSV
  D3D12_FEATURE_DATA_D3D12_OPTIONS m_options = {};
};

// ============================================================================
// D3D12 Descriptor Heap
// ============================================================================

class D3D12DescriptorHeapImpl : public ID3D12DescriptorHeap {
public:
  D3D12DescriptorHeapImpl(D3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC* desc);
  ~D3D12DescriptorHeapImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;

  D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetCPUDescriptorHandleForHeapStart() override;
  D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetGPUDescriptorHandleForHeapStart() override;
  D3D12_DESCRIPTOR_HEAP_DESC STDMETHODCALLTYPE GetDesc() override;

  // Internal
  VkDescriptorSet allocateDescriptor(VkDescriptorSetLayout layout);
  void updateBufferDescriptor(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range, VkDescriptorType type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  void updateImageDescriptor(VkDescriptorSet set, uint32_t binding, VkImageView view, VkSampler sampler, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  VkDescriptorPool getPool() const { return m_pool; }
  VkDevice getVkDevice() const;
  uint32_t getHandleIncrement() const { return m_handleIncrement; }

private:
  D3D12Device* m_device = nullptr;
  D3D12_DESCRIPTOR_HEAP_DESC m_desc = {};
  VkDescriptorPool m_pool = VK_NULL_HANDLE;
  uint32_t m_handleIncrement = 0;
  uint32_t m_maxSets = 256;
};

// ============================================================================
// D3D12 Resource
// ============================================================================

class D3D12ResourceImpl : public ID3D12Resource {
public:
  D3D12ResourceImpl(D3D12Device* device, const D3D12_RESOURCE_DESC* desc, D3D12_RESOURCE_STATES initialState);
  ~D3D12ResourceImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t* Name) override;
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;

  HRESULT STDMETHODCALLTYPE Map(UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData) override;
  void STDMETHODCALLTYPE Unmap(UINT Subresource, const D3D12_RANGE* pWrittenRange) override;
  HRESULT STDMETHODCALLTYPE GetDesc(D3D12_RESOURCE_DESC* pDesc) override;
  D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE GetGPUVirtualAddress() override;
  HRESULT STDMETHODCALLTYPE WriteToSubresource(UINT DstSubresource, const D3D12_RANGE* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) override;
  HRESULT STDMETHODCALLTYPE ReadFromSubresource(void* pDstData, UINT SrcSubresource, const D3D12_RANGE* pSrcBox, UINT SrcRowPitch, UINT SrcDepthPitch) override;
  HRESULT STDMETHODCALLTYPE GetHeapProperties(void* pHeapProperties, void* pResidencyPriority) override;

  // Internal — Vulkan backing stores
  VulkanBuffer vkBuffer = {};
  VulkanImage vkImage = {};
  bool isBuffer() const { return m_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER; }
  D3D12_RESOURCE_STATES getState() const { return m_currentState; }
  void setState(D3D12_RESOURCE_STATES state) { m_currentState = state; }
  bool allocateVulkanBacking();

  D3D12_RESOURCE_DESC m_desc = {};
  D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

private:
  D3D12Device* m_device = nullptr;
  void* m_stagingMemory = nullptr;
};

// ============================================================================
// D3D12 Heap
// ============================================================================

class D3D12HeapImpl : public ID3D12Heap {
public:
  D3D12HeapImpl(D3D12Device* device, const D3D12_HEAP_DESC* desc);
  ~D3D12HeapImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetDesc(D3D12_HEAP_DESC* pDesc) override;

private:
  D3D12Device* m_device = nullptr;
  D3D12_HEAP_DESC m_desc = {};
};

// ============================================================================
// D3D12 Root Signature
// ============================================================================

class D3D12RootSignatureImpl : public ID3D12RootSignature {
public:
  D3D12RootSignatureImpl(D3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC* desc);
  ~D3D12RootSignatureImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;

  // Internal
  VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }
  VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
  const D3D12_ROOT_SIGNATURE_DESC& getDesc() const { return m_desc; }
  uint32_t getPushConstantSize() const { return m_pushConstantSize; }

private:
  D3D12Device* m_device = nullptr;
  D3D12_ROOT_SIGNATURE_DESC m_desc = {};
  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
  uint32_t m_pushConstantSize = 0;
  std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};

// ============================================================================
// D3D12 Pipeline State
// ============================================================================

class D3D12PipelineStateImpl : public ID3D12PipelineState {
public:
  D3D12PipelineStateImpl() = default;
  ~D3D12PipelineStateImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetCachedBlob(void** ppBlob) override;

  bool createGraphics(D3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc);
  bool createCompute(D3D12Device* device, const D3D12_COMPUTE_PIPELINE_STATE_DESC* desc);
  void destroy();

  VkPipeline getPipeline() const { return m_pipeline; }
  VkPipelineLayout getPipelineLayout() const { return m_pipelineLayout; }
  bool isCompute() const { return m_isCompute; }

private:
  D3D12Device* m_device = nullptr;
  VkPipeline m_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
  bool m_isCompute = false;
  D3D12_GRAPHICS_PIPELINE_STATE_DESC m_graphicsDesc = {};
  D3D12_COMPUTE_PIPELINE_STATE_DESC m_computeDesc = {};
};

// ============================================================================
// D3D12 Command Allocator
// ============================================================================

class D3D12CommandAllocatorImpl : public ID3D12CommandAllocator {
public:
  D3D12CommandAllocatorImpl(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
  ~D3D12CommandAllocatorImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;
  HRESULT STDMETHODCALLTYPE Reset() override;

  VkCommandPool getCommandPool() const { return m_commandPool; }

private:
  D3D12Device* m_device = nullptr;
  D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  VkCommandPool m_commandPool = VK_NULL_HANDLE;
};

// ============================================================================
// D3D12 Command Queue
// ============================================================================

class D3D12CommandQueueImpl : public ID3D12CommandQueue {
public:
  D3D12CommandQueueImpl(D3D12Device* device, const D3D12_COMMAND_QUEUE_DESC* desc);
  ~D3D12CommandQueueImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;

  // Stubs for tiled resource methods
  void STDMETHODCALLTYPE UpdateTileMappings(UINT, const D3D12_TILED_RESOURCE_COORDINATE*, const D3D12_TILE_REGION_SIZE*, ID3D12Heap*, UINT, const UINT*, const UINT*, const UINT*, D3D12_TILE_MAPPING_FLAGS) override {}
  void STDMETHODCALLTYPE CopyTileMappings(ID3D12Resource*, const D3D12_TILED_RESOURCE_COORDINATE*, ID3D12Resource*, const D3D12_TILED_RESOURCE_COORDINATE*, const D3D12_TILE_REGION_SIZE*, D3D12_TILE_MAPPING_FLAGS) override {}

  void STDMETHODCALLTYPE ExecuteCommandLists(UINT NumCommandLists, ID3D12GraphicsCommandList* const* ppCommandLists) override;
  void STDMETHODCALLTYPE SetMarker(UINT, const void*, UINT) override {}
  void STDMETHODCALLTYPE BeginEvent(UINT, const void*, UINT) override {}
  void STDMETHODCALLTYPE EndEvent() override {}
  HRESULT STDMETHODCALLTYPE Signal(ID3D12Fence* pFence, uint64_t FenceValue) override;
  HRESULT STDMETHODCALLTYPE Wait(ID3D12Fence* pFence, uint64_t FenceValue) override;
  HRESULT STDMETHODCALLTYPE GetTimestampFrequency(uint64_t* pFrequency) override;
  HRESULT STDMETHODCALLTYPE GetClockCalibration(uint64_t* pGpuTimestamp, uint64_t* pCpuTimestamp) override;
  HRESULT STDMETHODCALLTYPE GetDesc(D3D12_COMMAND_QUEUE_DESC* pDesc) override;

  VkQueue getQueue() const { return m_queue; }

private:
  D3D12Device* m_device = nullptr;
  D3D12_COMMAND_QUEUE_DESC m_desc = {};
  VkQueue m_queue = VK_NULL_HANDLE;
  uint32_t m_queueFamilyIndex = 0;
};

// ============================================================================
// D3D12 Fence
// ============================================================================

class D3D12FenceImpl : public ID3D12Fence {
public:
  D3D12FenceImpl(D3D12Device* device, uint64_t initialValue, D3D12_FENCE_FLAGS flags);
  ~D3D12FenceImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;

  uint64_t STDMETHODCALLTYPE GetCompletedValue() override;
  HRESULT STDMETHODCALLTYPE SetEventOnCompletion(uint64_t Value, void* hEvent) override;
  HRESULT STDMETHODCALLTYPE Signal(uint64_t Value) override;

  VkFence getFence() const { return m_fence; }

private:
  D3D12Device* m_device = nullptr;
  VkFence m_fence = VK_NULL_HANDLE;
  D3D12_FENCE_FLAGS m_flags = D3D12_FENCE_FLAG_NONE;
};

// ============================================================================
// D3D12 Graphics Command List
// ============================================================================

class D3D12GraphicsCommandListImpl : public ID3D12GraphicsCommandList {
public:
  D3D12GraphicsCommandListImpl(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type, D3D12CommandAllocatorImpl* allocator, D3D12PipelineStateImpl* initialState);
  ~D3D12GraphicsCommandListImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetName(const wchar_t*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) override;
  D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE GetCommandListType() override;

  HRESULT STDMETHODCALLTYPE Close() override;
  HRESULT STDMETHODCALLTYPE Reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState) override;
  void STDMETHODCALLTYPE ClearState(ID3D12PipelineState* pPipelineState) override;

  // Draw
  void STDMETHODCALLTYPE DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) override;
  void STDMETHODCALLTYPE DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, int32_t BaseVertexLocation, UINT StartInstanceLocation) override;
  void STDMETHODCALLTYPE Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) override;

  // Copy
  void STDMETHODCALLTYPE CopyBufferRegion(ID3D12Resource* pDstBuffer, uint64_t DstOffset, ID3D12Resource* pSrcBuffer, uint64_t SrcOffset, uint64_t NumBytes) override;
  void STDMETHODCALLTYPE CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION* pSrc, const D3D12_BOX* pSrcBox) override;
  void STDMETHODCALLTYPE CopyResource(ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource) override;
  void STDMETHODCALLTYPE CopyTiles(ID3D12Resource*, const D3D12_TILED_RESOURCE_COORDINATE*, const D3D12_TILE_REGION_SIZE*, ID3D12Resource*, uint64_t, D3D12_TILE_COPY_FLAGS) override {}
  void STDMETHODCALLTYPE ResolveSubresource(ID3D12Resource* pDstResource, UINT DstSubresource, ID3D12Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) override;

  // Barrier
  void STDMETHODCALLTYPE ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers) override;
  void STDMETHODCALLTYPE ResourceBarrierBatch(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers) override { ResourceBarrier(NumBarriers, pBarriers); }

  // Query
  void STDMETHODCALLTYPE DiscardResource(ID3D12Resource*, const D3D12_DISCARD_REGION*) override {}
  void STDMETHODCALLTYPE BeginQuery(ID3D12QueryHeap*, D3D12_QUERY_TYPE, UINT) override {}
  void STDMETHODCALLTYPE EndQuery(ID3D12QueryHeap*, D3D12_QUERY_TYPE, UINT) override {}
  void STDMETHODCALLTYPE ResolveQueryData(ID3D12QueryHeap*, D3D12_QUERY_TYPE, UINT, UINT, ID3D12Resource*, uint64_t) override {}
  void STDMETHODCALLTYPE SetPredication(ID3D12Resource*, uint64_t, D3D12_PREDICATION_OP) override {}

  // Debug
  void STDMETHODCALLTYPE SetMarkerBits(UINT, const void*, UINT) override {}
  void STDMETHODCALLTYPE BeginEventBits(UINT, const void*, UINT) override {}
  void STDMETHODCALLTYPE EndEventBits() override {}
  void STDMETHODCALLTYPE ExecuteIndirect(ID3D12CommandSignature*, UINT, ID3D12Resource*, uint64_t, ID3D12Resource*, uint64_t) override {}
  void STDMETHODCALLTYPE AtomicCopyBufferUINT64(ID3D12Resource*, uint64_t, ID3D12Resource*, uint64_t, uint64_t, ID3D12Resource* const*, const D3D12_SUBRESOURCE_RANGE_UINT64*, D3D12_COPY_FLAGS) override {}

  // Root signature
  void STDMETHODCALLTYPE SetComputeRootSignature(ID3D12RootSignature* pRootSignature) override;
  void STDMETHODCALLTYPE SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) override;
  void STDMETHODCALLTYPE SetComputeRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
  void STDMETHODCALLTYPE SetGraphicsRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) override;
  void STDMETHODCALLTYPE SetComputeRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) override;
  void STDMETHODCALLTYPE SetGraphicsRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) override;
  void STDMETHODCALLTYPE SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) override;
  void STDMETHODCALLTYPE SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) override;
  void STDMETHODCALLTYPE SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
  void STDMETHODCALLTYPE SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
  void STDMETHODCALLTYPE SetComputeRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
  void STDMETHODCALLTYPE SetGraphicsRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
  void STDMETHODCALLTYPE SetComputeRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;
  void STDMETHODCALLTYPE SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) override;

  // State
  void STDMETHODCALLTYPE IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) override;
  void STDMETHODCALLTYPE RSSetViewports(UINT NumViewports, const D3D12_VIEWPORT* pViewports) override;
  void STDMETHODCALLTYPE RSSetScissorRects(UINT NumRects, const D3D12_RECT* pRects) override;
  void STDMETHODCALLTYPE OMSetBlendFactor(const float BlendFactor[4]) override;
  void STDMETHODCALLTYPE OMSetStencilRef(UINT StencilRef) override;
  void STDMETHODCALLTYPE SetPipelineState(ID3D12PipelineState* pPipelineState) override;

  // Descriptor heaps
  void STDMETHODCALLTYPE SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) override;
  void STDMETHODCALLTYPE SetSamplerOnGPUHeap(UINT, D3D12_GPU_DESCRIPTOR_HANDLE) override {}
  void STDMETHODCALLTYPE SetDescriptorHeapsBatch(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) override { SetDescriptorHeaps(NumDescriptorHeaps, ppDescriptorHeaps); }

  // Compute debug
  void STDMETHODCALLTYPE ComputeSetMarkerBits(UINT, const void*, UINT) override {}
  void STDMETHODCALLTYPE ComputeBeginEventBits(UINT, const void*, UINT) override {}
  void STDMETHODCALLTYPE ComputeEndEventBits() override {}
  void STDMETHODCALLTYPE ComputeExecuteIndirect(ID3D12CommandSignature*, UINT, ID3D12Resource*, uint64_t, ID3D12Resource*, uint64_t) override {}

  // GraphicsCommandList1
  void STDMETHODCALLTYPE CopyTextureRegion1(const D3D12_TEXTURE_COPY_LOCATION*, UINT, UINT, UINT, const D3D12_TEXTURE_COPY_LOCATION*, const D3D12_BOX*, D3D12_COPY_FLAGS) override {}
  void STDMETHODCALLTYPE WriteBufferImmediate(UINT, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER*, const D3D12_WRITEBUFFERIMMEDIATE_MODE*) override {}

  // Internal
  VkCommandBuffer getCommandBuffer() const { return m_cmdBuffer; }

private:
  void ensureRenderPass();
  void bindGraphicsPipeline();
  VkPipelineStageFlags stateToAccessFlags(D3D12_RESOURCE_STATES state);
  VkImageLayout stateToImageLayout(D3D12_RESOURCE_STATES state);

  D3D12Device* m_device = nullptr;
  D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  D3D12CommandAllocatorImpl* m_allocator = nullptr;
  D3D12PipelineStateImpl* m_currentPSO = nullptr;
  D3D12RootSignatureImpl* m_graphicsRootSig = nullptr;
  D3D12RootSignatureImpl* m_computeRootSig = nullptr;

  VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;
  VkCommandPool m_commandPool = VK_NULL_HANDLE;
  bool m_isRecording = false;
  bool m_hasBegunRenderPass = false;

  // Current state
  D3D12_PRIMITIVE_TOPOLOGY m_currentTopology = {};
  std::vector<D3D12_VIEWPORT> m_viewports;
  std::vector<D3D12_RECT> m_scissors;
  float m_blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  uint32_t m_stencilRef = 0;
  ID3D12DescriptorHeap* m_descriptorHeaps[2] = {}; // CBV_SRV_UAV, Sampler
  D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandles[8] = {};
  D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle = {};
  uint32_t m_numRTVs = 0;
};
