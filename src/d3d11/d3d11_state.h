#pragma once

#include "d3d11_interfaces.h"

// ============================================================================
// D3D11 State Objects
// ============================================================================
//
// Blend, Rasterizer, DepthStencil, Sampler state objects.
// These map to Vulkan pipeline state.
//

class D3D11BlendState : public ID3D11BlendState {
public:
  D3D11BlendState(D3D11Device* device) : m_device(device) {}
  ~D3D11BlendState() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}
  void STDMETHODCALLTYPE GetDesc(D3D11_BLEND_DESC* pDesc) override;

  D3D11Device* m_device = nullptr;
  D3D11_BLEND_DESC desc = {};
  const char* m_debugName = nullptr;
};

class D3D11RasterizerState : public ID3D11RasterizerState {
public:
  D3D11RasterizerState(D3D11Device* device) : m_device(device) {}
  ~D3D11RasterizerState() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}
  void STDMETHODCALLTYPE GetDesc(D3D11_RASTERIZER_DESC* pDesc) override;

  D3D11Device* m_device = nullptr;
  D3D11_RASTERIZER_DESC desc = {};
  const char* m_debugName = nullptr;
};

class D3D11DepthStencilState : public ID3D11DepthStencilState {
public:
  D3D11DepthStencilState(D3D11Device* device) : m_device(device) {}
  ~D3D11DepthStencilState() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}
  void STDMETHODCALLTYPE GetDesc(D3D11_DEPTH_STENCIL_DESC* pDesc) override;

  D3D11Device* m_device = nullptr;
  D3D11_DEPTH_STENCIL_DESC desc = {};
  const char* m_debugName = nullptr;
};

class D3D11SamplerState : public ID3D11SamplerState {
public:
  D3D11SamplerState(D3D11Device* device) : m_device(device) {}
  ~D3D11SamplerState() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}
  void STDMETHODCALLTYPE GetDesc(D3D11_SAMPLER_DESC* pDesc) override;

  D3D11Device* m_device = nullptr;
  D3D11_SAMPLER_DESC desc = {};
  const char* m_debugName = nullptr;
};
