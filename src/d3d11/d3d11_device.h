#pragma once

#include "d3d11_interfaces.h"
#include "../vulkan/vk_device.h"
#include "../common/config.h"
#include <memory>
#include <vector>
#include <cstring>
#include <unordered_map>
#include <mutex>

// ============================================================================
// D3D11 Device Implementation
// ============================================================================

class D3D11Device : public ID3D11Device {
public:
  D3D11Device();
  ~D3D11Device() override;

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

  // ID3D11Device
  HRESULT STDMETHODCALLTYPE CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) override;
  HRESULT STDMETHODCALLTYPE CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D) override;
  HRESULT STDMETHODCALLTYPE CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) override;
  HRESULT STDMETHODCALLTYPE CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D) override;

  HRESULT STDMETHODCALLTYPE CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView) override;
  HRESULT STDMETHODCALLTYPE CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) override;
  HRESULT STDMETHODCALLTYPE CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) override;
  HRESULT STDMETHODCALLTYPE CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView) override;

  HRESULT STDMETHODCALLTYPE CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader) override;
  HRESULT STDMETHODCALLTYPE CreateHullShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader) override;
  HRESULT STDMETHODCALLTYPE CreateDomainShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader) override;
  HRESULT STDMETHODCALLTYPE CreateGeometryShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader) override;
  HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput(const void* pShaderBytecode, SIZE_T BytecodeLength, const void* pSODeclaration, UINT NumEntries, const UINT* BufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader) override;
  HRESULT STDMETHODCALLTYPE CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader) override;
  HRESULT STDMETHODCALLTYPE CreateComputeShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader) override;

  HRESULT STDMETHODCALLTYPE CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState) override;
  HRESULT STDMETHODCALLTYPE CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) override;
  HRESULT STDMETHODCALLTYPE CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) override;
  HRESULT STDMETHODCALLTYPE CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) override;

  HRESULT STDMETHODCALLTYPE CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout) override;

  HRESULT STDMETHODCALLTYPE CreateQuery(const void* pQueryDesc, ID3D11Query** ppQuery) override;
  HRESULT STDMETHODCALLTYPE CreatePredicate(const void* pPredicateDesc, ID3D11Predicate** ppPredicate) override;

  HRESULT STDMETHODCALLTYPE CreateClassLinkage(ID3D11ClassLinkage** ppLinkage) override;

  void STDMETHODCALLTYPE GetImmediateContext(ID3D11DeviceContext** ppImmediateContext) override;
  HRESULT STDMETHODCALLTYPE CheckFeatureLevel(D3D11_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) override;
  UINT STDMETHODCALLTYPE CheckFormatSupport(DXGI_FORMAT Format) override;
  HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason() override;

  // Vulkan device access
  VulkanDevice& getVulkanDevice() { return *m_vkDevice; }

  // Vulkan swapchain / render pass ownership (populated by D3D11CreateDeviceAndSwapChain)
  VulkanSwapchain& getSwapchain() { return m_vkSwapchain; }
  const VulkanSwapchain& getSwapchain() const { return m_vkSwapchain; }
  VkRenderPass getRenderPass() const { return m_vkRenderPass; }
  void setSwapchain(VulkanSwapchain&& sc) { m_vkSwapchain = std::move(sc); }
  void setRenderPass(VkRenderPass rp) { m_vkRenderPass = rp; }
  void setImmediateContext(D3D11DeviceContext* ctx) { m_immediateContext = ctx; }

  // Shader module cache: maps shader object → VkShaderModule
  // Avoids re-translating SM4→SPIR-V when same shader is set again
  VkShaderModule getOrCreateShaderModuleFromCache(const void* shaderObj, const std::vector<uint32_t>& spirv);
  void invalidateShaderModuleCache(const void* shaderObj);

private:
  std::unique_ptr<VulkanDevice> m_vkDevice;
  D3D11DeviceContext* m_immediateContext = nullptr;
  VulkanSwapchain m_vkSwapchain;
  VkRenderPass m_vkRenderPass = VK_NULL_HANDLE;
  std::string m_debugName;

  // Shader module cache (keyed by shader object pointer)
  std::unordered_map<const void*, VkShaderModule> m_shaderModuleCache;
  std::mutex m_shaderCacheMutex;
};

// ============================================================================
// D3D11 Buffer
// ============================================================================

class D3D11BufferImpl : public ID3D11Buffer {
public:
  D3D11BufferImpl(D3D11Device* device, const D3D11_BUFFER_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData);
  ~D3D11BufferImpl() override;

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

  // ID3D11Resource
  void STDMETHODCALLTYPE GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) override;
  void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) override;
  UINT STDMETHODCALLTYPE GetEvictionPriority() override;

  // ID3D11Buffer
  void STDMETHODCALLTYPE GetDesc(D3D11_BUFFER_DESC* pDesc) override;

  // ID3D11DeviceChild
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return ""; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  D3D11_BUFFER_DESC desc;
  VulkanBuffer vkBuffer;
  D3D11Device* device;
  VulkanBuffer mapStagingBuffer;
};

// ============================================================================
// D3D11 Texture2D
// ============================================================================

class D3D11Texture2DImpl : public ID3D11Texture2D {
public:
  D3D11Texture2DImpl(D3D11Device* device, const D3D11_TEXTURE2D_DESC* desc, const D3D11_SUBRESOURCE_DATA* initialData);
  ~D3D11Texture2DImpl() override;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

  void STDMETHODCALLTYPE GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) override;
  void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) override;
  UINT STDMETHODCALLTYPE GetEvictionPriority() override;

  void STDMETHODCALLTYPE GetDesc(D3D11_TEXTURE2D_DESC* pDesc) override;

  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) override;
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) override;
  const char* STDMETHODCALLTYPE GetDebugName() override { return ""; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  D3D11_TEXTURE2D_DESC desc;
  VulkanImage vkImage;
  D3D11Device* device;
  VulkanBuffer mapStagingBuffer;
  void* directMapped = nullptr; // For staging textures (HOST_VISIBLE direct map)
};

// ============================================================================
// D3D11 InputLayout
// ============================================================================

class D3D11InputLayoutImpl final : public ID3D11InputLayout {
public:
  D3D11InputLayoutImpl(D3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* descs, UINT count, const void* bytecode, SIZE_T bytecodeLen)
    : m_device(device) {
    if (descs && count > 0) {
      m_elements.resize(count);
      memcpy(m_elements.data(), descs, count * sizeof(D3D11_INPUT_ELEMENT_DESC));
    }
    if (bytecode && bytecodeLen > 0) {
      m_bytecode.resize(bytecodeLen);
      memcpy(m_bytecode.data(), bytecode, bytecodeLen);
    }
  }
  ~D3D11InputLayoutImpl() override = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_ID3D11InputLayout || riid == IID_ID3D11DeviceChild || riid == IID_IUnknown) {
      AddRef();
      *ppvObject = static_cast<ID3D11InputLayout*>(this);
      return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override {
    if (ppDevice) { *ppDevice = m_device; if (m_device) m_device->AddRef(); }
  }
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT* pDataSize, void*) override { if (pDataSize) *pDataSize = 0; return S_OK; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return S_OK; }
  const char* STDMETHODCALLTYPE GetDebugName() override { return nullptr; }
  void STDMETHODCALLTYPE SetDebugName(const char*) override {}

  const std::vector<D3D11_INPUT_ELEMENT_DESC>& getElements() const { return m_elements; }
  const std::vector<uint8_t>& getBytecode() const { return m_bytecode; }

private:
  D3D11Device* m_device;
  std::vector<D3D11_INPUT_ELEMENT_DESC> m_elements;
  std::vector<uint8_t> m_bytecode;
};
