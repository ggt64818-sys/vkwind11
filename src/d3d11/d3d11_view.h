#pragma once

#include "d3d11_device.h"

// ============================================================================
// D3D11 Views (SRV, RTV, DSV)
// ============================================================================

class D3D11ShaderResourceView final : public ID3D11ShaderResourceView {
public:
  D3D11ShaderResourceView(D3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc, ID3D11Resource* resource);
  ~D3D11ShaderResourceView();

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;

  // ID3D11DeviceChild
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  // ID3D11View
  void STDMETHODCALLTYPE GetResource(ID3D11Resource** ppResource) override;

  // ID3D11ShaderResourceView
  void STDMETHODCALLTYPE GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc) override;

private:
  D3D11Device* m_device;
  ID3D11Resource* m_resource;
  D3D11_SHADER_RESOURCE_VIEW_DESC m_desc;
  const char* m_debugName = nullptr;
};

class D3D11RenderTargetView final : public ID3D11RenderTargetView {
public:
  D3D11RenderTargetView(D3D11Device* device, const D3D11_RENDER_TARGET_VIEW_DESC& desc, ID3D11Resource* resource);
  ~D3D11RenderTargetView();

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}
  void STDMETHODCALLTYPE GetResource(ID3D11Resource** ppResource) override;
  void STDMETHODCALLTYPE GetDesc(D3D11_RENDER_TARGET_VIEW_DESC* pDesc) override;

private:
  D3D11Device* m_device;
  ID3D11Resource* m_resource;
  D3D11_RENDER_TARGET_VIEW_DESC m_desc;
  const char* m_debugName = nullptr;
};

class D3D11DepthStencilView final : public ID3D11DepthStencilView {
public:
  D3D11DepthStencilView(D3D11Device* device, const D3D11_DEPTH_STENCIL_VIEW_DESC& desc, ID3D11Resource* resource);
  ~D3D11DepthStencilView();

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}
  void STDMETHODCALLTYPE GetResource(ID3D11Resource** ppResource) override;
  void STDMETHODCALLTYPE GetDesc(D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc) override;

private:
  D3D11Device* m_device;
  ID3D11Resource* m_resource;
  D3D11_DEPTH_STENCIL_VIEW_DESC m_desc;
  const char* m_debugName = nullptr;
};
