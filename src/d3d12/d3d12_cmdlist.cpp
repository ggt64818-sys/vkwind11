#include "d3d12_device.h"
#include "../common/logging.h"
#include "../common/dxgi_utils.h"
#include <cstring>
#include <chrono>

// ============================================================================
// D3D12 Command Allocator
// ============================================================================

D3D12CommandAllocatorImpl::D3D12CommandAllocatorImpl(D3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
  : m_device(device), m_type(type) {
  auto& vk = device->getVulkanDevice();
  m_commandPool = vk.createCommandPool(vk.getGraphicsQueueFamily());

  if (m_commandPool == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("D3D12CommandAllocatorImpl: failed to create command pool");
  } else {
    VKWIND11_LOG_INFO("D3D12CommandAllocatorImpl created pool=%p", m_commandPool);
  }
}

D3D12CommandAllocatorImpl::~D3D12CommandAllocatorImpl() {
  if (m_device && m_commandPool != VK_NULL_HANDLE) {
    auto& vk = m_device->getVulkanDevice();
    vkDestroyCommandPool(vk.getDevice(), m_commandPool, nullptr);
    m_commandPool = VK_NULL_HANDLE;
  }
}

HRESULT STDMETHODCALLTYPE D3D12CommandAllocatorImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12CommandAllocator || riid == IID_ID3D12Pageable || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12CommandAllocator*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12CommandAllocatorImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

HRESULT STDMETHODCALLTYPE D3D12CommandAllocatorImpl::Reset() {
  if (m_commandPool == VK_NULL_HANDLE) return E_FAIL;

  auto& vk = m_device->getVulkanDevice();
  VkResult result = vkResetCommandPool(vk.getDevice(), m_commandPool, 0);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12CommandAllocatorImpl::Reset failed %d", result);
    return E_FAIL;
  }

  VKWIND11_LOG_TRACE("D3D12CommandAllocatorImpl::Reset");
  return S_OK;
}

// ============================================================================
// D3D12 Fence
// ============================================================================

D3D12FenceImpl::D3D12FenceImpl(D3D12Device* device, uint64_t initialValue, D3D12_FENCE_FLAGS flags)
  : m_device(device), m_flags(flags) {
  auto& vk = device->getVulkanDevice();
  m_fence = vk.createFence(true); // Create signaled

  if (m_fence == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("D3D12FenceImpl: failed to create fence");
  } else {
    VKWIND11_LOG_INFO("D3D12FenceImpl created fence=%p initial=%llu", m_fence, initialValue);
  }
}

D3D12FenceImpl::~D3D12FenceImpl() {
  if (m_device && m_fence != VK_NULL_HANDLE) {
    auto& vk = m_device->getVulkanDevice();
    vkDestroyFence(vk.getDevice(), m_fence, nullptr);
    m_fence = VK_NULL_HANDLE;
  }
}

HRESULT STDMETHODCALLTYPE D3D12FenceImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12Fence || riid == IID_ID3D12Pageable || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12Fence*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12FenceImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

uint64_t STDMETHODCALLTYPE D3D12FenceImpl::GetCompletedValue() {
  auto& vk = m_device->getVulkanDevice();
  VkResult result = vkGetFenceStatus(vk.getDevice(), m_fence);

  if (result == VK_SUCCESS) return 1;
  if (result == VK_NOT_READY) return 0;

  VKWIND11_LOG_ERROR("D3D12FenceImpl::GetCompletedValue failed %d", result);
  return 0;
}

HRESULT STDMETHODCALLTYPE D3D12FenceImpl::SetEventOnCompletion(uint64_t Value, void* hEvent) {
  // On Windows, we could use WaitForSingleObject on the event handle
  // For now, just wait on the fence directly
  auto& vk = m_device->getVulkanDevice();
  VkResult result = vkWaitForFences(vk.getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12FenceImpl::SetEventOnCompletion failed %d", result);
    return E_FAIL;
  }

  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12FenceImpl::Signal(uint64_t Value) {
  auto& vk = m_device->getVulkanDevice();

  VkResult result = vkResetFences(vk.getDevice(), 1, &m_fence);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12FenceImpl::Signal reset failed %d", result);
    return E_FAIL;
  }

  // Submit a no-op to signal the fence
  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  VkCommandPool tempPool = vk.createCommandPool(vk.getGraphicsQueueFamily());
  VkCommandBuffer tempCmd = vk.allocateCommandBuffer(tempPool);

  vkBeginCommandBuffer(tempCmd, &beginInfo);
  vkEndCommandBuffer(tempCmd);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &tempCmd;

  result = vkQueueSubmit(vk.getGraphicsQueue(), 1, &submitInfo, m_fence);

  vkDestroyCommandPool(vk.getDevice(), tempPool, nullptr);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12FenceImpl::Signal submit failed %d", result);
    return E_FAIL;
  }

  VKWIND11_LOG_TRACE("D3D12FenceImpl::Signal value=%llu", Value);
  return S_OK;
}

// ============================================================================
// D3D12 Graphics Command List
// ============================================================================

D3D12GraphicsCommandListImpl::D3D12GraphicsCommandListImpl(
    D3D12Device* device, D3D12_COMMAND_LIST_TYPE type,
    D3D12CommandAllocatorImpl* allocator, D3D12PipelineStateImpl* initialState)
  : m_device(device), m_type(type), m_allocator(allocator), m_currentPSO(initialState) {
  auto& vk = device->getVulkanDevice();

  m_commandPool = allocator->getCommandPool();
  m_cmdBuffer = vk.allocateCommandBuffer(m_commandPool);

  if (m_cmdBuffer == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("D3D12GraphicsCommandListImpl: failed to allocate command buffer");
  } else {
    VKWIND11_LOG_INFO("D3D12GraphicsCommandListImpl created cmd=%p pool=%p", m_cmdBuffer, m_commandPool);
  }

  m_viewports.resize(16);
  m_scissors.resize(16);
}

D3D12GraphicsCommandListImpl::~D3D12GraphicsCommandListImpl() {
  // Command buffer is freed when command pool is destroyed
  m_cmdBuffer = VK_NULL_HANDLE;
}

HRESULT STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;

  if (riid == IID_ID3D12GraphicsCommandList || riid == IID_ID3D12CommandList || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12GraphicsCommandList*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

D3D12_COMMAND_LIST_TYPE STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::GetCommandListType() {
  return m_type;
}

// ============================================================================
// State Management
// ============================================================================

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::ClearState(ID3D12PipelineState* pPipelineState) {
  m_currentPSO = static_cast<D3D12PipelineStateImpl*>(pPipelineState);
  m_graphicsRootSig = nullptr;
  m_computeRootSig = nullptr;
  m_viewports.clear();
  m_scissors.clear();
  memset(m_blendFactor, 0, sizeof(m_blendFactor));
  m_stencilRef = 0;
  memset(m_descriptorHeaps, 0, sizeof(m_descriptorHeaps));
  m_numRTVs = 0;
}

HRESULT STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::Close() {
  if (!m_isRecording) {
    VKWIND11_LOG_WARN("D3D12GraphicsCommandListImpl::Close — not recording");
    return S_OK;
  }

  auto& vk = m_device->getVulkanDevice();

  if (m_hasBegunRenderPass) {
    vkCmdEndRenderPass(m_cmdBuffer);
    m_hasBegunRenderPass = false;
  }

  VkResult result = vkEndCommandBuffer(m_cmdBuffer);
  m_isRecording = false;

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12GraphicsCommandListImpl::Close — vkEndCommandBuffer failed %d", result);
    return E_FAIL;
  }

  VKWIND11_LOG_TRACE("D3D12GraphicsCommandListImpl::Close");
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::Reset(
    ID3D12CommandAllocator* pAllocator, ID3D12PipelineState* pInitialState) {
  if (pAllocator) {
    m_allocator = static_cast<D3D12CommandAllocatorImpl*>(pAllocator);
    m_commandPool = m_allocator->getCommandPool();
  }
  if (pInitialState) {
    m_currentPSO = static_cast<D3D12PipelineStateImpl*>(pInitialState);
  }

  auto& vk = m_device->getVulkanDevice();
  vkFreeCommandBuffers(vk.getDevice(), m_commandPool, 1, &m_cmdBuffer);
  m_cmdBuffer = vk.allocateCommandBuffer(m_commandPool);

  if (m_cmdBuffer == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("D3D12GraphicsCommandListImpl::Reset — failed to reallocate command buffer");
    return E_FAIL;
  }

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(m_cmdBuffer, &beginInfo);

  m_isRecording = true;
  m_hasBegunRenderPass = false;

  VKWIND11_LOG_TRACE("D3D12GraphicsCommandListImpl::Reset");
  return S_OK;
}

// ============================================================================
// Draw Commands
// ============================================================================

void D3D12GraphicsCommandListImpl::ensureRenderPass() {
  if (m_hasBegunRenderPass || m_cmdBuffer == VK_NULL_HANDLE) return;
  // Begin a basic render pass if we have RTVs
  // For now, skip render pass management — games will use explicit barriers
  m_hasBegunRenderPass = true;
}

void D3D12GraphicsCommandListImpl::bindGraphicsPipeline() {
  if (!m_currentPSO || m_currentPSO->isCompute()) return;

  VkPipeline pipeline = m_currentPSO->getPipeline();
  if (pipeline != VK_NULL_HANDLE) {
    vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  // Apply viewport
  if (!m_viewports.empty()) {
    std::vector<VkViewport> vkViewports(m_viewports.size());
    for (size_t i = 0; i < m_viewports.size(); i++) {
      vkViewports[i].x = m_viewports[i].TopLeftX;
      vkViewports[i].y = m_viewports[i].TopLeftY + m_viewports[i].Height;
      vkViewports[i].width = m_viewports[i].Width;
      vkViewports[i].height = -m_viewports[i].Height;
      vkViewports[i].minDepth = m_viewports[i].MinDepth;
      vkViewports[i].maxDepth = m_viewports[i].MaxDepth;
    }
    vkCmdSetViewport(m_cmdBuffer, 0, (uint32_t)vkViewports.size(), vkViewports.data());
  }

  // Apply scissor
  if (!m_scissors.empty()) {
    std::vector<VkRect2D> vkScissors(m_scissors.size());
    for (size_t i = 0; i < m_scissors.size(); i++) {
      vkScissors[i].offset = { m_scissors[i].left, m_scissors[i].top };
      vkScissors[i].extent = {
        (uint32_t)(m_scissors[i].right - m_scissors[i].left),
        (uint32_t)(m_scissors[i].bottom - m_scissors[i].top)
      };
    }
    vkCmdSetScissor(m_cmdBuffer, 0, (uint32_t)vkScissors.size(), vkScissors.data());
  }

  // Blend factor
  vkCmdSetBlendConstants(m_cmdBuffer, m_blendFactor);

  // Stencil ref
  vkCmdSetStencilReference(m_cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, m_stencilRef);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::DrawInstanced(
    UINT VertexCountPerInstance, UINT InstanceCount,
    UINT StartVertexLocation, UINT StartInstanceLocation) {
  if (m_cmdBuffer == VK_NULL_HANDLE) return;

  bindGraphicsPipeline();
  ensureRenderPass();

  vkCmdDraw(m_cmdBuffer, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);

  VKWIND11_LOG_TRACE("DrawInstanced verts=%u instances=%u", VertexCountPerInstance, InstanceCount);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::DrawIndexedInstanced(
    UINT IndexCountPerInstance, UINT InstanceCount,
    UINT StartIndexLocation, int32_t BaseVertexLocation, UINT StartInstanceLocation) {
  if (m_cmdBuffer == VK_NULL_HANDLE) return;

  bindGraphicsPipeline();
  ensureRenderPass();

  vkCmdDrawIndexed(m_cmdBuffer, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);

  VKWIND11_LOG_TRACE("DrawIndexedInstanced indices=%u instances=%u", IndexCountPerInstance, InstanceCount);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::Dispatch(
    UINT ThreadGroupCountX, UINT ThreadGroupCountY, UINT ThreadGroupCountZ) {
  if (m_cmdBuffer == VK_NULL_HANDLE) return;

  if (m_currentPSO && m_currentPSO->isCompute()) {
    VkPipeline pipeline = m_currentPSO->getPipeline();
    if (pipeline != VK_NULL_HANDLE) {
      vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    }
  }

  vkCmdDispatch(m_cmdBuffer, ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);

  VKWIND11_LOG_TRACE("Dispatch groups=%u,%u,%u", ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

// ============================================================================
// Copy Commands
// ============================================================================

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::CopyBufferRegion(
    ID3D12Resource* pDstBuffer, uint64_t DstOffset,
    ID3D12Resource* pSrcBuffer, uint64_t SrcOffset, uint64_t NumBytes) {
  if (!pDstBuffer || !pSrcBuffer || m_cmdBuffer == VK_NULL_HANDLE) return;

  auto* dst = static_cast<D3D12ResourceImpl*>(pDstBuffer);
  auto* src = static_cast<D3D12ResourceImpl*>(pSrcBuffer);

  if (!dst->isBuffer() || !src->isBuffer()) {
    VKWIND11_LOG_WARN("CopyBufferRegion called on non-buffer resource");
    return;
  }

  VkBufferCopy copyRegion = { SrcOffset, DstOffset, NumBytes };
  vkCmdCopyBuffer(m_cmdBuffer, src->vkBuffer.buffer, dst->vkBuffer.buffer, 1, &copyRegion);

  VKWIND11_LOG_TRACE("CopyBufferRegion size=%llu", NumBytes);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::CopyTextureRegion(
    const D3D12_TEXTURE_COPY_LOCATION* pDst, UINT DstX, UINT DstY, UINT DstZ,
    const D3D12_TEXTURE_COPY_LOCATION* pSrc, const D3D12_BOX* pSrcBox) {
  if (!pDst || !pSrc || m_cmdBuffer == VK_NULL_HANDLE) return;

  auto* dstRes = static_cast<D3D12ResourceImpl*>(pDst->pResource);
  auto* srcRes = static_cast<D3D12ResourceImpl*>(pSrc->pResource);

  VkImageCopy copyRegion = {};
  copyRegion.srcOffset = { pSrcBox ? (int32_t)pSrcBox->left : 0, pSrcBox ? (int32_t)pSrcBox->top : 0, pSrcBox ? (int32_t)pSrcBox->front : 0 };
  copyRegion.dstOffset = { (int32_t)DstX, (int32_t)DstY, (int32_t)DstZ };
  copyRegion.srcSubresource = { convertDxgiToVkFormat(srcRes->m_desc.Format), 0, 0, 1 };
  copyRegion.dstSubresource = { convertDxgiToVkFormat(dstRes->m_desc.Format), 0, 0, 1 };
  copyRegion.extent = { srcRes->m_desc.Width > 0 ? (uint32_t)srcRes->m_desc.Width : 1, srcRes->m_desc.Height > 0 ? srcRes->m_desc.Height : 1, 1 };

  vkCmdCopyImage(m_cmdBuffer,
    srcRes->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dstRes->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &copyRegion);

  VKWIND11_LOG_TRACE("CopyTextureRegion");
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::CopyResource(
    ID3D12Resource* pDstResource, ID3D12Resource* pSrcResource) {
  if (!pDstResource || !pSrcResource || m_cmdBuffer == VK_NULL_HANDLE) return;

  auto* dst = static_cast<D3D12ResourceImpl*>(pDstResource);
  auto* src = static_cast<D3D12ResourceImpl*>(pSrcResource);

  if (dst->isBuffer() && src->isBuffer()) {
    VkBufferCopy copyRegion = {};
    copyRegion.size = src->m_desc.Width;
    vkCmdCopyBuffer(m_cmdBuffer, src->vkBuffer.buffer, dst->vkBuffer.buffer, 1, &copyRegion);
  } else if (!dst->isBuffer() && !src->isBuffer()) {
    VkImageCopy copyRegion = {};
    copyRegion.srcSubresource = { convertDxgiToVkFormat(src->m_desc.Format), 0, 0, 1 };
    copyRegion.dstSubresource = { convertDxgiToVkFormat(dst->m_desc.Format), 0, 0, 1 };
    copyRegion.extent = { (uint32_t)src->m_desc.Width, src->m_desc.Height > 0 ? src->m_desc.Height : 1, 1 };

    vkCmdCopyImage(m_cmdBuffer,
      src->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
      dst->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      1, &copyRegion);
  }

  VKWIND11_LOG_TRACE("CopyResource");
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::ResolveSubresource(
    ID3D12Resource* pDstResource, UINT DstSubresource,
    ID3D12Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) {
  if (!pDstResource || !pSrcResource || m_cmdBuffer == VK_NULL_HANDLE) return;

  auto* dst = static_cast<D3D12ResourceImpl*>(pDstResource);
  auto* src = static_cast<D3D12ResourceImpl*>(pSrcResource);

  if (dst->isBuffer() || src->isBuffer()) {
    VKWIND11_LOG_WARN("ResolveSubresource called on buffer resource");
    return;
  }

  VkImageResolve resolveRegion = {};
  resolveRegion.srcSubresource = { convertDxgiToVkFormat(Format), 0, 0, 1 };
  resolveRegion.dstSubresource = { convertDxgiToVkFormat(Format), 0, 0, 1 };
  resolveRegion.extent = { (uint32_t)src->m_desc.Width, src->m_desc.Height > 0 ? src->m_desc.Height : 1, 1 };

  vkCmdResolveImage(m_cmdBuffer,
    src->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    dst->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    1, &resolveRegion);

  VKWIND11_LOG_TRACE("ResolveSubresource");
}

// ============================================================================
// Resource Barriers
// ============================================================================

VkPipelineStageFlags D3D12GraphicsCommandListImpl::stateToAccessFlags(D3D12_RESOURCE_STATES state) {
  VkPipelineStageFlags flags = 0;
  if (state & (D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT))
    flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
  if (state & D3D12_RESOURCE_STATE_RENDER_TARGET)
    flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  if (state & D3D12_RESOURCE_STATE_DEPTH_WRITE)
    flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  if (state & D3D12_RESOURCE_STATE_DEPTH_READ)
    flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
  if (state & (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
    flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  if (state & D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
  if (state & D3D12_RESOURCE_STATE_COPY_DEST)
    flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
  if (state & D3D12_RESOURCE_STATE_COPY_SOURCE)
    flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
  if (state & D3D12_RESOURCE_STATE_RESOLVE_DEST)
    flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
  if (state & D3D12_RESOURCE_STATE_RESOLVE_SOURCE)
    flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
  if (flags == 0) flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  return flags;
}

VkImageLayout D3D12GraphicsCommandListImpl::stateToImageLayout(D3D12_RESOURCE_STATES state) {
  if (state & D3D12_RESOURCE_STATE_RENDER_TARGET) return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  if (state & D3D12_RESOURCE_STATE_DEPTH_WRITE) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  if (state & D3D12_RESOURCE_STATE_DEPTH_READ) return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  if (state & D3D12_RESOURCE_STATE_COPY_DEST) return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  if (state & D3D12_RESOURCE_STATE_COPY_SOURCE) return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  if (state & (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE))
    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  if (state & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) return VK_IMAGE_LAYOUT_GENERAL;
  if (state & D3D12_RESOURCE_STATE_RESOLVE_DEST) return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  if (state & D3D12_RESOURCE_STATE_RESOLVE_SOURCE) return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  if (state & (D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_INDEX_BUFFER))
    return VK_IMAGE_LAYOUT_UNDEFINED;
  return VK_IMAGE_LAYOUT_UNDEFINED;
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::ResourceBarrier(
    UINT NumBarriers, const D3D12_RESOURCE_BARRIER* pBarriers) {
  if (!pBarriers || m_cmdBuffer == VK_NULL_HANDLE) return;

  for (uint32_t i = 0; i < NumBarriers; i++) {
    const auto& barrier = pBarriers[i];

    switch (barrier.Type) {
      case D3D12_RESOURCE_BARRIER_TYPE_TRANSITION: {
        const auto& trans = barrier.Transition;
        auto* resource = static_cast<D3D12ResourceImpl*>(trans.pResource);

        if (resource->isBuffer()) {
          VkBufferMemoryBarrier bufBarrier = {};
          bufBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
          bufBarrier.srcAccessMask = stateToAccessFlags(trans.StateBefore);
          bufBarrier.dstAccessMask = stateToAccessFlags(trans.StateAfter);
          bufBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
          bufBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
          bufBarrier.buffer = resource->vkBuffer.buffer;
          bufBarrier.offset = 0;
          bufBarrier.size = resource->m_desc.Width;

          vkCmdPipelineBarrier(m_cmdBuffer,
            stateToAccessFlags(trans.StateBefore),
            stateToAccessFlags(trans.StateAfter),
            0, 0, nullptr, 1, &bufBarrier, 0, nullptr);
        } else {
          VkImageMemoryBarrier imgBarrier = {};
          imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
          imgBarrier.srcAccessMask = stateToAccessFlags(trans.StateBefore);
          imgBarrier.dstAccessMask = stateToAccessFlags(trans.StateAfter);
          imgBarrier.oldLayout = stateToImageLayout(trans.StateBefore);
          imgBarrier.newLayout = stateToImageLayout(trans.StateAfter);
          imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
          imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
          imgBarrier.image = resource->vkImage.image;
          imgBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

          if (imgBarrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
              imgBarrier.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL) {
            imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
          }

          vkCmdPipelineBarrier(m_cmdBuffer,
            stateToAccessFlags(trans.StateBefore),
            stateToAccessFlags(trans.StateAfter),
            0, 0, nullptr, 0, nullptr, 1, &imgBarrier);
        }

        resource->setState(trans.StateAfter);
        break;
      }
      case D3D12_RESOURCE_BARRIER_TYPE_ALIASING: {
        // Aliasing barriers — just ensure memory is available
        const auto& alias = barrier.Aliasing;
        vkCmdPipelineBarrier(m_cmdBuffer,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
          0, 0, nullptr, 0, nullptr, 0, nullptr);
        break;
      }
      case D3D12_RESOURCE_BARRIER_TYPE_UAV: {
        const auto& uav = barrier.UAV;
        auto* resource = static_cast<D3D12ResourceImpl*>(uav.pResource);
        VkBufferMemoryBarrier uavBarrier = {};
        uavBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        uavBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        uavBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        uavBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        uavBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        if (resource->isBuffer()) {
          uavBarrier.buffer = resource->vkBuffer.buffer;
          uavBarrier.offset = 0;
          uavBarrier.size = resource->m_desc.Width;
          vkCmdPipelineBarrier(m_cmdBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, nullptr, 1, &uavBarrier, 0, nullptr);
        }
        break;
      }
    }
  }
}

// ============================================================================
// Root Signature Binding
// ============================================================================

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRootSignature(ID3D12RootSignature* pRootSignature) {
  m_computeRootSig = static_cast<D3D12RootSignatureImpl*>(pRootSignature);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature) {
  m_graphicsRootSig = static_cast<D3D12RootSignatureImpl*>(pRootSignature);

  // Bind the pipeline layout
  if (m_graphicsRootSig && m_currentPSO && !m_currentPSO->isCompute()) {
    // Pipeline layout is already set when pipeline was created
  }
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRootDescriptorTable(
    UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {
  if (!m_computeRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;

  auto& vk = m_device->getVulkanDevice();
  VkDescriptorSet set = (VkDescriptorSet)BaseDescriptor.ptr;
  vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
    m_computeRootSig->getPipelineLayout(), RootParameterIndex, 1, &set, 0, nullptr);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRootDescriptorTable(
    UINT RootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor) {
  if (!m_graphicsRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;

  auto& vk = m_device->getVulkanDevice();
  VkDescriptorSet set = (VkDescriptorSet)BaseDescriptor.ptr;
  vkCmdBindDescriptorSets(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    m_graphicsRootSig->getPipelineLayout(), RootParameterIndex, 1, &set, 0, nullptr);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRoot32BitConstant(
    UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) {
  if (!m_computeRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;

  vkCmdPushConstants(m_cmdBuffer, m_computeRootSig->getPipelineLayout(),
    VK_SHADER_STAGE_COMPUTE_BIT, DestOffsetIn32BitValues * 4, 4, &SrcData);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRoot32BitConstant(
    UINT RootParameterIndex, UINT SrcData, UINT DestOffsetIn32BitValues) {
  if (!m_graphicsRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;

  vkCmdPushConstants(m_cmdBuffer, m_graphicsRootSig->getPipelineLayout(),
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, DestOffsetIn32BitValues * 4, 4, &SrcData);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRoot32BitConstants(
    UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) {
  if (!m_computeRootSig || m_cmdBuffer == VK_NULL_HANDLE || !pSrcData) return;

  vkCmdPushConstants(m_cmdBuffer, m_computeRootSig->getPipelineLayout(),
    VK_SHADER_STAGE_COMPUTE_BIT, DestOffsetIn32BitValues * 4, Num32BitValuesToSet * 4, pSrcData);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRoot32BitConstants(
    UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues) {
  if (!m_graphicsRootSig || m_cmdBuffer == VK_NULL_HANDLE || !pSrcData) return;

  vkCmdPushConstants(m_cmdBuffer, m_graphicsRootSig->getPipelineLayout(),
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, DestOffsetIn32BitValues * 4, Num32BitValuesToSet * 4, pSrcData);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRootConstantBufferView(
    UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
  if (!m_computeRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;
  // For now, bind as generic buffer
  VKWIND11_LOG_TRACE("SetComputeRootConstantBufferView param=%u", RootParameterIndex);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRootConstantBufferView(
    UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
  if (!m_graphicsRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;
  VKWIND11_LOG_TRACE("SetGraphicsRootConstantBufferView param=%u", RootParameterIndex);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRootShaderResourceView(
    UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
  if (!m_computeRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;
  VKWIND11_LOG_TRACE("SetComputeRootShaderResourceView param=%u", RootParameterIndex);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRootShaderResourceView(
    UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
  if (!m_graphicsRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;
  VKWIND11_LOG_TRACE("SetGraphicsRootShaderResourceView param=%u", RootParameterIndex);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetComputeRootUnorderedAccessView(
    UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
  if (!m_computeRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;
  VKWIND11_LOG_TRACE("SetComputeRootUnorderedAccessView param=%u", RootParameterIndex);
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetGraphicsRootUnorderedAccessView(
    UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
  if (!m_graphicsRootSig || m_cmdBuffer == VK_NULL_HANDLE) return;
  VKWIND11_LOG_TRACE("SetGraphicsRootUnorderedAccessView param=%u", RootParameterIndex);
}

// ============================================================================
// State Setting
// ============================================================================

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::IASetPrimitiveTopology(
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology) {
  m_currentTopology = PrimitiveTopology;

  if (m_cmdBuffer == VK_NULL_HANDLE) return;

  VkPrimitiveTopology vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  switch (PrimitiveTopology) {
    case D3D_PRIMITIVE_TOPOLOGY_POINTLIST:    vkTopo = VK_PRIMITIVE_TOPOLOGY_POINT_LIST; break;
    case D3D_PRIMITIVE_TOPOLOGY_LINELIST:     vkTopo = VK_PRIMITIVE_TOPOLOGY_LINE_LIST; break;
    case D3D_PRIMITIVE_TOPOLOGY_LINESTRIP:    vkTopo = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; break;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST:  vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; break;
    case D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: vkTopo = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; break;
    default: break;
  }

  // Dynamic state is set during pipeline creation; topology is fixed at pipeline creation
  // We store it for reference
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::RSSetViewports(
    UINT NumViewports, const D3D12_VIEWPORT* pViewports) {
  if (!pViewports || m_cmdBuffer == VK_NULL_HANDLE) return;

  m_viewports.resize(NumViewports);
  memcpy(m_viewports.data(), pViewports, sizeof(D3D12_VIEWPORT) * NumViewports);

  std::vector<VkViewport> vkViewports(NumViewports);
  for (uint32_t i = 0; i < NumViewports; i++) {
    vkViewports[i].x = pViewports[i].TopLeftX;
    vkViewports[i].y = pViewports[i].TopLeftY + pViewports[i].Height;
    vkViewports[i].width = pViewports[i].Width;
    vkViewports[i].height = -pViewports[i].Height;
    vkViewports[i].minDepth = pViewports[i].MinDepth;
    vkViewports[i].maxDepth = pViewports[i].MaxDepth;
  }

  vkCmdSetViewport(m_cmdBuffer, 0, NumViewports, vkViewports.data());
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::RSSetScissorRects(
    UINT NumRects, const D3D12_RECT* pRects) {
  if (!pRects || m_cmdBuffer == VK_NULL_HANDLE) return;

  m_scissors.resize(NumRects);
  memcpy(m_scissors.data(), pRects, sizeof(D3D12_RECT) * NumRects);

  std::vector<VkRect2D> vkScissors(NumRects);
  for (uint32_t i = 0; i < NumRects; i++) {
    vkScissors[i].offset = { pRects[i].left, pRects[i].top };
    vkScissors[i].extent = {
      (uint32_t)(pRects[i].right - pRects[i].left),
      (uint32_t)(pRects[i].bottom - pRects[i].top)
    };
  }

  vkCmdSetScissor(m_cmdBuffer, 0, NumRects, vkScissors.data());
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::OMSetBlendFactor(const float BlendFactor[4]) {
  if (BlendFactor) {
    memcpy(m_blendFactor, BlendFactor, sizeof(float) * 4);
    if (m_cmdBuffer != VK_NULL_HANDLE) {
      vkCmdSetBlendConstants(m_cmdBuffer, m_blendFactor);
    }
  }
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::OMSetStencilRef(UINT StencilRef) {
  m_stencilRef = StencilRef;
  if (m_cmdBuffer != VK_NULL_HANDLE) {
    vkCmdSetStencilReference(m_cmdBuffer, VK_STENCIL_FACE_FRONT_AND_BACK, m_stencilRef);
  }
}

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetPipelineState(ID3D12PipelineState* pPipelineState) {
  m_currentPSO = static_cast<D3D12PipelineStateImpl*>(pPipelineState);
  if (m_currentPSO && m_cmdBuffer != VK_NULL_HANDLE) {
    VkPipeline pipeline = m_currentPSO->getPipeline();
    if (pipeline != VK_NULL_HANDLE) {
      VkPipelineBindPoint bindPoint = m_currentPSO->isCompute() ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS;
      vkCmdBindPipeline(m_cmdBuffer, bindPoint, pipeline);
    }
  }
}

// ============================================================================
// Descriptor Heaps
// ============================================================================

void STDMETHODCALLTYPE D3D12GraphicsCommandListImpl::SetDescriptorHeaps(
    UINT NumDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps) {
  for (uint32_t i = 0; i < NumDescriptorHeaps; i++) {
    auto* heap = static_cast<D3D12DescriptorHeapImpl*>(ppDescriptorHeaps[i]);
    auto desc = heap->GetDesc();

    switch (desc.Type) {
      case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
        m_descriptorHeaps[0] = heap;
        break;
      case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
        m_descriptorHeaps[1] = heap;
        break;
      default:
        break;
    }
  }
}
