#pragma once

#include "d3d11_interfaces.h"
#include "../vulkan/vk_device.h"
#include "../vulkan/vk_pipeline.h"
#include "../shader/sm4_translator.h"
#include <vulkan/vulkan.h>
#include <memory>

// ============================================================================
// D3D11 DeviceContext — immediate context implementation
// ============================================================================

class D3D11DeviceContext : public ID3D11DeviceContext {
public:
  explicit D3D11DeviceContext(D3D11Device* device);
  ~D3D11DeviceContext() override;

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override;

  // ID3D11DeviceChild
  void STDMETHODCALLTYPE GetDevice(ID3D11Device** ppDevice) override;
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, UINT* pDataSize, void* pData) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, UINT DataSize, const void* pData) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID refguid, const IUnknown* pData) override { return E_NOTIMPL; }
  const char* STDMETHODCALLTYPE GetDebugName() override { return ""; }
  void STDMETHODCALLTYPE SetDebugName(const char* Name) override {}

  // VS
  void STDMETHODCALLTYPE VSSetShader(ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) override;
  void STDMETHODCALLTYPE VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers) override;
  void STDMETHODCALLTYPE VSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) override;
  void STDMETHODCALLTYPE VSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers) override;
  void STDMETHODCALLTYPE VSGetShader(ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances) override;

  // PS
  void STDMETHODCALLTYPE PSSetShader(ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) override;
  void STDMETHODCALLTYPE PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers) override;
  void STDMETHODCALLTYPE PSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) override;
  void STDMETHODCALLTYPE PSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers) override;
  void STDMETHODCALLTYPE PSGetShader(ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances) override;

  // GS, HS, DS, CS
  void STDMETHODCALLTYPE GSSetShader(ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) override;
  void STDMETHODCALLTYPE HSSetShader(ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) override;
  void STDMETHODCALLTYPE DSSetShader(ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) override;
  void STDMETHODCALLTYPE CSSetShader(ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) override;

  // IA
  void STDMETHODCALLTYPE IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) override;
  void STDMETHODCALLTYPE IASetInputLayout(ID3D11InputLayout* pInputLayout) override;
  void STDMETHODCALLTYPE IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets) override;
  void STDMETHODCALLTYPE IASetIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset) override;
  void STDMETHODCALLTYPE IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY* pTopology) override;
  void STDMETHODCALLTYPE IAGetInputLayout(ID3D11InputLayout** ppInputLayout) override;
  void STDMETHODCALLTYPE IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets) override;
  void STDMETHODCALLTYPE IAGetIndexBuffer(ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset) override;

  // Draw
  void STDMETHODCALLTYPE Draw(UINT VertexCount, UINT StartVertexLocation) override;
  void STDMETHODCALLTYPE DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) override;
  void STDMETHODCALLTYPE DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) override;
  void STDMETHODCALLTYPE DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) override;
  void STDMETHODCALLTYPE DrawAuto() override;

  // OM
  void STDMETHODCALLTYPE OMSetRenderTargets(UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView) override;
  void STDMETHODCALLTYPE OMSetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts) override;
  void STDMETHODCALLTYPE OMSetBlendState(ID3D11BlendState* pBlendState, const float BlendFactor[4], UINT SampleMask) override;
  void STDMETHODCALLTYPE OMSetDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef) override;
  void STDMETHODCALLTYPE OMGetBlendState(ID3D11BlendState** ppBlendState, float* pBlendFactor, UINT* pSampleMask) override;
  void STDMETHODCALLTYPE OMGetDepthStencilState(ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef) override;

  // RS
  void STDMETHODCALLTYPE RSSetState(ID3D11RasterizerState* pRasterizerState) override;
  void STDMETHODCALLTYPE RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports) override;
  void STDMETHODCALLTYPE RSSetScissorRects(UINT NumRects, const D3D11_RECT* pRects) override;
  void STDMETHODCALLTYPE RSGetState(ID3D11RasterizerState** ppRasterizerState) override;
  void STDMETHODCALLTYPE RSGetViewports(UINT* pNumViewports, D3D11_VIEWPORT* pViewports) override;
  void STDMETHODCALLTYPE RSGetScissorRects(UINT* pNumRects, D3D11_RECT* pRects) override;

  // Clear
  void STDMETHODCALLTYPE ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, const float ColorRGBA[4]) override;
  void STDMETHODCALLTYPE ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, float Depth, UINT8 Stencil) override;
  void STDMETHODCALLTYPE ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[4]) override;
  void STDMETHODCALLTYPE ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView* pUnorderedAccessView, const float Values[4]) override;
  void STDMETHODCALLTYPE ClearState() override;

  // Map / Unmap / Update
  HRESULT STDMETHODCALLTYPE Map(ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource) override;
  void STDMETHODCALLTYPE Unmap(ID3D11Resource* pResource, UINT Subresource) override;
  void STDMETHODCALLTYPE UpdateSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) override;
  void STDMETHODCALLTYPE CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource) override;
  void STDMETHODCALLTYPE CopySubresourceRegion(ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox) override;

  // Query
  void STDMETHODCALLTYPE Begin(ID3D11Asynchronous* pAsync) override;
  HRESULT STDMETHODCALLTYPE End(ID3D11Asynchronous* pAsync) override;
  HRESULT STDMETHODCALLTYPE GetData(ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags) override;
  void STDMETHODCALLTYPE SetPredication(ID3D11Predicate* pPredicate, BOOL PredicateValue) override;

  // SO
  void STDMETHODCALLTYPE SOSetTargets(UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets) override;

  // Resolve
  void STDMETHODCALLTYPE ResolveSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) override;

  // ExecuteCommandList
  void STDMETHODCALLTYPE ExecuteCommandList(ID3D11CommandList* pCommandList, BOOL RestoreContextState) override;

private:
  D3D11Device* m_device;

  // Cached state
  ID3D11VertexShader* m_vs = nullptr;
  ID3D11PixelShader* m_ps = nullptr;
  ID3D11GeometryShader* m_gs = nullptr;
  ID3D11HullShader* m_hs = nullptr;
  ID3D11DomainShader* m_ds = nullptr;
  ID3D11ComputeShader* m_cs = nullptr;

  ID3D11InputLayout* m_inputLayout = nullptr;
  ID3D11BlendState* m_blendState = nullptr;
  ID3D11DepthStencilState* m_depthStencilState = nullptr;
  ID3D11RasterizerState* m_rasterizerState = nullptr;

  D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

  struct VertexBufferSlot {
    ID3D11Buffer* buffer = nullptr;
    UINT stride = 0;
    UINT offset = 0;
  };
  VertexBufferSlot m_vertexBuffers[32] = {};

  ID3D11Buffer* m_indexBuffer = nullptr;
  DXGI_FORMAT m_indexFormat = DXGI_FORMAT_R16_UINT;
  UINT m_indexOffset = 0;

  struct ConstantBufferSlot {
    ID3D11Buffer* buffer = nullptr;
  };
  ConstantBufferSlot m_vsConstantBuffers[14] = {};
  ConstantBufferSlot m_psConstantBuffers[14] = {};

  ID3D11ShaderResourceView* m_vsShaderResources[128] = {};
  ID3D11ShaderResourceView* m_psShaderResources[128] = {};
  ID3D11SamplerState* m_vsSamplers[16] = {};
  ID3D11SamplerState* m_psSamplers[16] = {};

  ID3D11RenderTargetView* m_renderTargets[8] = {};
  ID3D11DepthStencilView* m_depthStencil = nullptr;

  float m_blendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  UINT m_sampleMask = 0xFFFFFFFF;
  UINT m_stencilRef = 0;

  D3D11_VIEWPORT m_viewports[16] = {};
  UINT m_numViewports = 0;
  D3D11_RECT m_scissorRects[16] = {};
  UINT m_numScissorRects = 0;

  // Vulkan command recording state
  VkCommandBuffer m_cmdBuffer = VK_NULL_HANDLE;
  VkCommandPool m_cmdPool = VK_NULL_HANDLE;
  VkRenderPass m_currentRenderPass = VK_NULL_HANDLE;
  VkFramebuffer m_currentFramebuffer = VK_NULL_HANDLE;
  VkPipeline m_currentPipeline = VK_NULL_HANDLE;
  VkPipelineLayout m_currentPipelineLayout = VK_NULL_HANDLE;
  bool m_inRenderPass = false;

  // Pipeline management
  VulkanPipelineManager m_pipelineManager;

  // Cached shader modules (translated from DXBC → SPIR-V once)
  VkShaderModule m_vsModule = VK_NULL_HANDLE;
  VkShaderModule m_psModule = VK_NULL_HANDLE;

  // Cached shader objects for re-translation check
  ID3D11VertexShader* m_boundVS = nullptr;
  ID3D11PixelShader* m_boundPS = nullptr;

  // Helpers
  void ensureCommandBuffer();
  void beginRenderPassIfNeeded();
  void bindGraphicsPipeline();
  VkShaderModule getOrCreateVertexShaderModule();
  VkShaderModule getOrCreatePixelShaderModule();
  void applyDynamicState();
};
