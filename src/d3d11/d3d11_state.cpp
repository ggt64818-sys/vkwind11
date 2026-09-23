#include "d3d11_state.h"
#include "d3d11_device.h"
#include "../common/logging.h"

// ============================================================================
// D3D11 State Object Implementations
// ============================================================================

// --- BlendState ---
HRESULT D3D11BlendState::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11BlendState || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11BlendState*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11BlendState::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11BlendState::Release() { return RefCounted::Release(); }
void D3D11BlendState::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = static_cast<ID3D11Device*>(m_device); if (m_device) m_device->AddRef(); } }
HRESULT D3D11BlendState::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11BlendState::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11BlendState::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }
void D3D11BlendState::GetDesc(D3D11_BLEND_DESC* pDesc) { if (pDesc) *pDesc = desc; }

// --- RasterizerState ---
HRESULT D3D11RasterizerState::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11RasterizerState || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11RasterizerState*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11RasterizerState::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11RasterizerState::Release() { return RefCounted::Release(); }
void D3D11RasterizerState::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = static_cast<ID3D11Device*>(m_device); if (m_device) m_device->AddRef(); } }
HRESULT D3D11RasterizerState::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11RasterizerState::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11RasterizerState::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }
void D3D11RasterizerState::GetDesc(D3D11_RASTERIZER_DESC* pDesc) { if (pDesc) *pDesc = desc; }

// --- DepthStencilState ---
HRESULT D3D11DepthStencilState::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11DepthStencilState || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11DepthStencilState*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11DepthStencilState::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11DepthStencilState::Release() { return RefCounted::Release(); }
void D3D11DepthStencilState::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = static_cast<ID3D11Device*>(m_device); if (m_device) m_device->AddRef(); } }
HRESULT D3D11DepthStencilState::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11DepthStencilState::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11DepthStencilState::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }
void D3D11DepthStencilState::GetDesc(D3D11_DEPTH_STENCIL_DESC* pDesc) { if (pDesc) *pDesc = desc; }

// --- SamplerState ---
HRESULT D3D11SamplerState::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11SamplerState || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11SamplerState*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11SamplerState::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11SamplerState::Release() { return RefCounted::Release(); }
void D3D11SamplerState::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = static_cast<ID3D11Device*>(m_device); if (m_device) m_device->AddRef(); } }
HRESULT D3D11SamplerState::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11SamplerState::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11SamplerState::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }
void D3D11SamplerState::GetDesc(D3D11_SAMPLER_DESC* pDesc) { if (pDesc) *pDesc = desc; }
