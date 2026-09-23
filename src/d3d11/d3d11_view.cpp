#include "d3d11_view.h"
#include "../common/logging.h"

// ============================================================================
// D3D11 View Stubs — Phase 1 minimal implementation
// ============================================================================

// --- ShaderResourceView ---

D3D11ShaderResourceView::D3D11ShaderResourceView(D3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc, ID3D11Resource* resource)
  : m_device(device), m_resource(resource), m_desc(desc) {
  VKWIND11_LOG_INFO("D3D11ShaderResourceView created");
}

D3D11ShaderResourceView::~D3D11ShaderResourceView() {}

HRESULT D3D11ShaderResourceView::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11ShaderResourceView || riid == IID_ID3D11View || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef();
    *ppvObject = static_cast<ID3D11ShaderResourceView*>(this);
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

ULONG D3D11ShaderResourceView::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11ShaderResourceView::Release() { return RefCounted::Release(); }

void D3D11ShaderResourceView::GetDevice(ID3D11Device** ppDevice) {
  if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); }
}

HRESULT D3D11ShaderResourceView::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11ShaderResourceView::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11ShaderResourceView::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }

void D3D11ShaderResourceView::GetResource(ID3D11Resource** ppResource) {
  if (ppResource) { *ppResource = m_resource; }
}

void D3D11ShaderResourceView::GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc) {
  if (pDesc) *pDesc = m_desc;
}

// --- RenderTargetView ---

D3D11RenderTargetView::D3D11RenderTargetView(D3D11Device* device, const D3D11_RENDER_TARGET_VIEW_DESC& desc, ID3D11Resource* resource)
  : m_device(device), m_resource(resource), m_desc(desc) {
  VKWIND11_LOG_INFO("D3D11RenderTargetView created");
}

D3D11RenderTargetView::~D3D11RenderTargetView() {}

HRESULT D3D11RenderTargetView::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11RenderTargetView || riid == IID_ID3D11View || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef();
    *ppvObject = static_cast<ID3D11RenderTargetView*>(this);
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

ULONG D3D11RenderTargetView::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11RenderTargetView::Release() { return RefCounted::Release(); }

void D3D11RenderTargetView::GetDevice(ID3D11Device** ppDevice) {
  if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); }
}

HRESULT D3D11RenderTargetView::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11RenderTargetView::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11RenderTargetView::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }

void D3D11RenderTargetView::GetResource(ID3D11Resource** ppResource) {
  if (ppResource) { *ppResource = m_resource; }
}

void D3D11RenderTargetView::GetDesc(D3D11_RENDER_TARGET_VIEW_DESC* pDesc) {
  if (pDesc) *pDesc = m_desc;
}

// --- DepthStencilView ---

D3D11DepthStencilView::D3D11DepthStencilView(D3D11Device* device, const D3D11_DEPTH_STENCIL_VIEW_DESC& desc, ID3D11Resource* resource)
  : m_device(device), m_resource(resource), m_desc(desc) {
  VKWIND11_LOG_INFO("D3D11DepthStencilView created");
}

D3D11DepthStencilView::~D3D11DepthStencilView() {}

HRESULT D3D11DepthStencilView::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11DepthStencilView || riid == IID_ID3D11View || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef();
    *ppvObject = static_cast<ID3D11DepthStencilView*>(this);
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

ULONG D3D11DepthStencilView::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11DepthStencilView::Release() { return RefCounted::Release(); }

void D3D11DepthStencilView::GetDevice(ID3D11Device** ppDevice) {
  if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); }
}

HRESULT D3D11DepthStencilView::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11DepthStencilView::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11DepthStencilView::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }

void D3D11DepthStencilView::GetResource(ID3D11Resource** ppResource) {
  if (ppResource) { *ppResource = m_resource; }
}

void D3D11DepthStencilView::GetDesc(D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc) {
  if (pDesc) *pDesc = m_desc;
}
