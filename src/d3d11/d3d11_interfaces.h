#pragma once

#include "d3d11_types.h"
#include "../common/refcount.h"
#include <vector>

// ============================================================================
// Forward declarations
// ============================================================================

class D3D11Device;
class D3D11DeviceContext;
class D3D11Buffer;
class D3D11Texture1D;
class D3D11Texture2D;
class D3D11Texture3D;
class D3D11ShaderResourceView;
class D3D11RenderTargetView;
class D3D11DepthStencilView;
class D3D11UnorderedAccessView;
class D3D11VertexShader;
class D3D11HullShader;
class D3D11DomainShader;
class D3D11GeometryShader;
class D3D11PixelShader;
class D3D11ComputeShader;
class D3D11BlendState;
class D3D11DepthStencilState;
class D3D11RasterizerState;
class D3D11SamplerState;
class D3D11InputLayout;
class D3D11Query;
class D3D11Predicate;
class D3D11ClassInstance;
class D3D11ClassLinkage;

// Forward-declare interface types used in early definitions
struct ID3D11Device;

// DXGI
class DXGIFactory;
class DXGIFactory1;
class DXGIFactory2;
class DXGIAdapter;
class DXGIDevice;
class DXGISwapChain;

// ============================================================================
// ID3D11DeviceChild
// ============================================================================

struct ID3D11DeviceChild : public RefCounted {
  virtual void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) = 0;
  virtual const char* STDMETHODCALLTYPE GetDebugName() = 0;
  virtual void STDMETHODCALLTYPE SetDebugName(const char* Name) = 0;
};

// ============================================================================
// ID3D11Resource
// ============================================================================

struct ID3D11Resource : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) = 0;
  virtual void STDMETHODCALLTYPE SetEvictionPriority(UINT EvictionPriority) = 0;
  virtual UINT STDMETHODCALLTYPE GetEvictionPriority() = 0;
};

// ============================================================================
// ID3D11Buffer
// ============================================================================

struct ID3D11Buffer : public ID3D11Resource {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_BUFFER_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11Texture1D
// ============================================================================

struct ID3D11Texture1D : public ID3D11Resource {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_TEXTURE1D_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11Texture2D
// ============================================================================

struct ID3D11Texture2D : public ID3D11Resource {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_TEXTURE2D_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11Texture3D
// ============================================================================

struct ID3D11Texture3D : public ID3D11Resource {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_TEXTURE3D_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11View
// ============================================================================

struct ID3D11View : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetResource(ID3D11Resource** ppResource) = 0;
};

// ============================================================================
// ID3D11ShaderResourceView
// ============================================================================

struct ID3D11ShaderResourceView : public ID3D11View {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11RenderTargetView
// ============================================================================

struct ID3D11RenderTargetView : public ID3D11View {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_RENDER_TARGET_VIEW_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11DepthStencilView
// ============================================================================

struct ID3D11DepthStencilView : public ID3D11View {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11UnorderedAccessView
// ============================================================================

struct ID3D11UnorderedAccessView : public ID3D11View {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11VertexShader / PixelShader / etc.
// ============================================================================

struct ID3D11VertexShader : public ID3D11DeviceChild {
  // Shader bytecode stored internally
};

struct ID3D11HullShader : public ID3D11DeviceChild {};
struct ID3D11DomainShader : public ID3D11DeviceChild {};

struct ID3D11GeometryShader : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetClassInstance(UINT ClassInstanceIndex, UINT* pBufferCount, void* pBuffer) = 0;
};

struct ID3D11PixelShader : public ID3D11DeviceChild {};

struct ID3D11ComputeShader : public ID3D11DeviceChild {};

// ============================================================================
// ID3D11ClassInstance / ClassLinkage
// ============================================================================

struct ID3D11ClassInstance : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetClassLinkage(ID3D11ClassLinkage** ppLinkage) = 0;
};

struct ID3D11ClassLinkage : public ID3D11DeviceChild {
  virtual HRESULT STDMETHODCALLTYPE GetClassInstance(const char* pClassInstanceName, UINT InstanceIndex, ID3D11ClassInstance** ppInstance) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateClassInstance(const char* pClassTypeName, UINT ConstantBufferOffset, UINT ConstantVectorOffset, UINT TextureOffset, UINT SamplerOffset, ID3D11ClassInstance** ppInstance) = 0;
};

// ============================================================================
// ID3D11BlendState
// ============================================================================

struct ID3D11BlendState : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_BLEND_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11DepthStencilState
// ============================================================================

struct ID3D11DepthStencilState : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_DEPTH_STENCIL_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11RasterizerState
// ============================================================================

struct ID3D11RasterizerState : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_RASTERIZER_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11SamplerState
// ============================================================================

struct ID3D11SamplerState : public ID3D11DeviceChild {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_SAMPLER_DESC* pDesc) = 0;
};

// ============================================================================
// ID3D11InputLayout
// ============================================================================

struct ID3D11InputLayout : public ID3D11DeviceChild {};

// ============================================================================
// ID3D11Query / Predicate
// ============================================================================

struct ID3D11Asynchronous : public ID3D11DeviceChild {
  virtual UINT STDMETHODCALLTYPE GetDataSize() = 0;
};

struct ID3D11Query : public ID3D11Asynchronous {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_QUERY* pDesc) = 0;
};

struct ID3D11Predicate : public ID3D11Asynchronous {
  virtual void STDMETHODCALLTYPE GetDesc(D3D11_QUERY* pDesc) = 0;
};

// ============================================================================
// ID3D11Counter
// ============================================================================

struct ID3D11Counter : public ID3D11Asynchronous {
  // D3D11_COUNTER_DESC
};

// ============================================================================
// ID3D11DeviceContext
// ============================================================================

struct ID3D11DeviceContext : public ID3D11DeviceChild {
  // --- Immediate context methods ---

  // VSSetShader, VSSetConstantBuffers, VSSetShaderResources, VSSetSamplers
  virtual void STDMETHODCALLTYPE VSSetShader(ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) = 0;
  virtual void STDMETHODCALLTYPE VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers) = 0;
  virtual void STDMETHODCALLTYPE VSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) = 0;
  virtual void STDMETHODCALLTYPE VSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers) = 0;
  virtual void STDMETHODCALLTYPE VSGetShader(ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances) = 0;

  // PSSetShader, PSSetConstantBuffers, PSSetShaderResources, PSSetSamplers
  virtual void STDMETHODCALLTYPE PSSetShader(ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) = 0;
  virtual void STDMETHODCALLTYPE PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers) = 0;
  virtual void STDMETHODCALLTYPE PSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) = 0;
  virtual void STDMETHODCALLTYPE PSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers) = 0;
  virtual void STDMETHODCALLTYPE PSGetShader(ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances) = 0;

  // GSSetShader
  virtual void STDMETHODCALLTYPE GSSetShader(ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) = 0;

  // HSSetShader, DSSetShader
  virtual void STDMETHODCALLTYPE HSSetShader(ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) = 0;
  virtual void STDMETHODCALLTYPE DSSetShader(ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) = 0;

  // CSSetShader
  virtual void STDMETHODCALLTYPE CSSetShader(ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) = 0;

  // IA
  virtual void STDMETHODCALLTYPE IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) = 0;
  virtual void STDMETHODCALLTYPE IASetInputLayout(ID3D11InputLayout* pInputLayout) = 0;
  virtual void STDMETHODCALLTYPE IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets) = 0;
  virtual void STDMETHODCALLTYPE IASetIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset) = 0;
  virtual void STDMETHODCALLTYPE IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY* pTopology) = 0;
  virtual void STDMETHODCALLTYPE IAGetInputLayout(ID3D11InputLayout** ppInputLayout) = 0;
  virtual void STDMETHODCALLTYPE IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets) = 0;
  virtual void STDMETHODCALLTYPE IAGetIndexBuffer(ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset) = 0;

  // Draw
  virtual void STDMETHODCALLTYPE Draw(UINT VertexCount, UINT StartVertexLocation) = 0;
  virtual void STDMETHODCALLTYPE DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) = 0;
  virtual void STDMETHODCALLTYPE DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) = 0;
  virtual void STDMETHODCALLTYPE DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) = 0;
  virtual void STDMETHODCALLTYPE DrawAuto() = 0;

  // OM
  virtual void STDMETHODCALLTYPE OMSetRenderTargets(UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView) = 0;
  virtual void STDMETHODCALLTYPE OMSetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts) = 0;
  virtual void STDMETHODCALLTYPE OMSetBlendState(ID3D11BlendState* pBlendState, const float BlendFactor[4], UINT SampleMask) = 0;
  virtual void STDMETHODCALLTYPE OMSetDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef) = 0;
  virtual void STDMETHODCALLTYPE OMGetBlendState(ID3D11BlendState** ppBlendState, float* pBlendFactor, UINT* pSampleMask) = 0;
  virtual void STDMETHODCALLTYPE OMGetDepthStencilState(ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef) = 0;

  // RS
  virtual void STDMETHODCALLTYPE RSSetState(ID3D11RasterizerState* pRasterizerState) = 0;
  virtual void STDMETHODCALLTYPE RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports) = 0;
  virtual void STDMETHODCALLTYPE RSSetScissorRects(UINT NumRects, const D3D11_RECT* pRects) = 0;
  virtual void STDMETHODCALLTYPE RSGetState(ID3D11RasterizerState** ppRasterizerState) = 0;
  virtual void STDMETHODCALLTYPE RSGetViewports(UINT* pNumViewports, D3D11_VIEWPORT* pViewports) = 0;
  virtual void STDMETHODCALLTYPE RSGetScissorRects(UINT* pNumRects, D3D11_RECT* pRects) = 0;

  // Clear
  virtual void STDMETHODCALLTYPE ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, const float ColorRGBA[4]) = 0;
  virtual void STDMETHODCALLTYPE ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, float Depth, UINT8 Stencil) = 0;
  virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[4]) = 0;
  virtual void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView* pUnorderedAccessView, const float Values[4]) = 0;
  virtual void STDMETHODCALLTYPE ClearState() = 0;

  // Map / Unmap / Update
  virtual HRESULT STDMETHODCALLTYPE Map(ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource) = 0;
  virtual void STDMETHODCALLTYPE Unmap(ID3D11Resource* pResource, UINT Subresource) = 0;
  virtual void STDMETHODCALLTYPE UpdateSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) = 0;
  virtual void STDMETHODCALLTYPE CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource) = 0;
  virtual void STDMETHODCALLTYPE CopySubresourceRegion(ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox) = 0;

  // Query
  virtual void STDMETHODCALLTYPE Begin(ID3D11Asynchronous* pAsync) = 0;
  virtual HRESULT STDMETHODCALLTYPE End(ID3D11Asynchronous* pAsync) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetData(ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags) = 0;
  virtual void STDMETHODCALLTYPE SetPredication(ID3D11Predicate* pPredicate, BOOL PredicateValue) = 0;

  // SO
  virtual void STDMETHODCALLTYPE SOSetTargets(UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets) = 0;

  // Resolve
  virtual void STDMETHODCALLTYPE ResolveSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) = 0;

  // ExecuteCommandList
  virtual void STDMETHODCALLTYPE ExecuteCommandList(ID3D11CommandList* pCommandList, BOOL RestoreContextState) = 0;
};

// ============================================================================
// ID3D11Device
// ============================================================================

struct ID3D11Device : public RefCounted {
  // Resource creation
  virtual HRESULT STDMETHODCALLTYPE CreateBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateTexture1D(const D3D11_TEXTURE1D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture1D** ppTexture1D) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateTexture2D(const D3D11_TEXTURE2D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture2D** ppTexture2D) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateTexture3D(const D3D11_TEXTURE3D_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Texture3D** ppTexture3D) = 0;

  // View creation
  virtual HRESULT STDMETHODCALLTYPE CreateShaderResourceView(ID3D11Resource* pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC* pDesc, ID3D11ShaderResourceView** ppSRView) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateRenderTargetView(ID3D11Resource* pResource, const D3D11_RENDER_TARGET_VIEW_DESC* pDesc, ID3D11RenderTargetView** ppRTView) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilView(ID3D11Resource* pResource, const D3D11_DEPTH_STENCIL_VIEW_DESC* pDesc, ID3D11DepthStencilView** ppDepthStencilView) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateUnorderedAccessView(ID3D11Resource* pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC* pDesc, ID3D11UnorderedAccessView** ppUAView) = 0;

  // Shader creation
  virtual HRESULT STDMETHODCALLTYPE CreateVertexShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11VertexShader** ppVertexShader) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateHullShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11HullShader** ppHullShader) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateDomainShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11DomainShader** ppDomainShader) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateGeometryShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput(const void* pShaderBytecode, SIZE_T BytecodeLength, const void* pSODeclaration, UINT NumEntries, const UINT* BufferStrides, UINT NumStrides, UINT RasterizedStream, ID3D11ClassLinkage* pClassLinkage, ID3D11GeometryShader** ppGeometryShader) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreatePixelShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11PixelShader** ppPixelShader) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateComputeShader(const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage, ID3D11ComputeShader** ppComputeShader) = 0;

  // State creation
  virtual HRESULT STDMETHODCALLTYPE CreateBlendState(const D3D11_BLEND_DESC* pBlendStateDesc, ID3D11BlendState** ppBlendState) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateSamplerState(const D3D11_SAMPLER_DESC* pSamplerDesc, ID3D11SamplerState** ppSamplerState) = 0;

  // Input layout
  virtual HRESULT STDMETHODCALLTYPE CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength, ID3D11InputLayout** ppInputLayout) = 0;

  // Query
  virtual HRESULT STDMETHODCALLTYPE CreateQuery(const void* pQueryDesc, ID3D11Query** ppQuery) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreatePredicate(const void* pPredicateDesc, ID3D11Predicate** ppPredicate) = 0;

  // Class linkage
  virtual HRESULT STDMETHODCALLTYPE CreateClassLinkage(ID3D11ClassLinkage** ppLinkage) = 0;

  // Context
  virtual void STDMETHODCALLTYPE GetImmediateContext(ID3D11DeviceContext** ppImmediateContext) = 0;

  // Feature level
  virtual HRESULT STDMETHODCALLTYPE CheckFeatureLevel(D3D11_FEATURE Feature, void* pFeatureSupportData, UINT FeatureSupportDataSize) = 0;

  // Format support
  virtual UINT STDMETHODCALLTYPE CheckFormatSupport(DXGI_FORMAT Format) = 0;

  // Get info
  virtual HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason() = 0;
};

// ============================================================================
// IDXGIObject
// ============================================================================

struct IDXGIObject : public RefCounted {
  virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void** ppParent) = 0;
};

// ============================================================================
// IDXGIDeviceSubObject
// ============================================================================

struct IDXGIDeviceSubObject : public IDXGIObject {};

// ============================================================================
// IDXGIAdapter
// ============================================================================

struct IDXGIAdapter : public IDXGIObject {
  virtual HRESULT STDMETHODCALLTYPE EnumOutputs(UINT Output, void** ppOutput) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDesc(DXGI_ADAPTER_DESC* pDesc) = 0;
  virtual HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFIID InterfaceName, LARGE_INTEGER* pUMDVersion) = 0;
};

// ============================================================================
// IDXGIFactory
// ============================================================================

struct IDXGIFactory : public IDXGIObject {
  virtual HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) = 0;
  virtual HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND* pWindowHandle) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateSwapChain(void* pDevice, const DXGI_SWAP_CHAIN_DESC* pDesc, void** ppSwapChain) = 0;
};

// ============================================================================
// IDXGISwapChain
// ============================================================================

struct IDXGISwapChain : public IDXGIDeviceSubObject {
  virtual HRESULT STDMETHODCALLTYPE Present(UINT SyncInterval, UINT Flags) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, void* pTarget) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL* pFullscreen, void** ppTarget) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) = 0;
  virtual HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) = 0;
  virtual HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetContainingOutput(void** ppOutput) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFrameStatistics(void* pStats) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount) = 0;
};

// ============================================================================
// IDXGISwapChain1
// ============================================================================

struct IDXGISwapChain1 : public IDXGISwapChain {
  virtual HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetHwnd(HWND* pHwnd) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCoreWindow(REFIID refiid, void** ppCoreWindow) = 0;
  virtual HRESULT STDMETHODCALLTYPE Present1(UINT SyncInterval, UINT Flags, const void* pParameters) = 0;
  virtual BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported() = 0;
  virtual HRESULT STDMETHODCALLTYPE GetRestrictToOutput(void** ppRestrictToOutput) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetBackgroundColor(const float* pColor) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetBackgroundColor(float* pColor) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION Rotation) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION* pRotation) = 0;
};

// ============================================================================
// IDXGIDevice
// ============================================================================

struct IDXGIDevice : public IDXGIObject {
  virtual HRESULT STDMETHODCALLTYPE GetAdapter(IDXGIAdapter** pAdapter) = 0;
  virtual HRESULT STDMETHODCALLTYPE CreateSurface(const void* pDesc, UINT NumSurfaces, DXGI_USAGE Usage, const void* pSharedResource, void** ppSurface) = 0;
  virtual HRESULT STDMETHODCALLTYPE QueryResourceResidency(IUnknown* const* ppResources, void* pResidencyStatus, UINT NumResources) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetGPUThreadPriority(UINT Priority) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetGPUThreadPriority(UINT* pPriority) = 0;
};

// ============================================================================
// IDXGIDevice1
// ============================================================================

struct IDXGIDevice1 : public IDXGIDevice {
  virtual HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(UINT MaxLatency) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(UINT* pMaxLatency) = 0;
};

// ============================================================================
// D3D11 Command List
// ============================================================================

// ============================================================================
// Swap chain creation helper (defined in d3d11_dxgi.cpp)
// ============================================================================

IDXGISwapChain1* createSwapChainImpl(ID3D11Device* device, const DXGI_SWAP_CHAIN_DESC* desc);

// ============================================================================
// D3D11 Command List
// ============================================================================

struct ID3D11CommandList : public ID3D11DeviceChild {};
