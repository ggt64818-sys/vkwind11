#include "d3d12_device.h"
#include "../common/logging.h"

// ============================================================================
// D3D12 Heap
// ============================================================================

D3D12HeapImpl::D3D12HeapImpl(D3D12Device* device, const D3D12_HEAP_DESC* desc)
  : m_device(device), m_desc(*desc) {
  VKWIND11_LOG_INFO("D3D12HeapImpl created size=%llu", desc->SizeInBytes);
}

D3D12HeapImpl::~D3D12HeapImpl() {
  VKWIND11_LOG_INFO("D3D12HeapImpl destroyed");
}

HRESULT STDMETHODCALLTYPE D3D12HeapImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12Heap || riid == IID_ID3D12Pageable || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12Heap*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12HeapImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

HRESULT STDMETHODCALLTYPE D3D12HeapImpl::GetDesc(D3D12_HEAP_DESC* pDesc) {
  if (!pDesc) return E_POINTER;
  *pDesc = m_desc;
  return S_OK;
}
