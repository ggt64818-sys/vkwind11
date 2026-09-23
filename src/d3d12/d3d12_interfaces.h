#pragma once

// ============================================================================
// D3D12 Interfaces — COM vtables matching d3d12_device.h implementations
// All interfaces inherit from RefCounted for AddRef/Release/virtual dtor
// ============================================================================

#include "d3d12_types.h"
#include "../common/refcount.h"
#include <cstdint>

// ============================================================================
// ID3D12Object
// ============================================================================

struct ID3D12Object : public RefCounted {
  virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetName(const wchar_t* Name) = 0;
};

// ============================================================================
// ID3D12DeviceChild
// ============================================================================

struct ID3D12DeviceChild : ID3D12Object {
  virtual HRESULT STDMETHODCALLTYPE GetDevice(REFIID riid, void** ppDevice) = 0;
};

// ============================================================================
// ID3D12RootSignature
// ============================================================================

struct ID3D12RootSignature : ID3D12DeviceChild {};

// ============================================================================
// ID3D12PipelineState
// ============================================================================

struct ID3D12PipelineState : ID3D12DeviceChild {
  virtual HRESULT STDMETHODCALLTYPE GetCachedBlob(void** ppBlob) = 0;
};

// ============================================================================
// ID3D12Resource
// ============================================================================

struct ID3D12Resource : ID3D12DeviceChild {
  virtual HRESULT STDMETHODCALLTYPE Map(UINT Subresource, const D3D12_RANGE* pReadRange, void** ppData) = 0;
  virtual void STDMETHODCALLTYPE Unmap(UINT Subresource, const D3D12_RANGE* pWrittenRange) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDesc(D3D12_RESOURCE_DESC* pDesc) = 0;
  virtual D3D12_GPU_VIRTUAL_ADDRESS STDMETHODCALLTYPE GetGPUVirtualAddress() = 0;
  virtual HRESULT STDMETHODCALLTYPE WriteToSubresource(UINT DstSubresource, const D3D12_RANGE* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) = 0;
  virtual HRESULT STDMETHODCALLTYPE ReadFromSubresource(void* pDstData, UINT SrcSubresource, const D3D12_RANGE* pSrcBox, UINT SrcRowPitch, UINT DstDepthPitch) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetHeapProperties(void* pHeapProperties, void* pResidencyPriority) = 0;
};

// ============================================================================
// ID3D12Pageable
// ============================================================================

struct ID3D12Pageable : ID3D12DeviceChild {};

// ============================================================================
// ID3D12Heap
// ============================================================================

struct ID3D12Heap : ID3D12Pageable {
  virtual HRESULT STDMETHODCALLTYPE GetDesc(D3D12_HEAP_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D12Fence
// ============================================================================

struct ID3D12Fence : ID3D12Pageable {
  virtual uint64_t STDMETHODCALLTYPE GetCompletedValue() = 0;
  virtual HRESULT STDMETHODCALLTYPE SetEventOnCompletion(uint64_t Value, void* hEvent) = 0;
  virtual HRESULT STDMETHODCALLTYPE Signal(uint64_t Value) = 0;
};

// ============================================================================
// ID3D12CommandAllocator
// ============================================================================

struct ID3D12CommandAllocator : ID3D12Pageable {
  virtual HRESULT STDMETHODCALLTYPE Reset() = 0;
};

// ============================================================================
// ID3D12CommandSignature
// ============================================================================

struct ID3D12CommandSignature : ID3D12DeviceChild {};

// ============================================================================
// ID3D12QueryHeap
// ============================================================================

struct ID3D12QueryHeap : ID3D12DeviceChild {};

// ============================================================================
// ID3D12DescriptorHeap
// ============================================================================

struct ID3D12DescriptorHeap : ID3D12DeviceChild {
  virtual D3D12_CPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetCPUDescriptorHandleForHeapStart() = 0;
  virtual D3D12_GPU_DESCRIPTOR_HANDLE STDMETHODCALLTYPE GetGPUDescriptorHandleForHeapStart() = 0;
  virtual D3D12_DESCRIPTOR_HEAP_DESC STDMETHODCALLTYPE GetDesc() = 0;
};

// ============================================================================
// ID3D12GraphicsCommandList (= ID3D12CommandList)
// ============================================================================

struct ID3D12GraphicsCommandList : ID3D12DeviceChild {
  virtual D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE GetCommandListType() = 0;
  virtual HRESULT STDMETHODCALLTYPE Close() = 0;
  virtual HRESULT STDMETHODCALLTYPE Reset(ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState) = 0;
  virtual void STDMETHODCALLTYPE ClearState(ID3D12PipelineState* pPipelineState) = 0;

  // Draw
  virtual void STDMETHODCALLTYPE DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) = 0;
  virtual void STDMETHODCALLTYPE DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, int32_t BaseVertexLocation, UINT StartInstanceLocation) = 0;
  virtual void STDMETHODCALLTYPE Dispatch(UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) = 0;

  // Copy
  virtual void STDMETHODCALLTYPE CopyBufferRegion(ID3D12Resource* pDstBuffer, uint64_t DstOffset, ID3D12Resource* pSrcBuffer, uint64_t SrcOffset, uint64_t NumBytes) = 0;
  virtual void STDMETHODCALLTYPE CopyTextureRegion(const D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ, const D3D12_TEXTURE_COPY_LOCATION* pSrc, const D3D12_BOX* pSrcBox) = 0;
  virtual void STDMETHODCALLTYPE CopyResource(ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource) = 0;
  virtual void STDMETHODCALLTYPE CopyTiles(ID3D12Resource*, const D3D12_TILED_RESOURCE_COORDINATE*, const D3D12_TILE_REGION_SIZE*, ID3D12Resource*, uint64_t, D3D12_TILE_COPY_FLAGS) = 0;
  virtual void STDMETHODCALLTYPE ResolveSubresource(ID3D12Resource* pDstResource, UINT DstSubresource, ID3D12Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) = 0;

  // Barrier
  virtual void STDMETHODCALLTYPE ResourceBarrier(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers) = 0;
  virtual void STDMETHODCALLTYPE ResourceBarrierBatch(UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers) = 0;

  // Query
  virtual void STDMETHODCALLTYPE DiscardResource(ID3D12Resource*, const D3D12_DISCARD_REGION*) = 0;
  virtual void STDMETHODCALLTYPE BeginQuery(ID3D12QueryHeap*, D3D12_QUERY_TYPE, UINT) = 0;
  virtual void STDMETHODCALLTYPE EndQuery(ID3D12QueryHeap*, D3D12_QUERY_TYPE, UINT) = 0;
  virtual void STDMETHODCALLTYPE ResolveQueryData(ID3D12QueryHeap*, D3D12_QUERY_TYPE, UINT, UINT, ID3D12Resource*, uint64_t) = 0;
  virtual void STDMETHODCALLTYPE SetPredication(ID3D12Resource*, uint64_t, D3D12_PREDICATION_OP) = 0;

  // Debug
  virtual void STDMETHODCALLTYPE SetMarkerBits(UINT, const void*, UINT) = 0;
  virtual void STDMETHODCALLTYPE BeginEventBits(UINT, const void*, UINT) = 0;
  virtual void STDMETHODCALLTYPE EndEventBits() = 0;
  virtual void STDMETHODCALLTYPE ExecuteIndirect(ID3D12CommandSignature*, UINT, ID3D12Resource*, uint64_t, ID3D12Resource*, uint64_t) = 0;
  virtual void STDMETHODCALLTYPE AtomicCopyBufferUINT64(ID3D12Resource*, uint64_t, ID3D12Resource*, uint64_t, uint64_t, ID3D12Resource* const*, const D3D12_SUBRESOURCE_RANGE_UINT64*, D3D12_COPY_FLAGS) = 0;

  // Root signature
  virtual void STDMETHODCALLTYPE SetComputeRootSignature(ID3D12RootSignature* pRootSignature) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) = 0;
  virtual void STDMETHODCALLTYPE SetComputeRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRootDescriptorTable(UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) = 0;
  virtual void STDMETHODCALLTYPE SetComputeRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRoot32BitConstant(UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) = 0;
  virtual void STDMETHODCALLTYPE SetComputeRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRoot32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) = 0;
  virtual void STDMETHODCALLTYPE SetComputeRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRootConstantBufferView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
  virtual void STDMETHODCALLTYPE SetComputeRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
  virtual void STDMETHODCALLTYPE SetComputeRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;
  virtual void STDMETHODCALLTYPE SetGraphicsRootUnorderedAccessView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) = 0;

  // State
  virtual void STDMETHODCALLTYPE IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) = 0;
  virtual void STDMETHODCALLTYPE RSSetViewports(UINT NumViewports, const D3D12_VIEWPORT* pViewports) = 0;
  virtual void STDMETHODCALLTYPE RSSetScissorRects(UINT NumRects, const D3D12_RECT* pRects) = 0;
  virtual void STDMETHODCALLTYPE OMSetBlendFactor(const float BlendFactor[4]) = 0;
  virtual void STDMETHODCALLTYPE OMSetStencilRef(UINT StencilRef) = 0;
  virtual void STDMETHODCALLTYPE SetPipelineState(ID3D12PipelineState* pPipelineState) = 0;

  // Descriptor heaps
  virtual void STDMETHODCALLTYPE SetDescriptorHeaps(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) = 0;
  virtual void STDMETHODCALLTYPE SetSamplerOnGPUHeap(UINT, D3D12_GPU_DESCRIPTOR_HANDLE) = 0;
  virtual void STDMETHODCALLTYPE SetDescriptorHeapsBatch(UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) = 0;

  // Compute debug
  virtual void STDMETHODCALLTYPE ComputeSetMarkerBits(UINT, const void*, UINT) = 0;
  virtual void STDMETHODCALLTYPE ComputeBeginEventBits(UINT, const void*, UINT) = 0;
  virtual void STDMETHODCALLTYPE ComputeEndEventBits() = 0;
  virtual void STDMETHODCALLTYPE ComputeExecuteIndirect(ID3D12CommandSignature*, UINT, ID3D12Resource*, uint64_t, ID3D12Resource*, uint64_t) = 0;

  // GraphicsCommandList1
  virtual void STDMETHODCALLTYPE CopyTextureRegion1(const D3D12_TEXTURE_COPY_LOCATION*, UINT, UINT, UINT, const D3D12_TEXTURE_COPY_LOCATION*, const D3D12_BOX*, D3D12_COPY_FLAGS) = 0;
  virtual void STDMETHODCALLTYPE WriteBufferImmediate(UINT, const D3D12_WRITEBUFFERIMMEDIATE_PARAMETER*, const D3D12_WRITEBUFFERIMMEDIATE_MODE*) = 0;
};

// ID3D12CommandList is a typedef for ID3D12GraphicsCommandList
using ID3D12CommandList = ID3D12GraphicsCommandList;

// ============================================================================
// ID3D12CommandQueue
// ============================================================================

struct ID3D12CommandQueue : ID3D12DeviceChild {
  virtual void STDMETHODCALLTYPE UpdateTileMappings(UINT, const D3D12_TILED_RESOURCE_COORDINATE*, const D3D12_TILE_REGION_SIZE*, ID3D12Heap*, UINT, const UINT*, const UINT*, const UINT*, D3D12_TILE_MAPPING_FLAGS) = 0;
  virtual void STDMETHODCALLTYPE CopyTileMappings(ID3D12Resource*, const D3D12_TILED_RESOURCE_COORDINATE*, ID3D12Resource*, const D3D12_TILED_RESOURCE_COORDINATE*, const D3D12_TILE_REGION_SIZE*, D3D12_TILE_MAPPING_FLAGS) = 0;
  virtual void STDMETHODCALLTYPE ExecuteCommandLists(UINT NumCommandLists, ID3D12GraphicsCommandList* const* ppCommandLists) = 0;
  virtual void STDMETHODCALLTYPE SetMarker(UINT, const void*, UINT) = 0;
  virtual void STDMETHODCALLTYPE BeginEvent(UINT, const void*, UINT) = 0;
  virtual void STDMETHODCALLTYPE EndEvent() = 0;
  virtual HRESULT STDMETHODCALLTYPE Signal(ID3D12Fence* pFence, uint64_t FenceValue) = 0;
  virtual HRESULT STDMETHODCALLTYPE Wait(ID3D12Fence* pFence, uint64_t FenceValue) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetTimestampFrequency(uint64_t* pFrequency) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetClockCalibration(uint64_t* pGpuTimestamp, uint64_t* pCpuTimestamp) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDesc(D3D12_COMMAND_QUEUE_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D12Device
// ============================================================================

struct ID3D12Device : ID3D12Object {
  virtual HRESULT STDMETHODCALLTYPE GetNodeCount(UINT* pNodeCount) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateCommandQueue(const D3D12_COMMAND_QUEUE_DESC* pDesc, REFIID riid, void** ppCommandQueue) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, REFIID riid, void** ppCommandAllocator) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateCommandList(UINT nodeMask, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* pCommandAllocator, ID3D12PipelineState* pInitialState, REFIID riid, void** ppCommandList) = 0;
  virtual HRESULT STDMETHODCALLTYPE CheckFeatureSupport(D3D12_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC* pDescriptorHeapDesc, REFIID riid, void** ppvHeap) = 0;
  virtual UINT STDMETHODCALLTYPE GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeapType) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateRootSignature(UINT nodeMask, const void* pBlobWithRootSignature, SIZE_T blobLengthInBytes, REFIID riid, void** ppvRootSignature) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateGraphicsPipelineState(const void* pDesc, REFIID riid, void** ppPipelineState) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateComputePipelineState(const void* pDesc, REFIID riid, void** ppPipelineState) = 0;
  virtual void STDMETHODCALLTYPE CreateConstantBufferView(const D3D12_CONSTANT_BUFFER_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) = 0;
  virtual void STDMETHODCALLTYPE CreateShaderResourceView(ID3D12Resource* pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) = 0;
  virtual void STDMETHODCALLTYPE CreateUnorderedAccessView(ID3D12Resource* pResource, ID3D12Resource* pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) = 0;
  virtual void STDMETHODCALLTYPE CreateRenderTargetView(ID3D12Resource* pResource, const D3D12_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) = 0;
  virtual void STDMETHODCALLTYPE CreateDepthStencilView(ID3D12Resource* pResource, const D3D12_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) = 0;
  virtual void STDMETHODCALLTYPE CreateSampler(const D3D12_SAMPLER_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateCommittedResource(const D3D12_HEAP_PROPERTIES* pHeapProperties, D3D12_HEAP_FLAGS HeapFlags, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riidResource, void** ppvResource) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateHeap(const D3D12_HEAP_DESC* pDesc, REFIID riid, void** ppvHeap) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreatePlacedResource(ID3D12Heap* pHeap, uint64_t HeapOffset, const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppvResource) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateReservedResource(const D3D12_RESOURCE_DESC* pDesc, D3D12_RESOURCE_STATES InitialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue, REFIID riid, void** ppResource) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateSharedHandle(ID3D12DeviceChild* pObject, const void* pAttributes, DWORD Access, const wchar_t* Name, HANDLE* pHandle) = 0;
  virtual HRESULT STDMETHODCALLTYPE OpenSharedHandle(HANDLE NTHandle, REFIID riid, void** ppvObj) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateFence(uint64_t InitialValue, D3D12_FENCE_FLAGS Flags, REFIID riid, void** ppFence) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason() = 0;
  virtual void STDMETHODCALLTYPE GetCopyableFootprints(const D3D12_RESOURCE_DESC* pResourceDesc, UINT FirstSubresource, UINT NumSubresources, uint64_t BaseOffset, D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts, UINT* pNumRows, uint64_t* pRowSizeInBytes, uint64_t* pTotalBytes) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateQueryHeap(const void* pDesc, REFIID riid, void** ppvHeap) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateCommandSignature(const void* pDesc, ID3D12RootSignature* pRootSignature, REFIID riid, void** ppvCommandSignature) = 0;
  virtual void STDMETHODCALLTYPE GetResourceAllocationInfo(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO* pResourceAllocationInfo) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCustomHeapProperties(UINT nodeMask, D3D12_HEAP_TYPE HeapType, D3D12_HEAP_PROPERTIES* pHeapProperties) = 0;
  virtual D3D12_RESOURCE_ALLOCATION_INFO STDMETHODCALLTYPE GetResourceAllocationInfo1(UINT visibleMask, UINT numResourceDescs, const D3D12_RESOURCE_DESC* pResourceDescs, D3D12_RESOURCE_ALLOCATION_INFO1* pResourceAllocationInfo1) = 0;
};

// D3D12 Entry Points — declared in d3d12_main.cpp
HRESULT D3D12CreateDevice(IUnknown* pAdapter, UINT Flags, REFIID riid, void** ppDevice);
HRESULT D3D12GetDebugInterface(REFIID riid, void** ppDebug);
UINT D3D12SerializeRootSignature(const D3D12_ROOT_SIGNATURE_DESC* pRootSignatureDesc, UINT Version, void** ppBlob, void** ppErrorBlob);
