#pragma once

#include "d3d11_interfaces.h"
#include "../shader/dxbc_parser.h"
#include <vector>
#include <string>

// ============================================================================
// D3D11 Shader Objects
// ============================================================================

class D3D11VertexShader : public ID3D11VertexShader {
public:
  D3D11VertexShader(D3D11Device* device, const void* bytecode, size_t size);
  ~D3D11VertexShader() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  const ParsedDXBC& getDXBC() const { return m_dxbc; }
  const std::vector<uint32_t>& getBytecode() const { return m_bytecode; }

private:
  D3D11Device* m_device;
  ParsedDXBC m_dxbc;
  std::vector<uint32_t> m_bytecode;
  const char* m_debugName = nullptr;
};

class D3D11PixelShader : public ID3D11PixelShader {
public:
  D3D11PixelShader(D3D11Device* device, const void* bytecode, size_t size);
  ~D3D11PixelShader() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  const ParsedDXBC& getDXBC() const { return m_dxbc; }
  const std::vector<uint32_t>& getBytecode() const { return m_bytecode; }

private:
  D3D11Device* m_device;
  ParsedDXBC m_dxbc;
  std::vector<uint32_t> m_bytecode;
  const char* m_debugName = nullptr;
};

class D3D11ComputeShader : public ID3D11ComputeShader {
public:
  D3D11ComputeShader(D3D11Device* device, const void* bytecode, size_t size);
  ~D3D11ComputeShader() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return m_debugName; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  const ParsedDXBC& getDXBC() const { return m_dxbc; }

private:
  D3D11Device* m_device;
  ParsedDXBC m_dxbc;
  std::vector<uint32_t> m_bytecode;
  const char* m_debugName = nullptr;
};

// ============================================================================
// Stub shader types — Hull, Domain, Geometry
// Store bytecode but have no SPIR-V translation yet
// ============================================================================

class D3D11HullShader : public ID3D11HullShader {
public:
  D3D11HullShader(class D3D11Device* device, const void* bytecode, size_t size);
  ~D3D11HullShader() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return nullptr; }
  void STDMETHODCALLTYPE SetDebugName(const char*) override {}

  const std::vector<uint32_t>& getBytecode() const { return m_bytecode; }

private:
  D3D11Device* m_device;
  std::vector<uint32_t> m_bytecode;
};

class D3D11DomainShader : public ID3D11DomainShader {
public:
  D3D11DomainShader(class D3D11Device* device, const void* bytecode, size_t size);
  ~D3D11DomainShader() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return nullptr; }
  void STDMETHODCALLTYPE SetDebugName(const char*) override {}

  const std::vector<uint32_t>& getBytecode() const { return m_bytecode; }

private:
  D3D11Device* m_device;
  std::vector<uint32_t> m_bytecode;
};

class D3D11GeometryShader : public ID3D11GeometryShader {
public:
  D3D11GeometryShader(class D3D11Device* device, const void* bytecode, size_t size);
  ~D3D11GeometryShader() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return nullptr; }
  void STDMETHODCALLTYPE SetDebugName(const char*) override {}
  void STDMETHODCALLTYPE GetClassInstance(UINT, UINT*, void*) override {}

  const std::vector<uint32_t>& getBytecode() const { return m_bytecode; }

private:
  D3D11Device* m_device;
  std::vector<uint32_t> m_bytecode;
};
