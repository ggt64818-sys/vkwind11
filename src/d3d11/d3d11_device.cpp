#include "d3d11_device.h"
#include "d3d11_context.h"
#include "d3d11_view.h"
#include "d3d11_shader.h"
#include "d3d11_state.h"
#include "../common/logging.h"

// ============================================================================
// D3D11Device
// ============================================================================

D3D11Device::D3D11Device() : m_vkDevice(std::make_unique<VulkanDevice>()) {}
D3D11Device::~D3D11Device() {
  // Release the immediate context (COM refcount)
  if (m_immediateContext) {
    m_immediateContext->Release();
    m_immediateContext = nullptr;
  }

  // Destroy Vulkan swapchain resources before the device is torn down.
  // unique_ptr<VulkanDevice> is destroyed after these members (declaration order).
  if (m_vkDevice) {
    VkDevice dev = m_vkDevice->getDevice();
    if (dev) {
      if (m_vkRenderPass) {
        vkDestroyRenderPass(dev, m_vkRenderPass, nullptr);
        m_vkRenderPass = VK_NULL_HANDLE;
      }
      m_vkSwapchain.destroy(dev);
    }
  }
}

HRESULT STDMETHODCALLTYPE D3D11Device::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;

  if (riid == IID_ID3D11Device || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D11Device*>(this);
    AddRef();
    return S_OK;
  }

  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

// --- Resource Creation ---

HRESULT D3D11Device::CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) {
  if (!pDesc || !ppBuffer) return E_INVALIDARG;
  *ppBuffer = nullptr;

  auto buffer = new D3D11BufferImpl(this, pDesc, pInitialData);
  *ppBuffer = buffer;
  return S_OK;
}

HRESULT D3D11Device::CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D) {
  if (!pDesc || !ppTexture1D) return E_INVALIDARG;
  *ppTexture1D = nullptr;
  VKWIND11_LOG_WARN("CreateTexture1D stubbed — returning dummy");
  return S_OK;
}

HRESULT D3D11Device::CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) {
  if (!pDesc || !ppTexture2D) return E_INVALIDARG;
  *ppTexture2D = nullptr;

  auto texture = new D3D11Texture2DImpl(this, pDesc, pInitialData);
  *ppTexture2D = texture;
  return S_OK;
}

HRESULT D3D11Device::CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D) {
  if (!pDesc || !ppTexture3D) return E_INVALIDARG;
  *ppTexture3D = nullptr;
  VKWIND11_LOG_WARN("CreateTexture3D stubbed — returning dummy");
  return S_OK;
}


// --- View Creation ---

HRESULT D3D11Device::CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView) {
  if (!pResource || !ppSRView) return E_INVALIDARG;
  *ppSRView = nullptr;
  D3D11_SHADER_RESOURCE_VIEW_DESC defaultDesc = {};
  if (pDesc) {
    defaultDesc = *pDesc;
  } else {
    defaultDesc.Format = DXGI_FORMAT_UNKNOWN;
    defaultDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    defaultDesc.Texture2D.MipLevels = 1;
  }
  auto* srv = new D3D11ShaderResourceView(this, defaultDesc, pResource);
  *ppSRView = srv;
  return S_OK;
}

HRESULT D3D11Device::CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) {
  if (!pResource || !ppRTView) return E_INVALIDARG;
  *ppRTView = nullptr;
  D3D11_RENDER_TARGET_VIEW_DESC defaultDesc = {};
  if (pDesc) {
    defaultDesc = *pDesc;
  } else {
    defaultDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    defaultDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    defaultDesc.Texture2D.MipSlice = 0;
  }
  auto* rtv = new D3D11RenderTargetView(this, defaultDesc, pResource);
  *ppRTView = rtv;
  return S_OK;
}

HRESULT D3D11Device::CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) {
  if (!pResource || !ppDepthStencilView) return E_INVALIDARG;
  *ppDepthStencilView = nullptr;
  D3D11_DEPTH_STENCIL_VIEW_DESC defaultDesc = {};
  if (pDesc) {
    defaultDesc = *pDesc;
  } else {
    defaultDesc.Format = DXGI_FORMAT_D32_FLOAT;
    defaultDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    defaultDesc.Texture2D.MipSlice = 0;
  }
  auto* dsv = new D3D11DepthStencilView(this, defaultDesc, pResource);
  *ppDepthStencilView = dsv;
  return S_OK;
}

HRESULT D3D11Device::CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView) {
  if (!pResource || !ppUAView) return E_INVALIDARG;
  *ppUAView = nullptr;
  VKWIND11_LOG_WARN("CreateUnorderedAccessView stub");
  return S_OK;
}

// --- Shader Creation ---

HRESULT D3D11Device::CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader) {
  if (!pShaderBytecode || !ppVertexShader) return E_INVALIDARG;
  *ppVertexShader = nullptr;
  auto* shader = new D3D11VertexShader(this, pShaderBytecode, BytecodeLength);
  *ppVertexShader = shader;
  return S_OK;
}

HRESULT D3D11Device::CreateHullShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader) {
  if (!ppHullShader) return E_INVALIDARG;
  *ppHullShader = nullptr;
  auto* shader = new D3D11HullShader(this, pShaderBytecode, BytecodeLength);
  *ppHullShader = shader;
  return S_OK;
}

HRESULT D3D11Device::CreateDomainShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader) {
  if (!ppDomainShader) return E_INVALIDARG;
  *ppDomainShader = nullptr;
  auto* shader = new D3D11DomainShader(this, pShaderBytecode, BytecodeLength);
  *ppDomainShader = shader;
  return S_OK;
}

HRESULT D3D11Device::CreateGeometryShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader) {
  if (!ppGeometryShader) return E_INVALIDARG;
  *ppGeometryShader = nullptr;
  auto* shader = new D3D11GeometryShader(this, pShaderBytecode, BytecodeLength);
  *ppGeometryShader = shader;
  return S_OK;
}

HRESULT D3D11Device::CreateGeometryShaderWithStreamOutput(const void* pShaderBytecode, SIZE_T BytecodeLength, const void* pSODeclaration, UINT NumEntries, const UINT* BufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader) {
  if (!ppGeometryShader) return E_INVALIDARG;
  *ppGeometryShader = nullptr;
  auto* shader = new D3D11GeometryShader(this, pShaderBytecode, BytecodeLength);
  *ppGeometryShader = shader;
  return S_OK;
}

HRESULT D3D11Device::CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader) {
  if (!pShaderBytecode || !ppPixelShader) return E_INVALIDARG;
  *ppPixelShader = nullptr;
  auto* shader = new D3D11PixelShader(this, pShaderBytecode, BytecodeLength);
  *ppPixelShader = shader;
  return S_OK;
}

HRESULT D3D11Device::CreateComputeShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader) {
  if (!ppComputeShader) return E_INVALIDARG;
  *ppComputeShader = nullptr;
  auto* shader = new D3D11ComputeShader(this, pShaderBytecode, BytecodeLength);
  *ppComputeShader = shader;
  return S_OK;
}

// --- State Creation ---

HRESULT D3D11Device::CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState) {
  if (!pBlendStateDesc || !ppBlendState) return E_INVALIDARG;
  *ppBlendState = nullptr;
  auto* state = new D3D11BlendState(this);
  state->desc = *pBlendStateDesc;
  *ppBlendState = state;
  return S_OK;
}

HRESULT D3D11Device::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) {
  if (!pDepthStencilDesc || !ppDepthStencilState) return E_INVALIDARG;
  *ppDepthStencilState = nullptr;
  auto* state = new D3D11DepthStencilState(this);
  state->desc = *pDepthStencilDesc;
  *ppDepthStencilState = state;
  return S_OK;
}

HRESULT D3D11Device::CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) {
  if (!pRasterizerDesc || !ppRasterizerState) return E_INVALIDARG;
  *ppRasterizerState = nullptr;
  auto* state = new D3D11RasterizerState(this);
  state->desc = *pRasterizerDesc;
  *ppRasterizerState = state;
  return S_OK;
}

HRESULT D3D11Device::CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) {
  if (!pSamplerDesc || !ppSamplerState) return E_INVALIDARG;
  *ppSamplerState = nullptr;
  auto* state = new D3D11SamplerState(this);
  state->desc = *pSamplerDesc;
  *ppSamplerState = state;
  return S_OK;
}

HRESULT D3D11Device::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout) {
  if (!ppInputLayout) return E_INVALIDARG;
  *ppInputLayout = nullptr;
  auto* layout = new D3D11InputLayoutImpl(this, pInputElementDescs, NumElements, pShaderBytecodeWithInputSignature, BytecodeLength);
  *ppInputLayout = layout;
  return S_OK;
}

// --- Query ---

HRESULT D3D11Device::CreateQuery(const void* pQueryDesc, ID3D11Query** ppQuery) {
  if (!ppQuery) return E_INVALIDARG;
  *ppQuery = nullptr;
  VKWIND11_LOG_WARN("CreateQuery stub");
  return S_OK;
}

HRESULT D3D11Device::CreatePredicate(const void* pPredicateDesc, ID3D11Predicate** ppPredicate) {
  if (!ppPredicate) return E_INVALIDARG;
  *ppPredicate = nullptr;
  VKWIND11_LOG_WARN("CreatePredicate stub");
  return S_OK;
}

HRESULT D3D11Device::CreateClassLinkage(ID3D11ClassLinkage** ppLinkage) {
  if (!ppLinkage) return E_INVALIDARG;
  *ppLinkage = nullptr;
  return S_OK;
}

// --- Context ---
void D3D11Device::GetImmediateContext(ID3D11DeviceContext** ppImmediateContext) {
  if (ppImmediateContext) {
    *ppImmediateContext = m_immediateContext;
    if (m_immediateContext) m_immediateContext->AddRef();
  }
}

// --- Feature Level ---

HRESULT D3D11Device::CheckFeatureLevel(D3D11_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) {
  if (!pFeatureSupportData) return E_POINTER;
  memset(pFeatureSupportData, 0, FeatureSupportDataSize);
  VKWIND11_LOG_WARN("CheckFeatureLevel feature=%d — returning zeroed data", (int)Feature);
  return S_OK;
}

UINT D3D11Device::CheckFormatSupport(DXGI_FORMAT Format) {
  // TODO: implement format support queries
  // Return basic support for common formats
  UINT support = 0;
  switch (Format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R16G16_FLOAT:
      support = 0x1 | 0x2 | 0x4 | 0x10; // buffer, texture, texture_msaa, render_target
      break;
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
      support = 0x4 | 0x20; // texture, depth_stencil
      break;
    default:
      break;
  }
  return support;
}

HRESULT D3D11Device::GetDeviceRemovedReason() {
  return S_OK;
}

