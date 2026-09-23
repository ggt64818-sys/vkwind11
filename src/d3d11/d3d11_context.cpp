#include "d3d11_context.h"
#include "d3d11_device.h"
#include "d3d11_view.h"
#include "d3d11_shader.h"
#include "d3d11_state.h"
#include "../common/logging.h"
#include <cstring>

// ============================================================================
// D3D11DeviceContext Implementation
// ============================================================================

D3D11DeviceContext::D3D11DeviceContext(D3D11Device* device)
  : m_device(device) {
  memset(m_vsConstantBuffers, 0, sizeof(m_vsConstantBuffers));
  memset(m_psConstantBuffers, 0, sizeof(m_psConstantBuffers));
  memset(m_vsShaderResources, 0, sizeof(m_vsShaderResources));
  memset(m_psShaderResources, 0, sizeof(m_psShaderResources));
  memset(m_vsSamplers, 0, sizeof(m_vsSamplers));
  memset(m_psSamplers, 0, sizeof(m_psSamplers));
  memset(m_renderTargets, 0, sizeof(m_renderTargets));
  m_pipelineManager.initialize(m_device->getVulkanDevice().getDevice());
}

D3D11DeviceContext::~D3D11DeviceContext() = default;

HRESULT D3D11DeviceContext::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D11DeviceContext || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D11DeviceContext*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

void D3D11DeviceContext::GetDevice(ID3D11Device** ppDevice) {
  if (ppDevice) {
    *ppDevice = static_cast<ID3D11Device*>(m_device);
    if (m_device) m_device->AddRef();
  }
}

// ============================================================================
// VS
// ============================================================================

void D3D11DeviceContext::VSSetShader(ID3D11VertexShader* pVertexShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  if (m_vs != pVertexShader) {
    // Shader changed — invalidate cached module
    if (m_vsModule != VK_NULL_HANDLE) {
      vkDestroyShaderModule(m_device->getVulkanDevice().getDevice(), m_vsModule, nullptr);
      m_vsModule = VK_NULL_HANDLE;
    }
    m_currentPipeline = VK_NULL_HANDLE;
  }
  m_vs = pVertexShader;
  VKWIND11_LOG_TRACE("VSSetShader: %p", pVertexShader);
}

void D3D11DeviceContext::VSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers) {
  for (UINT i = 0; i < NumBuffers; i++) {
    if (StartSlot + i < 14) {
      m_vsConstantBuffers[StartSlot + i].buffer = ppConstantBuffers[i];
    }
  }
}

void D3D11DeviceContext::VSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) {
  for (UINT i = 0; i < NumViews; i++) {
    if (StartSlot + i < 128) {
      m_vsShaderResources[StartSlot + i] = ppShaderResourceViews[i];
    }
  }
}

void D3D11DeviceContext::VSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers) {
  for (UINT i = 0; i < NumSamplers; i++) {
    if (StartSlot + i < 16) {
      m_vsSamplers[StartSlot + i] = ppSamplers[i];
    }
  }
}

void D3D11DeviceContext::VSGetShader(ID3D11VertexShader** ppVertexShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances) {
  if (ppVertexShader) *ppVertexShader = m_vs;
  if (ppClassInstances) *ppClassInstances = nullptr;
  if (pNumClassInstances) *pNumClassInstances = 0;
}

// ============================================================================
// PS
// ============================================================================

void D3D11DeviceContext::PSSetShader(ID3D11PixelShader* pPixelShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  if (m_ps != pPixelShader) {
    // Shader changed — invalidate cached module
    if (m_psModule != VK_NULL_HANDLE) {
      vkDestroyShaderModule(m_device->getVulkanDevice().getDevice(), m_psModule, nullptr);
      m_psModule = VK_NULL_HANDLE;
    }
    m_currentPipeline = VK_NULL_HANDLE;
  }
  m_ps = pPixelShader;
  VKWIND11_LOG_TRACE("PSSetShader: %p", pPixelShader);
}

void D3D11DeviceContext::PSSetConstantBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppConstantBuffers) {
  for (UINT i = 0; i < NumBuffers; i++) {
    if (StartSlot + i < 14) {
      m_psConstantBuffers[StartSlot + i].buffer = ppConstantBuffers[i];
    }
  }
}

void D3D11DeviceContext::PSSetShaderResources(UINT StartSlot, UINT NumViews, ID3D11ShaderResourceView* const* ppShaderResourceViews) {
  for (UINT i = 0; i < NumViews; i++) {
    if (StartSlot + i < 128) {
      m_psShaderResources[StartSlot + i] = ppShaderResourceViews[i];
    }
  }
}

void D3D11DeviceContext::PSSetSamplers(UINT StartSlot, UINT NumSamplers, ID3D11SamplerState* const* ppSamplers) {
  for (UINT i = 0; i < NumSamplers; i++) {
    if (StartSlot + i < 16) {
      m_psSamplers[StartSlot + i] = ppSamplers[i];
    }
  }
}

void D3D11DeviceContext::PSGetShader(ID3D11PixelShader** ppPixelShader, ID3D11ClassInstance** ppClassInstances, UINT* pNumClassInstances) {
  if (ppPixelShader) *ppPixelShader = m_ps;
  if (ppClassInstances) *ppClassInstances = nullptr;
  if (pNumClassInstances) *pNumClassInstances = 0;
}

// ============================================================================
// GS / HS / DS / CS
// ============================================================================

void D3D11DeviceContext::GSSetShader(ID3D11GeometryShader* pShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  m_gs = pShader;
}

void D3D11DeviceContext::HSSetShader(ID3D11HullShader* pHullShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  m_hs = pHullShader;
}

void D3D11DeviceContext::DSSetShader(ID3D11DomainShader* pDomainShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  m_ds = pDomainShader;
}

void D3D11DeviceContext::CSSetShader(ID3D11ComputeShader* pComputeShader, ID3D11ClassInstance* const* ppClassInstances, UINT NumClassInstances) {
  m_cs = pComputeShader;
}

// ============================================================================
// IA
// ============================================================================

void D3D11DeviceContext::IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY Topology) {
  if (m_topology != Topology) m_currentPipeline = VK_NULL_HANDLE;
  m_topology = Topology;
}

void D3D11DeviceContext::IASetInputLayout(ID3D11InputLayout* pInputLayout) {
  if (m_inputLayout != pInputLayout) m_currentPipeline = VK_NULL_HANDLE;
  m_inputLayout = pInputLayout;
}

void D3D11DeviceContext::IASetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer* const* ppVertexBuffers, const UINT* pStrides, const UINT* pOffsets) {
  for (UINT i = 0; i < NumBuffers; i++) {
    if (StartSlot + i < 32) {
      auto& slot = m_vertexBuffers[StartSlot + i];
      if (slot.buffer != ppVertexBuffers[i] ||
          (pStrides && slot.stride != pStrides[i])) {
        m_currentPipeline = VK_NULL_HANDLE;
      }
      slot.buffer = ppVertexBuffers[i];
      slot.stride = pStrides ? pStrides[i] : 0;
      slot.offset = pOffsets ? pOffsets[i] : 0;
    }
  }
}

void D3D11DeviceContext::IASetIndexBuffer(ID3D11Buffer* pIndexBuffer, DXGI_FORMAT Format, UINT Offset) {
  m_indexBuffer = pIndexBuffer;
  m_indexFormat = Format;
  m_indexOffset = Offset;
}

void D3D11DeviceContext::IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY* pTopology) {
  if (pTopology) *pTopology = m_topology;
}

void D3D11DeviceContext::IAGetInputLayout(ID3D11InputLayout** ppInputLayout) {
  if (ppInputLayout) *ppInputLayout = m_inputLayout;
}

void D3D11DeviceContext::IAGetVertexBuffers(UINT StartSlot, UINT NumBuffers, ID3D11Buffer** ppVertexBuffers, UINT* pStrides, UINT* pOffsets) {
  for (UINT i = 0; i < NumBuffers; i++) {
    if (StartSlot + i < 32) {
      if (ppVertexBuffers) ppVertexBuffers[i] = m_vertexBuffers[StartSlot + i].buffer;
      if (pStrides) pStrides[i] = m_vertexBuffers[StartSlot + i].stride;
      if (pOffsets) pOffsets[i] = m_vertexBuffers[StartSlot + i].offset;
    }
  }
}

void D3D11DeviceContext::IAGetIndexBuffer(ID3D11Buffer** pIndexBuffer, DXGI_FORMAT* Format, UINT* Offset) {
  if (pIndexBuffer) *pIndexBuffer = m_indexBuffer;
  if (Format) *Format = m_indexFormat;
  if (Offset) *Offset = m_indexOffset;
}

// ============================================================================
// Draw
// ============================================================================

void D3D11DeviceContext::Draw(UINT VertexCount, UINT StartVertexLocation) {
  VKWIND11_LOG_TRACE("Draw: vertices=%u, start=%u", VertexCount, StartVertexLocation);
  ensureCommandBuffer();
  beginRenderPassIfNeeded();
  if (!m_inRenderPass) return;
  bindGraphicsPipeline();
  if (m_currentPipeline == VK_NULL_HANDLE) return;

  // Bind vertex buffers
  for (UINT i = 0; i < 32; i++) {
    if (m_vertexBuffers[i].buffer) {
      auto* buf = static_cast<D3D11BufferImpl*>(m_vertexBuffers[i].buffer);
      VkBuffer vkBuf = buf->vkBuffer.buffer;
      VkDeviceSize offset = m_vertexBuffers[i].offset;
      vkCmdBindVertexBuffers(m_cmdBuffer, i, 1, &vkBuf, &offset);
    }
  }

  vkCmdDraw(m_cmdBuffer, VertexCount, 1, StartVertexLocation, 0);
  VKWIND11_LOG_TRACE("Draw: vkCmdDraw issued");
}

void D3D11DeviceContext::DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) {
  VKWIND11_LOG_TRACE("DrawIndexed: indices=%u, start=%u, base=%d", IndexCount, StartIndexLocation, BaseVertexLocation);
  ensureCommandBuffer();
  beginRenderPassIfNeeded();
  if (!m_inRenderPass) return;
  bindGraphicsPipeline();
  if (m_currentPipeline == VK_NULL_HANDLE) return;

  // Bind vertex buffers
  for (UINT i = 0; i < 32; i++) {
    if (m_vertexBuffers[i].buffer) {
      auto* buf = static_cast<D3D11BufferImpl*>(m_vertexBuffers[i].buffer);
      VkBuffer vkBuf = buf->vkBuffer.buffer;
      VkDeviceSize offset = m_vertexBuffers[i].offset;
      vkCmdBindVertexBuffers(m_cmdBuffer, i, 1, &vkBuf, &offset);
    }
  }

  // Bind index buffer
  if (m_indexBuffer) {
    auto* idxBuf = static_cast<D3D11BufferImpl*>(m_indexBuffer);
    VkIndexType idxType = (m_indexFormat == DXGI_FORMAT_R16_UINT)
                          ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    vkCmdBindIndexBuffer(m_cmdBuffer, idxBuf->vkBuffer.buffer, m_indexOffset, idxType);
  }

  vkCmdDrawIndexed(m_cmdBuffer, IndexCount, 1, StartIndexLocation, BaseVertexLocation, 0);
  VKWIND11_LOG_TRACE("DrawIndexed: vkCmdDrawIndexed issued");
}

void D3D11DeviceContext::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {
  VKWIND11_LOG_TRACE("DrawInstanced: verts=%u, instances=%u", VertexCountPerInstance, InstanceCount);
  ensureCommandBuffer();
  beginRenderPassIfNeeded();
  if (!m_inRenderPass) return;
  bindGraphicsPipeline();
  if (m_currentPipeline == VK_NULL_HANDLE) return;

  // Bind vertex buffers
  for (UINT i = 0; i < 32; i++) {
    if (m_vertexBuffers[i].buffer) {
      auto* buf = static_cast<D3D11BufferImpl*>(m_vertexBuffers[i].buffer);
      VkBuffer vkBuf = buf->vkBuffer.buffer;
      VkDeviceSize offset = m_vertexBuffers[i].offset;
      vkCmdBindVertexBuffers(m_cmdBuffer, i, 1, &vkBuf, &offset);
    }
  }

  vkCmdDraw(m_cmdBuffer, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
  VKWIND11_LOG_TRACE("DrawInstanced: vkCmdDraw issued");
}

void D3D11DeviceContext::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
  VKWIND11_LOG_TRACE("DrawIndexedInstanced: indices=%u, instances=%u", IndexCountPerInstance, InstanceCount);
  ensureCommandBuffer();
  beginRenderPassIfNeeded();
  if (!m_inRenderPass) return;
  bindGraphicsPipeline();
  if (m_currentPipeline == VK_NULL_HANDLE) return;

  // Bind vertex buffers
  for (UINT i = 0; i < 32; i++) {
    if (m_vertexBuffers[i].buffer) {
      auto* buf = static_cast<D3D11BufferImpl*>(m_vertexBuffers[i].buffer);
      VkBuffer vkBuf = buf->vkBuffer.buffer;
      VkDeviceSize offset = m_vertexBuffers[i].offset;
      vkCmdBindVertexBuffers(m_cmdBuffer, i, 1, &vkBuf, &offset);
    }
  }

  // Bind index buffer
  if (m_indexBuffer) {
    auto* idxBuf = static_cast<D3D11BufferImpl*>(m_indexBuffer);
    VkIndexType idxType = (m_indexFormat == DXGI_FORMAT_R16_UINT)
                          ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    vkCmdBindIndexBuffer(m_cmdBuffer, idxBuf->vkBuffer.buffer, m_indexOffset, idxType);
  }

  vkCmdDrawIndexed(m_cmdBuffer, IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
  VKWIND11_LOG_TRACE("DrawIndexedInstanced: vkCmdDrawIndexed issued");
}

void D3D11DeviceContext::DrawAuto() {
  VKWIND11_LOG_TRACE("DrawAuto");
}

// ============================================================================
// OM
// ============================================================================

void D3D11DeviceContext::OMSetRenderTargets(UINT NumViews, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView) {
  // End current render pass if active
  if (m_inRenderPass && m_cmdBuffer != VK_NULL_HANDLE) {
    vkCmdEndRenderPass(m_cmdBuffer);
    m_inRenderPass = false;
  }

  for (UINT i = 0; i < 8; i++) m_renderTargets[i] = nullptr;
  for (UINT i = 0; i < NumViews && i < 8; i++) {
    m_renderTargets[i] = ppRenderTargetViews[i];
  }
  m_depthStencil = pDepthStencilView;

  // Destroy old render pass and framebuffer
  auto device = m_device->getVulkanDevice().getDevice();
  if (m_currentFramebuffer != VK_NULL_HANDLE) {
    vkDestroyFramebuffer(device, m_currentFramebuffer, nullptr);
    m_currentFramebuffer = VK_NULL_HANDLE;
  }
  if (m_currentRenderPass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(device, m_currentRenderPass, nullptr);
    m_currentRenderPass = VK_NULL_HANDLE;
  }

  // Create new render pass + framebuffer from first RTV
  if (NumViews > 0 && ppRenderTargetViews[0]) {
    ID3D11Resource* resource = nullptr;
    ppRenderTargetViews[0]->GetResource(&resource);
    if (resource) {
      D3D11_RESOURCE_DIMENSION dim;
      resource->GetType(&dim);
      if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
        auto* tex = static_cast<D3D11Texture2DImpl*>(resource);
        VkFormat colorFormat = tex->vkImage.format;

        // Create render pass
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = colorFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rpInfo.attachmentCount = 1;
        rpInfo.pAttachments = &colorAttachment;
        rpInfo.subpassCount = 1;
        rpInfo.pSubpasses = &subpass;
        rpInfo.dependencyCount = 1;
        rpInfo.pDependencies = &dependency;

        m_currentRenderPass = m_device->getVulkanDevice().createRenderPass(rpInfo);

        // Create framebuffer
        std::vector<VkImageView> attachments = {tex->vkImage.view};
        m_currentFramebuffer = m_device->getVulkanDevice().createFramebuffer(
          m_currentRenderPass, attachments, tex->desc.Width, tex->desc.Height);

        VKWIND11_LOG_TRACE("OMSetRenderTargets: created render pass %p framebuffer %p (%ux%u)",
                           m_currentRenderPass, m_currentFramebuffer, tex->desc.Width, tex->desc.Height);
      }
      resource->Release();
    }
  }
}

void D3D11DeviceContext::OMSetRenderTargetsAndUnorderedAccessViews(UINT NumRTVs, ID3D11RenderTargetView* const* ppRenderTargetViews, ID3D11DepthStencilView* pDepthStencilView, UINT UAVStartSlot, UINT NumUAVs, ID3D11UnorderedAccessView* const* ppUnorderedAccessViews, const UINT* pUAVInitialCounts) {
  OMSetRenderTargets(NumRTVs, ppRenderTargetViews, pDepthStencilView);
  // TODO: UAV binding
}

void D3D11DeviceContext::OMSetBlendState(ID3D11BlendState* pBlendState, const float BlendFactor[4], UINT SampleMask) {
  if (m_blendState != pBlendState) m_currentPipeline = VK_NULL_HANDLE;
  m_blendState = pBlendState;
  if (BlendFactor) memcpy(m_blendFactor, BlendFactor, sizeof(float) * 4);
  m_sampleMask = SampleMask;
}

void D3D11DeviceContext::OMSetDepthStencilState(ID3D11DepthStencilState* pDepthStencilState, UINT StencilRef) {
  if (m_depthStencilState != pDepthStencilState) m_currentPipeline = VK_NULL_HANDLE;
  m_depthStencilState = pDepthStencilState;
  m_stencilRef = StencilRef;
}

void D3D11DeviceContext::OMGetBlendState(ID3D11BlendState** ppBlendState, float* pBlendFactor, UINT* pSampleMask) {
  if (ppBlendState) *ppBlendState = m_blendState;
  if (pBlendFactor) memcpy(pBlendFactor, m_blendFactor, sizeof(float) * 4);
  if (pSampleMask) *pSampleMask = m_sampleMask;
}

void D3D11DeviceContext::OMGetDepthStencilState(ID3D11DepthStencilState** ppDepthStencilState, UINT* pStencilRef) {
  if (ppDepthStencilState) *ppDepthStencilState = m_depthStencilState;
  if (pStencilRef) *pStencilRef = m_stencilRef;
}

// ============================================================================
// RS
// ============================================================================

void D3D11DeviceContext::RSSetState(ID3D11RasterizerState* pRasterizerState) {
  if (m_rasterizerState != pRasterizerState) m_currentPipeline = VK_NULL_HANDLE;
  m_rasterizerState = pRasterizerState;
}

void D3D11DeviceContext::RSSetViewports(UINT NumViewports, const D3D11_VIEWPORT* pViewports) {
  m_numViewports = NumViewports;
  for (UINT i = 0; i < NumViewports && i < 16; i++) {
    m_viewports[i] = pViewports[i];
  }
  if (m_cmdBuffer != VK_NULL_HANDLE && m_inRenderPass && NumViewports > 0) {
    std::vector<VkViewport> vkViewports(NumViewports);
    for (UINT i = 0; i < NumViewports; i++) {
      vkViewports[i].x = pViewports[i].TopLeftX;
      vkViewports[i].y = pViewports[i].TopLeftY + pViewports[i].Height;
      vkViewports[i].width = pViewports[i].Width;
      vkViewports[i].height = -pViewports[i].Height;
      vkViewports[i].minDepth = 0.0f;
      vkViewports[i].maxDepth = 1.0f;
    }
    vkCmdSetViewport(m_cmdBuffer, 0, NumViewports, vkViewports.data());
  }
}

void D3D11DeviceContext::RSSetScissorRects(UINT NumRects, const D3D11_RECT* pRects) {
  m_numScissorRects = NumRects;
  for (UINT i = 0; i < NumRects && i < 16; i++) {
    m_scissorRects[i] = pRects[i];
  }
  if (m_cmdBuffer != VK_NULL_HANDLE && m_inRenderPass && NumRects > 0) {
    std::vector<VkRect2D> vkScissors(NumRects);
    for (UINT i = 0; i < NumRects; i++) {
      vkScissors[i].offset = {pRects[i].left, pRects[i].top};
      vkScissors[i].extent = {
        (uint32_t)(pRects[i].right - pRects[i].left),
        (uint32_t)(pRects[i].bottom - pRects[i].top)
      };
    }
    vkCmdSetScissor(m_cmdBuffer, 0, NumRects, vkScissors.data());
  }
}

void D3D11DeviceContext::RSGetState(ID3D11RasterizerState** ppRasterizerState) {
  if (ppRasterizerState) *ppRasterizerState = m_rasterizerState;
}

void D3D11DeviceContext::RSGetViewports(UINT* pNumViewports, D3D11_VIEWPORT* pViewports) {
  if (pNumViewports) *pNumViewports = m_numViewports;
  if (pViewports) memcpy(pViewports, m_viewports, sizeof(D3D11_VIEWPORT) * m_numViewports);
}

void D3D11DeviceContext::RSGetScissorRects(UINT* pNumRects, D3D11_RECT* pRects) {
  if (pNumRects) *pNumRects = m_numScissorRects;
  if (pRects) memcpy(pRects, m_scissorRects, sizeof(D3D11_RECT) * m_numScissorRects);
}

// ============================================================================
// Clear
// ============================================================================

void D3D11DeviceContext::ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView, const float ColorRGBA[4]) {
  VKWIND11_LOG_TRACE("ClearRenderTargetView");
  if (!pRenderTargetView || !ColorRGBA) return;
  if (m_cmdBuffer == VK_NULL_HANDLE) return;
  VkClearColorValue clearColor = {};
  clearColor.float32[0] = ColorRGBA[0];
  clearColor.float32[1] = ColorRGBA[1];
  clearColor.float32[2] = ColorRGBA[2];
  clearColor.float32[3] = ColorRGBA[3];
  VkImageSubresourceRange range = {};
  range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  range.baseMipLevel = 0;
  range.levelCount = 1;
  range.baseArrayLayer = 0;
  range.layerCount = 1;
  // TODO: resolve the RTV to VkImage, vkCmdClearColorImage or use render pass load-op clear
  // For now this is a placeholder that validates inputs are captured correctly.
}

void D3D11DeviceContext::ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView, UINT ClearFlags, float Depth, UINT8 Stencil) {
  VKWIND11_LOG_TRACE("ClearDepthStencilView");
}

void D3D11DeviceContext::ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView* pUnorderedAccessView, const UINT Values[4]) {}
void D3D11DeviceContext::ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView* pUnorderedAccessView, const float Values[4]) {}

void D3D11DeviceContext::ClearState() {
  m_vs = nullptr;
  m_ps = nullptr;
  m_gs = nullptr;
  m_hs = nullptr;
  m_ds = nullptr;
  m_cs = nullptr;
  m_inputLayout = nullptr;
  m_blendState = nullptr;
  m_depthStencilState = nullptr;
  m_rasterizerState = nullptr;
  m_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
  memset(m_vertexBuffers, 0, sizeof(m_vertexBuffers));
  m_indexBuffer = nullptr;
  memset(m_vsConstantBuffers, 0, sizeof(m_vsConstantBuffers));
  memset(m_psConstantBuffers, 0, sizeof(m_psConstantBuffers));
  memset(m_renderTargets, 0, sizeof(m_renderTargets));
  m_depthStencil = nullptr;
  m_numViewports = 0;
  m_numScissorRects = 0;
  m_currentPipeline = VK_NULL_HANDLE;
  m_currentPipelineLayout = VK_NULL_HANDLE;
  m_boundVS = nullptr;
  m_boundPS = nullptr;
}

// ============================================================================
// Helpers
// ============================================================================

static VkFormat dxgiToVkFormat(DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:    return VK_FORMAT_R32G32B32A32_SFLOAT;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:    return VK_FORMAT_R16G16B16A16_SFLOAT;
    case DXGI_FORMAT_R8G8B8A8_UNORM:        return VK_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return VK_FORMAT_R8G8B8A8_SRGB;
    case DXGI_FORMAT_R8G8B8A8_UINT:         return VK_FORMAT_R8G8B8A8_UINT;
    case DXGI_FORMAT_R8G8B8A8_SNORM:        return VK_FORMAT_R8G8B8A8_SNORM;
    case DXGI_FORMAT_R8G8B8A8_SINT:         return VK_FORMAT_R8G8B8A8_SINT;
    case DXGI_FORMAT_B8G8R8A8_UNORM:        return VK_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
    case DXGI_FORMAT_R32_FLOAT:             return VK_FORMAT_R32_SFLOAT;
    case DXGI_FORMAT_R16_FLOAT:             return VK_FORMAT_R16_SFLOAT;
    case DXGI_FORMAT_R16_UNORM:             return VK_FORMAT_R16_UNORM;
    case DXGI_FORMAT_R8_UNORM:              return VK_FORMAT_R8_UNORM;
    case DXGI_FORMAT_D32_FLOAT:             return VK_FORMAT_D32_SFLOAT;
    case DXGI_FORMAT_D16_UNORM:             return VK_FORMAT_D16_UNORM;
    default:                                return VK_FORMAT_R8G8B8A8_UNORM;
  }
}

static VkPrimitiveTopology d3d11TopologyToVk(D3D11_PRIMITIVE_TOPOLOGY topology) {
  switch (topology) {
    case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    default:                                      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  }
}

// ============================================================================
// D3D11DeviceContext — Draw helper implementations
// ============================================================================

void D3D11DeviceContext::ensureCommandBuffer() {
  if (m_cmdBuffer != VK_NULL_HANDLE) return;
  auto& vk = m_device->getVulkanDevice();
  m_cmdPool = vk.createCommandPool(vk.getGraphicsQueueFamily());
  m_cmdBuffer = vk.allocateCommandBuffer(m_cmdPool);
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(m_cmdBuffer, &beginInfo);
  VKWIND11_LOG_TRACE("ensureCommandBuffer: created new command buffer %p", m_cmdBuffer);
}

void D3D11DeviceContext::beginRenderPassIfNeeded() {
  if (m_inRenderPass) return;
  if (m_currentRenderPass == VK_NULL_HANDLE || m_currentFramebuffer == VK_NULL_HANDLE) return;

  // Determine render area from the first RTV's texture dimensions
  VkRect2D renderArea{};
  renderArea.offset = {0, 0};
  renderArea.extent = {800, 600}; // default; overridden below if RTV available

  if (m_renderTargets[0]) {
    ID3D11Resource* resource = nullptr;
    m_renderTargets[0]->GetResource(&resource);
    if (resource) {
      D3D11_RESOURCE_DIMENSION dim;
      resource->GetType(&dim);
      if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
        auto* tex = static_cast<D3D11Texture2DImpl*>(resource);
        renderArea.extent = {tex->desc.Width, tex->desc.Height};
      }
      resource->Release();
    }
  }

  VkClearValue clearColor{};
  clearColor.color = {{0.0f, 0.0f, 0.0f, 1.0f}};

  VkRenderPassBeginInfo rpBegin{};
  rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rpBegin.renderPass = m_currentRenderPass;
  rpBegin.framebuffer = m_currentFramebuffer;
  rpBegin.renderArea = renderArea;
  rpBegin.clearValueCount = 1;
  rpBegin.pClearValues = &clearColor;

  vkCmdBeginRenderPass(m_cmdBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
  m_inRenderPass = true;

  applyDynamicState();

  VKWIND11_LOG_TRACE("beginRenderPassIfNeeded: began render pass %p framebuffer %p",
                     m_currentRenderPass, m_currentFramebuffer);
}

void D3D11DeviceContext::applyDynamicState() {
  if (m_cmdBuffer == VK_NULL_HANDLE) return;

  if (m_numViewports > 0) {
    std::vector<VkViewport> vkViewports(m_numViewports);
    for (UINT i = 0; i < m_numViewports; i++) {
      vkViewports[i].x = m_viewports[i].TopLeftX;
      vkViewports[i].y = m_viewports[i].TopLeftY + m_viewports[i].Height;
      vkViewports[i].width = m_viewports[i].Width;
      vkViewports[i].height = -m_viewports[i].Height;
      vkViewports[i].minDepth = 0.0f;
      vkViewports[i].maxDepth = 1.0f;
    }
    vkCmdSetViewport(m_cmdBuffer, 0, m_numViewports, vkViewports.data());
  }

  if (m_numScissorRects > 0) {
    std::vector<VkRect2D> vkScissors(m_numScissorRects);
    for (UINT i = 0; i < m_numScissorRects; i++) {
      vkScissors[i].offset = {m_scissorRects[i].left, m_scissorRects[i].top};
      vkScissors[i].extent = {
        (uint32_t)(m_scissorRects[i].right - m_scissorRects[i].left),
        (uint32_t)(m_scissorRects[i].bottom - m_scissorRects[i].top)
      };
    }
    vkCmdSetScissor(m_cmdBuffer, 0, m_numScissorRects, vkScissors.data());
  }
}

VkShaderModule D3D11DeviceContext::getOrCreateVertexShaderModule() {
  if (m_vsModule != VK_NULL_HANDLE) return m_vsModule;
  if (!m_vs) return VK_NULL_HANDLE;

  auto* vs = static_cast<D3D11VertexShader*>(m_vs);
  SM4Translator translator;
  SM4TranslateResult result = translator.translate(vs->getDXBC());

  if (!result.success) {
    VKWIND11_LOG_WARN("VS translation failed: %s", result.errorMessage.c_str());
    return VK_NULL_HANDLE;
  }

  auto& vk = m_device->getVulkanDevice();
  m_vsModule = vk.createShaderModule(result.spirvWords.data(),
                                     result.spirvWords.size() * sizeof(uint32_t));
  m_boundVS = m_vs;

  VKWIND11_LOG_TRACE("getOrCreateVertexShaderModule: created VkShaderModule %p", m_vsModule);
  return m_vsModule;
}

VkShaderModule D3D11DeviceContext::getOrCreatePixelShaderModule() {
  if (m_psModule != VK_NULL_HANDLE) return m_psModule;
  if (!m_ps) return VK_NULL_HANDLE;

  auto* ps = static_cast<D3D11PixelShader*>(m_ps);
  SM4Translator translator;
  SM4TranslateResult result = translator.translate(ps->getDXBC());

  if (!result.success) {
    VKWIND11_LOG_WARN("PS translation failed: %s", result.errorMessage.c_str());
    return VK_NULL_HANDLE;
  }

  auto& vk = m_device->getVulkanDevice();
  m_psModule = vk.createShaderModule(result.spirvWords.data(),
                                     result.spirvWords.size() * sizeof(uint32_t));
  m_boundPS = m_ps;

  VKWIND11_LOG_TRACE("getOrCreatePixelShaderModule: created VkShaderModule %p", m_psModule);
  return m_psModule;
}

void D3D11DeviceContext::bindGraphicsPipeline() {
  VkShaderModule vsModule = getOrCreateVertexShaderModule();
  VkShaderModule psModule = getOrCreatePixelShaderModule();

  if (vsModule == VK_NULL_HANDLE || psModule == VK_NULL_HANDLE) {
    VKWIND11_LOG_WARN("bindGraphicsPipeline: missing shader modules (vs=%p, ps=%p)",
                      vsModule, psModule);
    return;
  }

  PipelineKey key{};
  key.vertexShader = vsModule;
  key.pixelShader = psModule;
  key.topology = d3d11TopologyToVk(m_topology);
  key.renderPass = m_currentRenderPass;

  // Rasterizer state
  if (m_rasterizerState) {
    auto& desc = static_cast<D3D11RasterizerState*>(m_rasterizerState)->desc;
    key.polygonMode = VK_POLYGON_MODE_FILL;
    key.cullMode = 0;
    if (desc.CullMode == D3D11_CULL_FRONT)        key.cullMode = VK_CULL_MODE_FRONT_BIT;
    else if (desc.CullMode == D3D11_CULL_BACK)     key.cullMode = VK_CULL_MODE_BACK_BIT;
    else if (desc.CullMode == D3D11_CULL_FRONT_AND_BACK) key.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
    key.frontFace = desc.FrontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    if (desc.FillMode == D3D11_FILL_WIREFRAME) key.polygonMode = VK_POLYGON_MODE_LINE;
    else if (desc.FillMode == D3D11_FILL_POINTS) key.polygonMode = VK_POLYGON_MODE_POINT;
  } else {
    key.polygonMode = VK_POLYGON_MODE_FILL;
    key.cullMode = VK_CULL_MODE_BACK_BIT;
    key.frontFace = VK_FRONT_FACE_CLOCKWISE;
  }

  // Depth/stencil state
  if (m_depthStencilState) {
    auto& desc = static_cast<D3D11DepthStencilState*>(m_depthStencilState)->desc;
    key.depthTestEnable = desc.DepthEnable != 0;
    key.depthWriteEnable = desc.DepthWriteMask != 0;
    switch (desc.DepthFunc) {
      case D3D11_COMPARISON_ALWAYS:       key.depthCompareOp = VK_COMPARE_OP_ALWAYS; break;
      case D3D11_COMPARISON_NEVER:        key.depthCompareOp = VK_COMPARE_OP_NEVER; break;
      case D3D11_COMPARISON_EQUAL:        key.depthCompareOp = VK_COMPARE_OP_EQUAL; break;
      case D3D11_COMPARISON_NOT_EQUAL:    key.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL; break;
      case D3D11_COMPARISON_LESS:         key.depthCompareOp = VK_COMPARE_OP_LESS; break;
      case D3D11_COMPARISON_LESS_EQUAL:   key.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; break;
      case D3D11_COMPARISON_GREATER:      key.depthCompareOp = VK_COMPARE_OP_GREATER; break;
      case D3D11_COMPARISON_GREATER_EQUAL:key.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL; break;
      default:                            key.depthCompareOp = VK_COMPARE_OP_LESS; break;
    }
  } else {
    key.depthTestEnable = false;
    key.depthWriteEnable = false;
    key.depthCompareOp = VK_COMPARE_OP_ALWAYS;
  }

  // Blend state
  if (m_blendState && static_cast<D3D11BlendState*>(m_blendState)->desc.RenderTarget[0].BlendEnable) {
    key.blendEnable = true;
  } else {
    key.blendEnable = false;
  }

  // Vertex input from input layout
  if (m_inputLayout) {
    auto* layout = static_cast<D3D11InputLayoutImpl*>(m_inputLayout);
    auto& elements = layout->getElements();

    uint32_t bindingSlot = 0;
    for (auto& elem : elements) {
      // Find matching vertex buffer for this semantic
      if (elem.InputSlot != bindingSlot) {
        bindingSlot = elem.InputSlot;
      }

      VkVertexInputBindingDescription binding{};
      binding.binding = elem.InputSlot;
      binding.stride = m_vertexBuffers[elem.InputSlot].stride;
      binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      // Avoid duplicate bindings
      bool found = false;
      for (auto& b : key.vertexBindings) {
        if (b.binding == binding.binding) { found = true; break; }
      }
      if (!found) key.vertexBindings.push_back(binding);

      VkVertexInputAttributeDescription attr{};
      attr.location = (uint32_t)elem.SemanticIndex;
      attr.binding = elem.InputSlot;
      attr.offset = elem.AlignedByteOffset;

      switch (elem.Format) {
        case DXGI_FORMAT_R32G32B32A32_FLOAT: attr.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
        case DXGI_FORMAT_R32G32B32_FLOAT:    attr.format = VK_FORMAT_R32G32B32_SFLOAT; break;
        case DXGI_FORMAT_R32_FLOAT:          attr.format = VK_FORMAT_R32_SFLOAT; break;
        case DXGI_FORMAT_R8G8B8A8_UNORM:     attr.format = VK_FORMAT_R8G8B8A8_UNORM; break;
        case DXGI_FORMAT_R16G16_FLOAT:       attr.format = VK_FORMAT_R16G16_SFLOAT; break;
        case DXGI_FORMAT_R16_FLOAT:          attr.format = VK_FORMAT_R16_SFLOAT; break;
        default:                             attr.format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
      }

      key.vertexAttributes.push_back(attr);
    }
  }

  VkPipeline pipeline = m_pipelineManager.getOrCreateGraphicsPipeline(key);
  if (pipeline == VK_NULL_HANDLE) {
    VKWIND11_LOG_WARN("bindGraphicsPipeline: failed to create/get pipeline");
    return;
  }

  vkCmdBindPipeline(m_cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

  m_currentPipeline = pipeline;

  VKWIND11_LOG_TRACE("bindGraphicsPipeline: bound pipeline %p", pipeline);
}

static uint32_t GetBytesPerPixel(DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
    case DXGI_FORMAT_R16G16B16A16_FLOAT: return 8;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
      return 4;
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_D16_UNORM:
      return 2;
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_SNORM:
      return 1;
    default: return 4;
  }
}

static void TransitionImageLayout(VkDevice device, VkCommandBuffer cmd, VkImage image,
                                   VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags srcStage, dstStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
    barrier.srcAccessMask = 0;
    srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else {
    barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    srcStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }

  if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    barrier.dstAccessMask = 0;
    dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  }

  vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

static void BeginOneShotCommands(VulkanDevice& vk, VkCommandPool& pool, VkCommandBuffer& cmd) {
  pool = vk.createCommandPool(vk.getGraphicsQueueFamily());
  cmd = vk.allocateCommandBuffer(pool);
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(cmd, &beginInfo);
}

static void EndAndSubmitCommands(VulkanDevice& vk, VkCommandPool pool, VkCommandBuffer cmd) {
  vkEndCommandBuffer(cmd);
  vk.submitImmediate(cmd);
  auto device = vk.getDevice();
  vkFreeCommandBuffers(device, pool, 1, &cmd);
  vkDestroyCommandPool(device, pool, nullptr);
}

// ============================================================================
// Map / Unmap / Update
// ============================================================================

HRESULT D3D11DeviceContext::Map(ID3D11Resource* pResource, UINT Subresource, D3D11_MAP MapType, UINT MapFlags, D3D11_MAPPED_SUBRESOURCE* pMappedResource) {
  if (!pResource || !pMappedResource) return E_INVALIDARG;

  D3D11_RESOURCE_DIMENSION dim;
  pResource->GetType(&dim);

  if (dim == D3D11_RESOURCE_DIMENSION_BUFFER) {
    auto buffer = static_cast<D3D11BufferImpl*>(pResource);

    if (buffer->vkBuffer.mapped) {
      pMappedResource->pData = buffer->vkBuffer.mapped;
      pMappedResource->RowPitch = buffer->desc.ByteWidth;
      pMappedResource->DepthPitch = buffer->desc.ByteWidth;
      return S_OK;
    }

    if (buffer->vkBuffer.isHostVisible()) {
      auto& vk = m_device->getVulkanDevice();
      VkResult result = vkMapMemory(vk.getDevice(), buffer->vkBuffer.memory, 0, buffer->vkBuffer.size, 0, &buffer->vkBuffer.mapped);
      if (result != VK_SUCCESS) {
        VKWIND11_LOG_WARN("Map: vkMapMemory failed for buffer (%d)", result);
        return E_FAIL;
      }
      pMappedResource->pData = buffer->vkBuffer.mapped;
      pMappedResource->RowPitch = buffer->desc.ByteWidth;
      pMappedResource->DepthPitch = buffer->desc.ByteWidth;
      return S_OK;
    }

    VKWIND11_LOG_WARN("Map: buffer is not HOST_VISIBLE — creating staging buffer");
    auto& vk = m_device->getVulkanDevice();
    if (!vk.createBuffer(buffer->desc.ByteWidth, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         buffer->mapStagingBuffer)) {
      VKWIND11_LOG_WARN("Map: failed to create staging buffer for non-visible buffer");
      return E_FAIL;
    }
    if (buffer->desc.Usage == D3D11_USAGE_DEFAULT && (MapType == D3D11_MAP_READ || MapType == D3D11_MAP_READ_WRITE)) {
      VkCommandPool pool;
      VkCommandBuffer cmd;
      BeginOneShotCommands(vk, pool, cmd);
      VkBufferCopy copyRegion{};
      copyRegion.size = buffer->desc.ByteWidth;
      vkCmdCopyBuffer(cmd, buffer->vkBuffer.buffer, buffer->mapStagingBuffer.buffer, 1, &copyRegion);
      EndAndSubmitCommands(vk, pool, cmd);
    }
    vkMapMemory(vk.getDevice(), buffer->mapStagingBuffer.memory, 0, buffer->desc.ByteWidth, 0, &buffer->mapStagingBuffer.mapped);
    pMappedResource->pData = buffer->mapStagingBuffer.mapped;
    pMappedResource->RowPitch = buffer->desc.ByteWidth;
    pMappedResource->DepthPitch = buffer->desc.ByteWidth;
    return S_OK;
  }

  if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    auto texture = static_cast<D3D11Texture2DImpl*>(pResource);

    if (texture->mapStagingBuffer.buffer != VK_NULL_HANDLE) {
      VKWIND11_LOG_WARN("Map: texture already mapped");
      return E_FAIL;
    }

    auto& vk = m_device->getVulkanDevice();
    auto device = vk.getDevice();

    uint32_t bpp = GetBytesPerPixel(texture->desc.Format);
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(texture->desc.Width) * texture->desc.Height * bpp;

    if (MapType == D3D11_MAP_READ) {
      if (!vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           texture->mapStagingBuffer)) {
        VKWIND11_LOG_WARN("Map: failed to create staging buffer for texture read");
        return E_FAIL;
      }

      VkCommandPool pool;
      VkCommandBuffer cmd;
      BeginOneShotCommands(vk, pool, cmd);
      TransitionImageLayout(device, cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

      VkBufferImageCopy copyRegion{};
      copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
      copyRegion.imageExtent = {texture->desc.Width, texture->desc.Height, 1};
      vkCmdCopyImageToBuffer(cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                              texture->mapStagingBuffer.buffer, 1, &copyRegion);

      TransitionImageLayout(device, cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      EndAndSubmitCommands(vk, pool, cmd);

      vkMapMemory(device, texture->mapStagingBuffer.memory, 0, bufferSize, 0, &texture->mapStagingBuffer.mapped);

      pMappedResource->pData = texture->mapStagingBuffer.mapped;
      pMappedResource->RowPitch = texture->desc.Width * bpp;
      pMappedResource->DepthPitch = pMappedResource->RowPitch * texture->desc.Height;
      return S_OK;
    }

    if (MapType == D3D11_MAP_WRITE_DISCARD || MapType == D3D11_MAP_WRITE) {
      if (!vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                           texture->mapStagingBuffer)) {
        VKWIND11_LOG_WARN("Map: failed to create staging buffer for texture write");
        return E_FAIL;
      }

      vkMapMemory(device, texture->mapStagingBuffer.memory, 0, bufferSize, 0, &texture->mapStagingBuffer.mapped);

      pMappedResource->pData = texture->mapStagingBuffer.mapped;
      pMappedResource->RowPitch = texture->desc.Width * bpp;
      pMappedResource->DepthPitch = pMappedResource->RowPitch * texture->desc.Height;
      return S_OK;
    }

    VKWIND11_LOG_WARN("Map: unsupported map type %d for texture — returning E_FAIL", MapType);
    return E_FAIL;
  }

  VKWIND11_LOG_WARN("Map: unsupported resource type %d", dim);
  return E_FAIL;
}

void D3D11DeviceContext::Unmap(ID3D11Resource* pResource, UINT Subresource) {
  if (!pResource) return;

  D3D11_RESOURCE_DIMENSION dim;
  pResource->GetType(&dim);

  if (dim == D3D11_RESOURCE_DIMENSION_BUFFER) {
    return;
  }

  if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    auto texture = static_cast<D3D11Texture2DImpl*>(pResource);

    if (texture->mapStagingBuffer.buffer == VK_NULL_HANDLE) return;

    auto& vk = m_device->getVulkanDevice();
    auto device = vk.getDevice();

    if (texture->mapStagingBuffer.usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
      VkCommandPool pool;
      VkCommandBuffer cmd;
      BeginOneShotCommands(vk, pool, cmd);
      TransitionImageLayout(device, cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

      VkBufferImageCopy copyRegion{};
      copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
      copyRegion.imageExtent = {texture->desc.Width, texture->desc.Height, 1};
      vkCmdCopyBufferToImage(cmd, texture->mapStagingBuffer.buffer, texture->vkImage.image,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

      TransitionImageLayout(device, cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
      EndAndSubmitCommands(vk, pool, cmd);
    }

    vkUnmapMemory(device, texture->mapStagingBuffer.memory);
    texture->mapStagingBuffer.destroy(device);
  }
}

void D3D11DeviceContext::UpdateSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, const D3D11_BOX* pDstBox, const void* pSrcData, UINT SrcRowPitch, UINT SrcDepthPitch) {
  if (!pDstResource || !pSrcData) return;

  D3D11_RESOURCE_DIMENSION dim;
  pDstResource->GetType(&dim);

  auto& vk = m_device->getVulkanDevice();
  auto device = vk.getDevice();

  if (dim == D3D11_RESOURCE_DIMENSION_BUFFER) {
    auto buffer = static_cast<D3D11BufferImpl*>(pDstResource);

    if (buffer->vkBuffer.isHostVisible()) {
      UINT copySize = SrcRowPitch ? SrcRowPitch : buffer->desc.ByteWidth;
      void* dst = buffer->vkBuffer.mapped;
      if (!dst) {
        vkMapMemory(device, buffer->vkBuffer.memory, 0, buffer->vkBuffer.size, 0, &dst);
      }
      if (dst) {
        if (pDstBox) {
          copySize = pDstBox->right - pDstBox->left;
          dst = static_cast<uint8_t*>(dst) + pDstBox->left;
        }
        memcpy(dst, pSrcData, copySize);
      }
    } else {
      UINT copySize = SrcRowPitch ? SrcRowPitch : buffer->desc.ByteWidth;
      if (pDstBox) copySize = pDstBox->right - pDstBox->left;

      VulkanBuffer staging;
      vk.createBuffer(copySize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);

      void* mapped = nullptr;
      vkMapMemory(device, staging.memory, 0, copySize, 0, &mapped);
      memcpy(mapped, pSrcData, copySize);
      vkUnmapMemory(device, staging.memory);

      VkCommandPool pool;
      VkCommandBuffer cmd;
      BeginOneShotCommands(vk, pool, cmd);

      VkBufferCopy copyRegion{};
      copyRegion.dstOffset = pDstBox ? pDstBox->left : 0;
      copyRegion.size = copySize;
      vkCmdCopyBuffer(cmd, staging.buffer, buffer->vkBuffer.buffer, 1, &copyRegion);

      EndAndSubmitCommands(vk, pool, cmd);
      staging.destroy(device);
    }
    return;
  }

  if (dim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    auto texture = static_cast<D3D11Texture2DImpl*>(pDstResource);
    uint32_t bpp = GetBytesPerPixel(texture->desc.Format);

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(texture->desc.Width) * texture->desc.Height * bpp;

    VulkanBuffer staging;
    vk.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging);

    void* mapped = nullptr;
    vkMapMemory(device, staging.memory, 0, bufferSize, 0, &mapped);

    uint32_t dstRowPitch = texture->desc.Width * bpp;
    uint32_t srcRowPitch = SrcRowPitch ? SrcRowPitch : dstRowPitch;

    if (srcRowPitch == dstRowPitch) {
      memcpy(mapped, pSrcData, bufferSize);
    } else {
      uint8_t* dst = static_cast<uint8_t*>(mapped);
      const uint8_t* src = static_cast<const uint8_t*>(pSrcData);
      for (uint32_t y = 0; y < texture->desc.Height; ++y) {
        memcpy(dst + y * dstRowPitch, src + y * srcRowPitch, dstRowPitch);
      }
    }
    vkUnmapMemory(device, staging.memory);

    VkCommandPool pool;
    VkCommandBuffer cmd;
    BeginOneShotCommands(vk, pool, cmd);
    TransitionImageLayout(device, cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copyRegion{};
    copyRegion.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};

    if (pDstBox) {
      copyRegion.imageOffset = {static_cast<int32_t>(pDstBox->left), static_cast<int32_t>(pDstBox->top), 0};
      copyRegion.imageExtent = {pDstBox->right - pDstBox->left, pDstBox->bottom - pDstBox->top, 1};
    } else {
      copyRegion.imageExtent = {texture->desc.Width, texture->desc.Height, 1};
    }

    vkCmdCopyBufferToImage(cmd, staging.buffer, texture->vkImage.image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    TransitionImageLayout(device, cmd, texture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    EndAndSubmitCommands(vk, pool, cmd);
    staging.destroy(device);
    return;
  }
}

void D3D11DeviceContext::CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource) {
  if (!pDstResource || !pSrcResource) return;

  D3D11_RESOURCE_DIMENSION srcDim, dstDim;
  pSrcResource->GetType(&srcDim);
  pDstResource->GetType(&dstDim);

  if (srcDim != dstDim) {
    VKWIND11_LOG_WARN("CopyResource: src and dst dimensions mismatch");
    return;
  }

  auto& vk = m_device->getVulkanDevice();
  auto device = vk.getDevice();

  if (srcDim == D3D11_RESOURCE_DIMENSION_BUFFER) {
    auto srcBuffer = static_cast<D3D11BufferImpl*>(pSrcResource);
    auto dstBuffer = static_cast<D3D11BufferImpl*>(pDstResource);

    VkCommandPool pool;
    VkCommandBuffer cmd;
    BeginOneShotCommands(vk, pool, cmd);

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer->desc.ByteWidth;
    vkCmdCopyBuffer(cmd, srcBuffer->vkBuffer.buffer, dstBuffer->vkBuffer.buffer, 1, &copyRegion);

    EndAndSubmitCommands(vk, pool, cmd);
    return;
  }

  if (srcDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    auto srcTexture = static_cast<D3D11Texture2DImpl*>(pSrcResource);
    auto dstTexture = static_cast<D3D11Texture2DImpl*>(pDstResource);

    VkCommandPool pool;
    VkCommandBuffer cmd;
    BeginOneShotCommands(vk, pool, cmd);

    TransitionImageLayout(device, cmd, srcTexture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout(device, cmd, dstTexture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copyRegion{};
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    copyRegion.extent = {srcTexture->desc.Width, srcTexture->desc.Height, 1};
    vkCmdCopyImage(cmd, srcTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    dstTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    TransitionImageLayout(device, cmd, srcTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    TransitionImageLayout(device, cmd, dstTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    EndAndSubmitCommands(vk, pool, cmd);
    return;
  }

  VKWIND11_LOG_WARN("CopyResource: unsupported resource type %d", srcDim);
}

void D3D11DeviceContext::CopySubresourceRegion(ID3D11Resource* pDstResource, UINT DstSubresource, UINT DstX, UINT DstY, UINT DstZ, ID3D11Resource* pSrcResource, UINT SrcSubresource, const D3D11_BOX* pSrcBox) {
  if (!pDstResource || !pSrcResource) return;

  D3D11_RESOURCE_DIMENSION srcDim, dstDim;
  pSrcResource->GetType(&srcDim);
  pDstResource->GetType(&dstDim);

  if (srcDim != dstDim) {
    VKWIND11_LOG_WARN("CopySubresourceRegion: src and dst dimensions mismatch");
    return;
  }

  auto& vk = m_device->getVulkanDevice();
  auto device = vk.getDevice();

  if (srcDim == D3D11_RESOURCE_DIMENSION_BUFFER) {
    auto srcBuffer = static_cast<D3D11BufferImpl*>(pSrcResource);
    auto dstBuffer = static_cast<D3D11BufferImpl*>(pDstResource);

    VkCommandPool pool;
    VkCommandBuffer cmd;
    BeginOneShotCommands(vk, pool, cmd);

    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = DstX;

    if (pSrcBox) {
      copyRegion.srcOffset = pSrcBox->left;
      copyRegion.size = pSrcBox->right - pSrcBox->left;
    } else {
      copyRegion.srcOffset = 0;
      copyRegion.size = srcBuffer->desc.ByteWidth;
    }

    vkCmdCopyBuffer(cmd, srcBuffer->vkBuffer.buffer, dstBuffer->vkBuffer.buffer, 1, &copyRegion);

    EndAndSubmitCommands(vk, pool, cmd);
    return;
  }

  if (srcDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
    auto srcTexture = static_cast<D3D11Texture2DImpl*>(pSrcResource);
    auto dstTexture = static_cast<D3D11Texture2DImpl*>(pDstResource);

    VkCommandPool pool;
    VkCommandBuffer cmd;
    BeginOneShotCommands(vk, pool, cmd);

    TransitionImageLayout(device, cmd, srcTexture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    TransitionImageLayout(device, cmd, dstTexture->vkImage.image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkImageCopy copyRegion{};
    copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, SrcSubresource, 0, 1};
    copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, DstSubresource, 0, 1};
    copyRegion.dstOffset = {static_cast<int32_t>(DstX), static_cast<int32_t>(DstY), static_cast<int32_t>(DstZ)};

    if (pSrcBox) {
      copyRegion.srcOffset = {static_cast<int32_t>(pSrcBox->left), static_cast<int32_t>(pSrcBox->top), static_cast<int32_t>(pSrcBox->front)};
      copyRegion.extent = {pSrcBox->right - pSrcBox->left, pSrcBox->bottom - pSrcBox->top, pSrcBox->back - pSrcBox->front};
    } else {
      copyRegion.srcOffset = {0, 0, 0};
      copyRegion.extent = {srcTexture->desc.Width, srcTexture->desc.Height, 1};
    }

    vkCmdCopyImage(cmd, srcTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    dstTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    TransitionImageLayout(device, cmd, srcTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    TransitionImageLayout(device, cmd, dstTexture->vkImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    EndAndSubmitCommands(vk, pool, cmd);
    return;
  }

  VKWIND11_LOG_WARN("CopySubresourceRegion: unsupported resource type %d", srcDim);
}

// ============================================================================
// Query
// ============================================================================

void D3D11DeviceContext::Begin(ID3D11Asynchronous* pAsync) {}
HRESULT D3D11DeviceContext::End(ID3D11Asynchronous* pAsync) { return S_OK; }
HRESULT D3D11DeviceContext::GetData(ID3D11Asynchronous* pAsync, void* pData, UINT DataSize, UINT GetDataFlags) { return S_OK; }
void D3D11DeviceContext::SetPredication(ID3D11Predicate* pPredicate, BOOL PredicateValue) {}

// ============================================================================
// SO
// ============================================================================

void D3D11DeviceContext::SOSetTargets(UINT NumBuffers, ID3D11Buffer* const* ppSOTargets, const UINT* pOffsets) {}

// ============================================================================
// Resolve
// ============================================================================

void D3D11DeviceContext::ResolveSubresource(ID3D11Resource* pDstResource, UINT DstSubresource, ID3D11Resource* pSrcResource, UINT SrcSubresource, DXGI_FORMAT Format) {
  VKWIND11_LOG_WARN("ResolveSubresource stub");
}

void D3D11DeviceContext::ExecuteCommandList(ID3D11CommandList* pCommandList, BOOL RestoreContextState) {}
