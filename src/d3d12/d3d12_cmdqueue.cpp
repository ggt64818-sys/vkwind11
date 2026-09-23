#include "d3d12_device.h"
#include "../common/logging.h"

// ============================================================================
// D3D12 Command Queue Implementation
// ============================================================================

D3D12CommandQueueImpl::D3D12CommandQueueImpl(D3D12Device* device, const D3D12_COMMAND_QUEUE_DESC* desc)
  : m_device(device), m_desc(*desc) {
  auto& vk = device->getVulkanDevice();

  // Select queue family based on type
  switch (desc->Type) {
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
      m_queueFamilyIndex = vk.supportsCompute() ? vk.getGraphicsQueueFamily() : vk.getGraphicsQueueFamily();
      break;
    case D3D12_COMMAND_LIST_TYPE_COPY:
      m_queueFamilyIndex = vk.getGraphicsQueueFamily();
      break;
    default:
      m_queueFamilyIndex = vk.getGraphicsQueueFamily();
      break;
  }

  m_queue = vk.getGraphicsQueue();

  VKWIND11_LOG_INFO("D3D12CommandQueueImpl created type=%d queue=%p", desc->Type, m_queue);
}

D3D12CommandQueueImpl::~D3D12CommandQueueImpl() {
  // Queue is owned by VulkanDevice, don't destroy it
  m_queue = VK_NULL_HANDLE;
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12CommandQueue || riid == IID_ID3D12DeviceChild || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12CommandQueue*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

void STDMETHODCALLTYPE D3D12CommandQueueImpl::ExecuteCommandLists(
    UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
  if (!ppCommandLists || NumCommandLists == 0) return;

  auto& vk = m_device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  for (uint32_t i = 0; i < NumCommandLists; i++) {
    auto* cmdList = static_cast<D3D12GraphicsCommandListImpl*>(ppCommandLists[i]);
    VkCommandBuffer cmd = cmdList->getCommandBuffer();

    if (cmd == VK_NULL_HANDLE) {
      VKWIND11_LOG_WARN("D3D12CommandQueueImpl::ExecuteCommandLists — null command buffer at index %u", i);
      continue;
    }

    // End recording if still active (just try to end — VK_ERROR_DEVICE_LOST if not recording)
    vkEndCommandBuffer(cmd);

    // Submit
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkFence fence = VK_NULL_HANDLE;
    vkQueueSubmit(m_queue, 1, &submitInfo, fence);
  }

  VKWIND11_LOG_TRACE("D3D12CommandQueueImpl::ExecuteCommandLists count=%u", NumCommandLists);
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::Signal(ID3D12Fence* pFence, uint64_t FenceValue) {
  if (!pFence) return E_INVALIDARG;

  auto* fence = static_cast<D3D12FenceImpl*>(pFence);
  auto& vk = m_device->getVulkanDevice();

  VkFence vkFence = fence->getFence();
  VkResult result = vkQueueSubmit(m_queue, 0, nullptr, vkFence);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12CommandQueueImpl::Signal — vkQueueSubmit failed %d", result);
    return E_FAIL;
  }

  VKWIND11_LOG_TRACE("D3D12CommandQueueImpl::Signal value=%llu", FenceValue);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::Wait(ID3D12Fence* pFence, uint64_t FenceValue) {
  if (!pFence) return E_INVALIDARG;

  auto* fence = static_cast<D3D12FenceImpl*>(pFence);
  auto& vk = m_device->getVulkanDevice();

  VkFence vkFence = fence->getFence();
  VkResult result = vkWaitForFences(vk.getDevice(), 1, &vkFence, VK_TRUE, UINT64_MAX);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12CommandQueueImpl::Wait — vkWaitForFences failed %d", result);
    return E_FAIL;
  }

  VKWIND11_LOG_TRACE("D3D12CommandQueueImpl::Wait value=%llu", FenceValue);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::GetTimestampFrequency(uint64_t* pFrequency) {
  if (!pFrequency) return E_POINTER;
  // Assume 1 GHz timestamp (1 ns per tick)
  *pFrequency = 1000000000ULL;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::GetClockCalibration(uint64_t* pGpuTimestamp, uint64_t* pCpuTimestamp) {
  if (!pGpuTimestamp || !pCpuTimestamp) return E_POINTER;
  auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
  *pGpuTimestamp = (uint64_t)now;
  *pCpuTimestamp = (uint64_t)now;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE D3D12CommandQueueImpl::GetDesc(D3D12_COMMAND_QUEUE_DESC* pDesc) {
  if (!pDesc) return E_POINTER;
  pDesc->Type = m_desc.Type;
  pDesc->Priority = m_desc.Priority;
  pDesc->Flags = m_desc.Flags;
  pDesc->NodeMask = m_desc.NodeMask;
  return S_OK;
}
