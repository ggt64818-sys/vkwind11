#include "d3d11_shader.h"
#include "d3d11_device.h"
#include "../common/logging.h"

// ============================================================================
// D3D11 Shader Implementations
// ============================================================================

// --- VertexShader ---

D3D11VertexShader::D3D11VertexShader(D3D11Device* device, const void* bytecode, size_t size)
  : m_device(device) {
  if (bytecode && size >= 4) {
    m_bytecode.assign((const uint32_t*)bytecode, (const uint32_t*)bytecode + (size / 4));
    parseDXBC(m_bytecode.data(), size, m_dxbc);
  }
  VKWIND11_LOG_INFO("D3D11VertexShader created (%zu bytes)", size);
}

HRESULT D3D11VertexShader::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11VertexShader || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11VertexShader*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}

ULONG D3D11VertexShader::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11VertexShader::Release() { return RefCounted::Release(); }
void D3D11VertexShader::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); } }
HRESULT D3D11VertexShader::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11VertexShader::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11VertexShader::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }

// --- PixelShader ---

D3D11PixelShader::D3D11PixelShader(D3D11Device* device, const void* bytecode, size_t size)
  : m_device(device) {
  if (bytecode && size >= 4) {
    m_bytecode.assign((const uint32_t*)bytecode, (const uint32_t*)bytecode + (size / 4));
    parseDXBC(m_bytecode.data(), size, m_dxbc);
  }
  VKWIND11_LOG_INFO("D3D11PixelShader created (%zu bytes)", size);
}

HRESULT D3D11PixelShader::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11PixelShader || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11PixelShader*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}

ULONG D3D11PixelShader::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11PixelShader::Release() { return RefCounted::Release(); }
void D3D11PixelShader::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); } }
HRESULT D3D11PixelShader::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11PixelShader::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11PixelShader::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }

// --- ComputeShader ---

D3D11ComputeShader::D3D11ComputeShader(D3D11Device* device, const void* bytecode, size_t size)
  : m_device(device) {
  if (bytecode && size >= 4) {
    m_bytecode.assign((const uint32_t*)bytecode, (const uint32_t*)bytecode + (size / 4));
    parseDXBC(m_bytecode.data(), size, m_dxbc);
  }
  VKWIND11_LOG_INFO("D3D11ComputeShader created (%zu bytes)", size);
}

HRESULT D3D11ComputeShader::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11ComputeShader || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11ComputeShader*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}

ULONG D3D11ComputeShader::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11ComputeShader::Release() { return RefCounted::Release(); }
void D3D11ComputeShader::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); } }
HRESULT D3D11ComputeShader::GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11ComputeShader::SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) { return S_OK; }
HRESULT D3D11ComputeShader::SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) { return S_OK; }

// ============================================================================
// Stub Shaders — Hull, Domain, Geometry
// ============================================================================

D3D11HullShader::D3D11HullShader(D3D11Device* device, const void* bytecode, size_t size)
  : m_device(device) {
  if (bytecode && size >= 4)
    m_bytecode.assign((const uint32_t*)bytecode, (const uint32_t*)bytecode + (size / 4));
  VKWIND11_LOG_INFO("D3D11HullShader created (%zu bytes)", size);
}

HRESULT D3D11HullShader::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11HullShader || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11HullShader*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11HullShader::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11HullShader::Release() { return RefCounted::Release(); }
void D3D11HullShader::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); } }
HRESULT D3D11HullShader::GetPrivateData(REFGUID, UINT* pDataSize, void*) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11HullShader::SetPrivateData(REFGUID, UINT, const void*) { return S_OK; }
HRESULT D3D11HullShader::SetPrivateDataInterface(REFGUID, const IUnknown*) { return S_OK; }

D3D11DomainShader::D3D11DomainShader(D3D11Device* device, const void* bytecode, size_t size)
  : m_device(device) {
  if (bytecode && size >= 4)
    m_bytecode.assign((const uint32_t*)bytecode, (const uint32_t*)bytecode + (size / 4));
  VKWIND11_LOG_INFO("D3D11DomainShader created (%zu bytes)", size);
}

HRESULT D3D11DomainShader::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11DomainShader || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11DomainShader*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11DomainShader::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11DomainShader::Release() { return RefCounted::Release(); }
void D3D11DomainShader::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); } }
HRESULT D3D11DomainShader::GetPrivateData(REFGUID, UINT* pDataSize, void*) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11DomainShader::SetPrivateData(REFGUID, UINT, const void*) { return S_OK; }
HRESULT D3D11DomainShader::SetPrivateDataInterface(REFGUID, const IUnknown*) { return S_OK; }

D3D11GeometryShader::D3D11GeometryShader(D3D11Device* device, const void* bytecode, size_t size)
  : m_device(device) {
  if (bytecode && size >= 4)
    m_bytecode.assign((const uint32_t*)bytecode, (const uint32_t*)bytecode + (size / 4));
  VKWIND11_LOG_INFO("D3D11GeometryShader created (%zu bytes)", size);
}

HRESULT D3D11GeometryShader::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11GeometryShader || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
    AddRef(); *ppvObject = static_cast<ID3D11GeometryShader*>(this); return S_OK;
  }
  *ppvObject = nullptr; return E_NOINTERFACE;
}
ULONG D3D11GeometryShader::AddRef() { return RefCounted::AddRef(); }
ULONG D3D11GeometryShader::Release() { return RefCounted::Release(); }
void D3D11GeometryShader::GetDevice(ID3D11Device** ppDevice) { if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); } }
HRESULT D3D11GeometryShader::GetPrivateData(REFGUID, UINT* pDataSize, void*) { if (pDataSize) *pDataSize = 0; return S_OK; }
HRESULT D3D11GeometryShader::SetPrivateData(REFGUID, UINT, const void*) { return S_OK; }
HRESULT D3D11GeometryShader::SetPrivateDataInterface(REFGUID, const IUnknown*) { return S_OK; }
