#include "vk_pipeline.h"
#include "../common/logging.h"

// ============================================================================
// VulkanPipelineManager Implementation
// ============================================================================

VulkanPipelineManager::~VulkanPipelineManager() {
  shutdown();
}

void VulkanPipelineManager::initialize(VkDevice device) {
  m_device = device;

  VkPipelineCacheCreateInfo cacheInfo = {};
  cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  VkResult result = vkCreatePipelineCache(device, &cacheInfo, nullptr, &m_pipelineCache);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create pipeline cache: %d", result);
  } else {
    VKWIND11_LOG_INFO("Pipeline cache created");
  }
}

void VulkanPipelineManager::shutdown() {
  if (m_device) {
    for (auto& [hash, pipeline] : m_pipelines) {
      if (pipeline.pipeline) vkDestroyPipeline(m_device, pipeline.pipeline, nullptr);
      if (pipeline.layout) vkDestroyPipelineLayout(m_device, pipeline.layout, nullptr);
    }
    m_pipelines.clear();

    if (m_pipelineCache) {
      vkDestroyPipelineCache(m_device, m_pipelineCache, nullptr);
      m_pipelineCache = VK_NULL_HANDLE;
    }
  }
  m_device = VK_NULL_HANDLE;
}

bool PipelineKey::operator==(const PipelineKey& other) const {
  if (vertexShader != other.vertexShader) return false;
  if (pixelShader != other.pixelShader) return false;
  if (topology != other.topology) return false;
  if (polygonMode != other.polygonMode) return false;
  if (cullMode != other.cullMode) return false;
  if (frontFace != other.frontFace) return false;
  if (depthTestEnable != other.depthTestEnable) return false;
  if (depthWriteEnable != other.depthWriteEnable) return false;
  if (depthCompareOp != other.depthCompareOp) return false;
  if (blendEnable != other.blendEnable) return false;
  if (renderPass != other.renderPass) return false;
  if (subpass != other.subpass) return false;
  // Vulkan structs don't have operator==; compare field-by-field
  if (vertexBindings.size() != other.vertexBindings.size()) return false;
  for (size_t i = 0; i < vertexBindings.size(); i++) {
    if (vertexBindings[i].binding != other.vertexBindings[i].binding) return false;
    if (vertexBindings[i].stride != other.vertexBindings[i].stride) return false;
    if (vertexBindings[i].inputRate != other.vertexBindings[i].inputRate) return false;
  }
  if (vertexAttributes.size() != other.vertexAttributes.size()) return false;
  for (size_t i = 0; i < vertexAttributes.size(); i++) {
    if (vertexAttributes[i].location != other.vertexAttributes[i].location) return false;
    if (vertexAttributes[i].binding != other.vertexAttributes[i].binding) return false;
    if (vertexAttributes[i].format != other.vertexAttributes[i].format) return false;
    if (vertexAttributes[i].offset != other.vertexAttributes[i].offset) return false;
  }
  return true;
}

size_t VulkanPipelineManager::computeKeyHash(const PipelineKey& key) const {
  // Boost-style hash combine: seed ^= hash(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2)
  auto combine = [](size_t& seed, size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  };

  size_t hash = 0;
  combine(hash, std::hash<VkShaderModule>{}(key.vertexShader));
  combine(hash, std::hash<VkShaderModule>{}(key.pixelShader));
  combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(key.topology)));
  combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(key.polygonMode)));
  combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(key.cullMode)));
  combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(key.frontFace)));
  combine(hash, std::hash<bool>{}(key.depthTestEnable));
  combine(hash, std::hash<bool>{}(key.depthWriteEnable));
  combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(key.depthCompareOp)));
  combine(hash, std::hash<bool>{}(key.blendEnable));
  combine(hash, std::hash<void*>{}((void*)key.renderPass));
  combine(hash, std::hash<uint32_t>{}(key.subpass));

  for (const auto& binding : key.vertexBindings) {
    combine(hash, std::hash<uint32_t>{}(binding.binding));
    combine(hash, std::hash<uint32_t>{}(binding.stride));
    combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(binding.inputRate)));
  }

  for (const auto& attr : key.vertexAttributes) {
    combine(hash, std::hash<uint32_t>{}(attr.location));
    combine(hash, std::hash<uint32_t>{}(attr.binding));
    combine(hash, std::hash<uint32_t>{}(static_cast<uint32_t>(attr.format)));
    combine(hash, std::hash<uint32_t>{}(attr.offset));
  }

  return hash;
}

VkPipeline VulkanPipelineManager::getOrCreateGraphicsPipeline(const PipelineKey& key) {
  // 1. Compute hash
  size_t hash = computeKeyHash(key);

  // 2. Check cache
  {
    std::lock_guard lock(m_mutex);
    auto it = m_pipelines.find(hash);
    if (it != m_pipelines.end() && it->second.pipeline) {
      return it->second.pipeline;
    }
  }

  if (!m_device) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: device not initialized");
    return VK_NULL_HANDLE;
  }

  // 3. Validate shader modules — at least one must be provided
  if (key.vertexShader == VK_NULL_HANDLE && key.pixelShader == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: both vertex and pixel shader are VK_NULL_HANDLE");
    return VK_NULL_HANDLE;
  }

  if (key.renderPass == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: renderPass is VK_NULL_HANDLE");
    return VK_NULL_HANDLE;
  }

  // 4. Create descriptor set layout for the pipeline
  //    Binding 0: uniform buffer (vertex stage) — cbuffer0
  //    Binding 1: combined image sampler (fragment stage) — texture0
  VkDescriptorSetLayoutBinding bindings[2] = {};

  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  bindings[0].pImmutableSamplers = nullptr;

  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  bindings[1].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
  descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorLayoutInfo.bindingCount = 2;
  descriptorLayoutInfo.pBindings = bindings;

  VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
  VkResult result = vkCreateDescriptorSetLayout(m_device, &descriptorLayoutInfo, nullptr, &descriptorSetLayout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: failed to create descriptor set layout: %d", result);
    return VK_NULL_HANDLE;
  }

  // 5. Create pipeline layout
  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;

  VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
  result = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: failed to create pipeline layout: %d", result);
    vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
    return VK_NULL_HANDLE;
  }

  // 6. Shader stages
  VkPipelineShaderStageCreateInfo vertStage = {};
  VkPipelineShaderStageCreateInfo fragStage = {};
  VkPipelineShaderStageCreateInfo stages[2] = {};
  uint32_t stageCount = 0;

  if (key.vertexShader != VK_NULL_HANDLE) {
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = key.vertexShader;
    vertStage.pName = "main";
    vertStage.pSpecializationInfo = nullptr;
    stages[stageCount++] = vertStage;
  }

  if (key.pixelShader != VK_NULL_HANDLE) {
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = key.pixelShader;
    fragStage.pName = "main";
    fragStage.pSpecializationInfo = nullptr;
    stages[stageCount++] = fragStage;
  }

  // 7. Vertex input state
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(key.vertexBindings.size());
  vertexInputInfo.pVertexBindingDescriptions = key.vertexBindings.empty() ? nullptr : key.vertexBindings.data();
  vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(key.vertexAttributes.size());
  vertexInputInfo.pVertexAttributeDescriptions = key.vertexAttributes.empty() ? nullptr : key.vertexAttributes.data();

  // 8. Input assembly
  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = key.topology;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  // 9. Viewport/scissor — dynamic state, so we provide dummy values
  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  // pViewports and pScissors are null since we use dynamic state

  // 10. Rasterization
  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = key.polygonMode;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = key.cullMode;
  rasterizer.frontFace = key.frontFace;
  rasterizer.depthBiasEnable = VK_FALSE;
  rasterizer.depthBiasConstantFactor = 0.0f;
  rasterizer.depthBiasClamp = 0.0f;
  rasterizer.depthBiasSlopeFactor = 0.0f;

  // 11. Multisampling — no MSAA for now
  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.minSampleShading = 1.0f;
  multisampling.pSampleMask = nullptr;
  multisampling.alphaToCoverageEnable = VK_FALSE;
  multisampling.alphaToOneEnable = VK_FALSE;

  // 12. Depth/stencil
  VkPipelineDepthStencilStateCreateInfo depthStencil = {};
  depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = key.depthTestEnable ? VK_TRUE : VK_FALSE;
  depthStencil.depthWriteEnable = key.depthWriteEnable ? VK_TRUE : VK_FALSE;
  depthStencil.depthCompareOp = key.depthCompareOp;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;
  depthStencil.front.failOp = VK_STENCIL_OP_KEEP;
  depthStencil.front.passOp = VK_STENCIL_OP_KEEP;
  depthStencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
  depthStencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStencil.front.compareMask = 0;
  depthStencil.front.writeMask = 0;
  depthStencil.front.reference = 0;
  depthStencil.back = depthStencil.front;
  depthStencil.minDepthBounds = 0.0f;
  depthStencil.maxDepthBounds = 1.0f;

  // 13. Color blending
  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  if (key.blendEnable) {
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  } else {
    colorBlendAttachment.blendEnable = VK_FALSE;
  }

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  // 14. Dynamic state: viewport + scissor
  VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = 2;
  dynamicState.pDynamicStates = dynamicStates;

  // 15. Assemble VkGraphicsPipelineCreateInfo
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = stageCount;
  pipelineInfo.pStages = stages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pTessellationState = nullptr;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = pipelineLayout;
  pipelineInfo.renderPass = key.renderPass;
  pipelineInfo.subpass = key.subpass;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
  pipelineInfo.basePipelineIndex = -1;

  // 16. Create the graphics pipeline
  VkPipeline pipeline = VK_NULL_HANDLE;
  result = vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: vkCreateGraphicsPipelines failed: %d", result);
    vkDestroyPipelineLayout(m_device, pipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout, nullptr);
    return VK_NULL_HANDLE;
  }

  // 17. Store in cache
  {
    std::lock_guard lock(m_mutex);
    VulkanGraphicsPipeline cached{};
    cached.pipeline = pipeline;
    cached.layout = pipelineLayout;
    cached.key = key;
    m_pipelines[hash] = cached;
  }

  return pipeline;
}

VkPipelineLayout VulkanPipelineManager::createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout) {
  VkPipelineLayoutCreateInfo layoutInfo = {};
  layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layoutInfo.setLayoutCount = 1;
  layoutInfo.pSetLayouts = &descriptorSetLayout;

  VkPipelineLayout layout;
  VkResult result = vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &layout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("Failed to create pipeline layout: %d", result);
    return VK_NULL_HANDLE;
  }

  return layout;
}

void VulkanPipelineManager::loadPipelineCache(const void* data, size_t size) {
  if (!m_pipelineCache || !data || size == 0) return;

  VkResult result = vkMergePipelineCaches(m_device, m_pipelineCache, 1,
    (const VkPipelineCache*)data);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_WARN("Failed to merge pipeline cache: %d", result);
  }
}

std::vector<uint8_t> VulkanPipelineManager::getPipelineCacheData() const {
  if (!m_pipelineCache) return {};

  size_t dataSize = 0;
  VkResult result = vkGetPipelineCacheData(m_device, m_pipelineCache, &dataSize, nullptr);
  if (result != VK_SUCCESS || dataSize == 0) return {};

  std::vector<uint8_t> data(dataSize);
  result = vkGetPipelineCacheData(m_device, m_pipelineCache, &dataSize, data.data());
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_WARN("Failed to get pipeline cache data: %d", result);
    return {};
  }

  return data;
}

void VulkanPipelineManager::clear() {
  std::lock_guard lock(m_mutex);
  for (auto& [hash, pipeline] : m_pipelines) {
    if (pipeline.pipeline) vkDestroyPipeline(m_device, pipeline.pipeline, nullptr);
    if (pipeline.layout) vkDestroyPipelineLayout(m_device, pipeline.layout, nullptr);
  }
  m_pipelines.clear();
}
