#include "d3d12_device.h"
#include "../common/logging.h"
#include "../common/dxgi_utils.h"

// ============================================================================
// D3D12 Root Signature
// ============================================================================

D3D12RootSignatureImpl::D3D12RootSignatureImpl(D3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC* desc)
  : m_device(device), m_desc(*desc) {
  auto& vk = device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  std::vector<VkDescriptorSetLayoutBinding> bindings;
  uint32_t pushConstantSize = 0;

  for (uint32_t i = 0; i < desc->NumParameters; i++) {
    const auto& param = desc->pParameters[i];

    switch (param.ParameterType) {
      case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = i;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

        const auto& table = param.DescriptorTable;
        if (table.NumDescriptorRanges > 0) {
          switch (table.pDescriptorRanges[0].RangeType) {
            case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
            case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
            case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
              binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
              break;
            case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
              binding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
              break;
          }
        }
        bindings.push_back(binding);
        break;
      }
      case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS: {
        pushConstantSize += param.Constants.Num32BitValues * 4;
        break;
      }
      case D3D12_ROOT_PARAMETER_TYPE_CBV:
      case D3D12_ROOT_PARAMETER_TYPE_SRV:
      case D3D12_ROOT_PARAMETER_TYPE_UAV: {
        VkDescriptorSetLayoutBinding binding = {};
        binding.binding = i;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindings.push_back(binding);
        break;
      }
    }
  }

  m_bindings = bindings;
  m_pushConstantSize = pushConstantSize;

  // Add static samplers
  std::vector<VkSampler> staticSamplers;
  std::vector<VkDescriptorSetLayoutBinding> samplerBindings;

  for (uint32_t i = 0; i < desc->NumStaticSamplers; i++) {
    const auto& samplerDesc = desc->pStaticSamplers[i];

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.addressModeU = convertTextureAddressMode(samplerDesc.AddressU);
    samplerInfo.addressModeV = convertTextureAddressMode(samplerDesc.AddressV);
    samplerInfo.addressModeW = convertTextureAddressMode(samplerDesc.AddressW);
    samplerInfo.maxAnisotropy = samplerDesc.MaxAnisotropy > 0 ? samplerDesc.MaxAnisotropy : 1;
    samplerInfo.maxLod = samplerDesc.MaxLOD;
    samplerInfo.minLod = samplerDesc.MinLOD;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSampler sampler;
    VkResult result = vkCreateSampler(vkDev, &samplerInfo, nullptr, &sampler);
    if (result == VK_SUCCESS) {
      staticSamplers.push_back(sampler);
    }
  }

  VkDescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = (uint32_t)bindings.size();
  layoutInfo.pBindings = bindings.data();

  VkResult result = vkCreateDescriptorSetLayout(vkDev, &layoutInfo, nullptr, &m_descriptorSetLayout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12RootSignatureImpl: vkCreateDescriptorSetLayout failed %d", result);
  }

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
  pipelineLayoutInfo.pushConstantRangeCount = 0;

  if (pushConstantSize > 0) {
    VkPushConstantRange pushRange = {};
    pushRange.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    pushRange.offset = 0;
    pushRange.size = pushConstantSize;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushRange;
  }

  result = vkCreatePipelineLayout(vkDev, &pipelineLayoutInfo, nullptr, &m_pipelineLayout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12RootSignatureImpl: vkCreatePipelineLayout failed %d", result);
  }

  VKWIND11_LOG_INFO("D3D12RootSignatureImpl created bindings=%zu pushConstants=%u", bindings.size(), pushConstantSize);
}

D3D12RootSignatureImpl::~D3D12RootSignatureImpl() {
  if (!m_device) return;
  auto& vk = m_device->getVulkanDevice();
  VkDevice vkDev = vk.getDevice();

  if (m_pipelineLayout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(vkDev, m_pipelineLayout, nullptr);
  }
  if (m_descriptorSetLayout != VK_NULL_HANDLE) {
    vkDestroyDescriptorSetLayout(vkDev, m_descriptorSetLayout, nullptr);
  }
}

HRESULT STDMETHODCALLTYPE D3D12RootSignatureImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12RootSignature || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12RootSignature*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12RootSignatureImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

// ============================================================================
// D3D12 Pipeline State
// ============================================================================

D3D12PipelineStateImpl::~D3D12PipelineStateImpl() {
  destroy();
}

void D3D12PipelineStateImpl::destroy() {
  if (m_device && m_pipeline != VK_NULL_HANDLE) {
    auto& vk = m_device->getVulkanDevice();
    vkDestroyPipeline(vk.getDevice(), m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;
  }
}

HRESULT STDMETHODCALLTYPE D3D12PipelineStateImpl::QueryInterface(REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  if (riid == IID_ID3D12PipelineState || riid == IID_IUnknown) {
    *ppvObject = static_cast<ID3D12PipelineState*>(this);
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE D3D12PipelineStateImpl::GetDevice(REFIID riid, void** ppDevice) {
  if (!ppDevice) return E_POINTER;
  return m_device->QueryInterface(riid, ppDevice);
}

HRESULT STDMETHODCALLTYPE D3D12PipelineStateImpl::GetCachedBlob(void** ppBlob) {
  if (ppBlob) *ppBlob = nullptr;
  return E_FAIL;
}

bool D3D12PipelineStateImpl::createGraphics(D3D12Device* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* desc) {
  m_device = device;
  m_isCompute = false;
  m_graphicsDesc = *desc;

  if (desc->pRootSignature) {
    m_pipelineLayout = static_cast<D3D12RootSignatureImpl*>(desc->pRootSignature)->getPipelineLayout();
  }

  auto& vk = device->getVulkanDevice();

  // Create shader modules from bytecode
  VkShaderModule vsModule = VK_NULL_HANDLE;
  VkShaderModule psModule = VK_NULL_HANDLE;
  VkShaderModule gsModule = VK_NULL_HANDLE;

  if (desc->VS.pShaderBytecode && desc->VS.BytecodeLength > 0) {
    vsModule = vk.createShaderModule((const uint32_t*)desc->VS.pShaderBytecode, desc->VS.BytecodeLength);
  }
  if (desc->PS.pShaderBytecode && desc->PS.BytecodeLength > 0) {
    psModule = vk.createShaderModule((const uint32_t*)desc->PS.pShaderBytecode, desc->PS.BytecodeLength);
  }
  if (desc->GS.pShaderBytecode && desc->GS.BytecodeLength > 0) {
    gsModule = vk.createShaderModule((const uint32_t*)desc->GS.pShaderBytecode, desc->GS.BytecodeLength);
  }

  // Shader stages
  std::vector<VkPipelineShaderStageCreateInfo> stages;
  if (vsModule != VK_NULL_HANDLE) {
    VkPipelineShaderStageCreateInfo stage = {};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    stage.module = vsModule;
    stage.pName = "main";
    stages.push_back(stage);
  }
  if (psModule != VK_NULL_HANDLE) {
    VkPipelineShaderStageCreateInfo stage = {};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stage.module = psModule;
    stage.pName = "main";
    stages.push_back(stage);
  }
  if (gsModule != VK_NULL_HANDLE) {
    VkPipelineShaderStageCreateInfo stage = {};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    stage.module = gsModule;
    stage.pName = "main";
    stages.push_back(stage);
  }

  if (stages.empty()) {
    VKWIND11_LOG_ERROR("D3D12PipelineStateImpl::createGraphics — no shader stages");
    return false;
  }

  // Vertex input (from input layout)
  std::vector<VkVertexInputBindingDescription> bindings;
  std::vector<VkVertexInputAttributeDescription> attributes;

  for (uint32_t i = 0; i < desc->InputLayout_NumElements; i++) {
    const auto& elem = desc->InputLayout_pInputElementDescs[i];

    uint32_t binding = elem.InputSlot;
    bool found = false;
    for (auto& b : bindings) {
      if (b.binding == binding) { found = true; break; }
    }
    if (!found) {
      VkVertexInputBindingDescription bindDesc = {};
      bindDesc.binding = binding;
      bindDesc.stride = 0;
      bindDesc.inputRate = (elem.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA) ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
      bindings.push_back(bindDesc);
    }

    VkVertexInputAttributeDescription attr = {};
    attr.location = (uint32_t)attributes.size();
    attr.binding = binding;
    attr.format = convertDxgiToVkFormat(elem.Format);
    attr.offset = elem.AlignedByteOffset;
    attributes.push_back(attr);
  }

  VkPipelineVertexInputStateCreateInfo vertexInput = {};
  vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInput.vertexBindingDescriptionCount = (uint32_t)bindings.size();
  vertexInput.pVertexBindingDescriptions = bindings.data();
  vertexInput.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
  vertexInput.pVertexAttributeDescriptions = attributes.data();

  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Rasterizer
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.polygonMode = (desc->RasterizerState.FillMode == D3D12_FILL_MODE_WIREFRAME) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
  rasterizer.cullMode = convertCullMode(desc->RasterizerState.CullMode);
  rasterizer.frontFace = desc->RasterizerState.FrontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
  rasterizer.lineWidth = 1.0f;
  rasterizer.depthBiasEnable = desc->RasterizerState.DepthBias != 0 ? VK_TRUE : VK_FALSE;
  rasterizer.depthBiasConstantFactor = (float)desc->RasterizerState.DepthBias;
  rasterizer.depthBiasSlopeFactor = desc->RasterizerState.SlopeScaledDepthBias;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.rasterizationSamples = (VkSampleCountFlagBits)desc->SampleDesc_Count;

  // Depth stencil
  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = desc->DepthStencilState.DepthEnable ? VK_TRUE : VK_FALSE;
  depthStencil.depthWriteEnable = (desc->DepthStencilState.DepthWriteMask == D3D12_DEPTH_WRITE_MASK_ALL) ? VK_TRUE : VK_FALSE;
  depthStencil.depthCompareOp = convertComparisonFunc(desc->DepthStencilState.DepthFunc);
  depthStencil.stencilTestEnable = desc->DepthStencilState.StencilEnable ? VK_TRUE : VK_FALSE;

  // Blend state
  VkPipelineColorBlendAttachmentState blendAttachment = {};
  blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAttachment.blendEnable = desc->BlendState.RenderTarget[0].BlendEnable ? VK_TRUE : VK_FALSE;
  blendAttachment.srcColorBlendFactor = convertBlend(desc->BlendState.RenderTarget[0].SrcBlend);
  blendAttachment.dstColorBlendFactor = convertBlend(desc->BlendState.RenderTarget[0].DestBlend);
  blendAttachment.colorBlendOp = convertBlendOp(desc->BlendState.RenderTarget[0].BlendOp);
  blendAttachment.srcAlphaBlendFactor = convertBlend(desc->BlendState.RenderTarget[0].SrcBlendAlpha);
  blendAttachment.dstAlphaBlendFactor = convertBlend(desc->BlendState.RenderTarget[0].DestBlendAlpha);
  blendAttachment.alphaBlendOp = convertBlendOp(desc->BlendState.RenderTarget[0].BlendOpAlpha);

  std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(desc->NumRenderTargets > 0 ? desc->NumRenderTargets : 1, blendAttachment);

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.attachmentCount = (uint32_t)blendAttachments.size();
  colorBlending.pAttachments = blendAttachments.data();

  // Dynamic state
  std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = (uint32_t)dynamicStates.size();
  dynamicState.pDynamicStates = dynamicStates.data();

  // Viewport/scissor (placeholder — will be dynamic)
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  // Render pass
  VkFormat rtFormat = convertDxgiToVkFormat(desc->RTVFormats[0]);
  VkRenderPass renderPass = VK_NULL_HANDLE;

  // Create a simple render pass
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = rtFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorRef = {};
  colorRef.attachment = 0;
  colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo rpInfo = {};
  rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rpInfo.attachmentCount = 1;
  rpInfo.pAttachments = &colorAttachment;
  rpInfo.subpassCount = 1;
  rpInfo.pSubpasses = &subpass;
  rpInfo.dependencyCount = 1;
  rpInfo.pDependencies = &dependency;

  vkCreateRenderPass(vk.getDevice(), &rpInfo, nullptr, &renderPass);

  // Create graphics pipeline
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = (uint32_t)stages.size();
  pipelineInfo.pStages = stages.data();
  pipelineInfo.pVertexInputState = &vertexInput;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = m_pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;

  VkResult result = vkCreateGraphicsPipelines(vk.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);

  // Cleanup temporary resources
  if (vsModule != VK_NULL_HANDLE) vkDestroyShaderModule(vk.getDevice(), vsModule, nullptr);
  if (psModule != VK_NULL_HANDLE) vkDestroyShaderModule(vk.getDevice(), psModule, nullptr);
  if (gsModule != VK_NULL_HANDLE) vkDestroyShaderModule(vk.getDevice(), gsModule, nullptr);
  vkDestroyRenderPass(vk.getDevice(), renderPass, nullptr);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12PipelineStateImpl::createGraphics — vkCreateGraphicsPipelines failed %d", result);
    return false;
  }

  VKWIND11_LOG_INFO("D3D12PipelineStateImpl::createGraphics — pipeline created");
  return true;
}

bool D3D12PipelineStateImpl::createCompute(D3D12Device* device, const D3D12_COMPUTE_PIPELINE_STATE_DESC* desc) {
  m_device = device;
  m_isCompute = true;
  m_computeDesc = *desc;

  if (desc->pRootSignature) {
    m_pipelineLayout = static_cast<D3D12RootSignatureImpl*>(desc->pRootSignature)->getPipelineLayout();
  }

  auto& vk = device->getVulkanDevice();

  VkShaderModule csModule = VK_NULL_HANDLE;
  if (desc->CS.pShaderBytecode && desc->CS.BytecodeLength > 0) {
    csModule = vk.createShaderModule((const uint32_t*)desc->CS.pShaderBytecode, desc->CS.BytecodeLength);
  }

  if (csModule == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("D3D12PipelineStateImpl::createCompute — no compute shader");
    return false;
  }

  VkComputePipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  pipelineInfo.stage.module = csModule;
  pipelineInfo.stage.pName = "main";
  pipelineInfo.layout = m_pipelineLayout;

  VkResult result = vkCreateComputePipelines(vk.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline);

  vkDestroyShaderModule(vk.getDevice(), csModule, nullptr);

  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("D3D12PipelineStateImpl::createCompute — vkCreateComputePipelines failed %d", result);
    return false;
  }

  VKWIND11_LOG_INFO("D3D12PipelineStateImpl::createCompute — pipeline created");
  return true;
}
