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
  if (alphaToCoverage != other.alphaToCoverage) return false;
  if (renderPass != other.renderPass) return false;
  if (subpass != other.subpass) return false;
  if (numBindings != other.numBindings) return false;
  if (numAttributes != other.numAttributes) return false;

  for (uint32_t i = 0; i < numBindings; i++) {
    if (vertexBindings[i].binding != other.vertexBindings[i].binding) return false;
    if (vertexBindings[i].stride != other.vertexBindings[i].stride) return false;
    if (vertexBindings[i].inputRate != other.vertexBindings[i].inputRate) return false;
  }
  for (uint32_t i = 0; i < numAttributes; i++) {
    if (vertexAttributes[i].location != other.vertexAttributes[i].location) return false;
    if (vertexAttributes[i].binding != other.vertexAttributes[i].binding) return false;
    if (vertexAttributes[i].format != other.vertexAttributes[i].format) return false;
    if (vertexAttributes[i].offset != other.vertexAttributes[i].offset) return false;
  }
  return true;
}

size_t VulkanPipelineManager::computeKeyHash(const PipelineKey& key) const {
  // FNV-1a style hash — fast, no virtual calls
  auto combine = [](size_t& seed, size_t value) {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  };

  size_t hash = 14695981039346656037ULL; // FNV offset basis
  combine(hash, (size_t)key.vertexShader);
  combine(hash, (size_t)key.pixelShader);
  combine(hash, (size_t)key.topology);
  combine(hash, (size_t)key.polygonMode);
  combine(hash, (size_t)key.cullMode);
  combine(hash, (size_t)key.frontFace);
  combine(hash, (size_t)key.depthTestEnable);
  combine(hash, (size_t)key.depthWriteEnable);
  combine(hash, (size_t)key.depthCompareOp);
  combine(hash, (size_t)key.blendEnable);
  combine(hash, (size_t)key.alphaToCoverage);
  combine(hash, (size_t)key.renderPass);
  combine(hash, (size_t)key.subpass);
  combine(hash, (size_t)key.numBindings);
  combine(hash, (size_t)key.numAttributes);

  for (uint32_t i = 0; i < key.numBindings; i++) {
    combine(hash, (size_t)key.vertexBindings[i].binding);
    combine(hash, (size_t)key.vertexBindings[i].stride);
  }

  for (uint32_t i = 0; i < key.numAttributes; i++) {
    combine(hash, (size_t)key.vertexAttributes[i].location);
    combine(hash, (size_t)key.vertexAttributes[i].binding);
    combine(hash, (size_t)key.vertexAttributes[i].format);
    combine(hash, (size_t)key.vertexAttributes[i].offset);
  }

  return hash;
}

// ============================================================================
// Pipeline creation (extracted for reuse by sync + async paths)
// ============================================================================

VkPipeline VulkanPipelineManager::createPipelineInternal(const PipelineKey& key, size_t hash) {
  if (!m_device) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: device not initialized");
    return VK_NULL_HANDLE;
  }

  if (key.vertexShader == VK_NULL_HANDLE && key.pixelShader == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: both shaders are VK_NULL_HANDLE");
    return VK_NULL_HANDLE;
  }

  if (key.renderPass == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("VulkanPipelineManager: renderPass is VK_NULL_HANDLE");
    return VK_NULL_HANDLE;
  }

  // Descriptor set layout
  VkDescriptorSetLayoutBinding bindings[2] = {};
  bindings[0].binding = 0;
  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount = 1;
  bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  bindings[1].binding = 1;
  bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[1].descriptorCount = 1;
  bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo dslInfo = {};
  dslInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  dslInfo.bindingCount = 2;
  dslInfo.pBindings = bindings;

  VkDescriptorSetLayout dsl = VK_NULL_HANDLE;
  VkResult result = vkCreateDescriptorSetLayout(m_device, &dslInfo, nullptr, &dsl);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("failed to create descriptor set layout: %d", result);
    return VK_NULL_HANDLE;
  }

  // Pipeline layout with push constants (alpha test: alphaRef + alphaFunc)
  VkPushConstantRange pcRange{};
  pcRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  pcRange.offset = 0;
  pcRange.size = 8; // float alphaRef + int alphaFunc

  VkPipelineLayoutCreateInfo plInfo = {};
  plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  plInfo.setLayoutCount = 1;
  plInfo.pSetLayouts = &dsl;
  plInfo.pushConstantRangeCount = 1;
  plInfo.pPushConstantRanges = &pcRange;
  VkPipelineLayout layout = VK_NULL_HANDLE;
  result = vkCreatePipelineLayout(m_device, &plInfo, nullptr, &layout);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("failed to create pipeline layout: %d", result);
    vkDestroyDescriptorSetLayout(m_device, dsl, nullptr);
    return VK_NULL_HANDLE;
  }

  // Shader stages
  VkPipelineShaderStageCreateInfo stages[2] = {};
  uint32_t stageCount = 0;

  if (key.vertexShader != VK_NULL_HANDLE) {
    stages[stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[stageCount].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[stageCount].module = key.vertexShader;
    stages[stageCount].pName = "main";
    stageCount++;
  }
  if (key.pixelShader != VK_NULL_HANDLE) {
    stages[stageCount].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[stageCount].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[stageCount].module = key.pixelShader;
    stages[stageCount].pName = "main";
    stageCount++;
  }

  // Vertex input
  VkPipelineVertexInputStateCreateInfo viInfo = {};
  viInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  viInfo.vertexBindingDescriptionCount = key.numBindings;
  viInfo.pVertexBindingDescriptions = key.numBindings > 0 ? key.vertexBindings : nullptr;
  viInfo.vertexAttributeDescriptionCount = key.numAttributes;
  viInfo.pVertexAttributeDescriptions = key.numAttributes > 0 ? key.vertexAttributes : nullptr;

  // Input assembly
  VkPipelineInputAssemblyStateCreateInfo iaInfo = {};
  iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  iaInfo.topology = key.topology;

  // Viewport
  VkPipelineViewportStateCreateInfo vpInfo = {};
  vpInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vpInfo.viewportCount = 1;
  vpInfo.scissorCount = 1;

  // Rasterization
  VkPipelineRasterizationStateCreateInfo rsInfo = {};
  rsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rsInfo.polygonMode = key.polygonMode;
  rsInfo.lineWidth = 1.0f;
  rsInfo.cullMode = key.cullMode;
  rsInfo.frontFace = key.frontFace;

  // Multisampling
  VkPipelineMultisampleStateCreateInfo msInfo = {};
  msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  msInfo.minSampleShading = 1.0f;
  msInfo.alphaToCoverageEnable = key.alphaToCoverage ? VK_TRUE : VK_FALSE;

  // Depth/stencil
  VkPipelineDepthStencilStateCreateInfo dsInfo = {};
  dsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  dsInfo.depthTestEnable = key.depthTestEnable ? VK_TRUE : VK_FALSE;
  dsInfo.depthWriteEnable = key.depthWriteEnable ? VK_TRUE : VK_FALSE;
  dsInfo.depthCompareOp = key.depthCompareOp;
  dsInfo.maxDepthBounds = 1.0f;
  dsInfo.front = {
    VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP,
    VK_COMPARE_OP_ALWAYS, 0, 0, 0
  };
  dsInfo.back = dsInfo.front;

  // Blend
  VkPipelineColorBlendAttachmentState cba = {};
  cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                       VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  if (key.blendEnable) {
    cba.blendEnable = VK_TRUE;
    cba.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    cba.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    cba.colorBlendOp = VK_BLEND_OP_ADD;
    cba.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    cba.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    cba.alphaBlendOp = VK_BLEND_OP_ADD;
  }

  VkPipelineColorBlendStateCreateInfo cbInfo = {};
  cbInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  cbInfo.logicOp = VK_LOGIC_OP_COPY;
  cbInfo.attachmentCount = 1;
  cbInfo.pAttachments = &cba;

  // Dynamic state
  VkDynamicState dynStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
  VkPipelineDynamicStateCreateInfo dynInfo = {};
  dynInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynInfo.dynamicStateCount = 2;
  dynInfo.pDynamicStates = dynStates;

  // Assemble
  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = stageCount;
  pipelineInfo.pStages = stages;
  pipelineInfo.pVertexInputState = &viInfo;
  pipelineInfo.pInputAssemblyState = &iaInfo;
  pipelineInfo.pViewportState = &vpInfo;
  pipelineInfo.pRasterizationState = &rsInfo;
  pipelineInfo.pMultisampleState = &msInfo;
  pipelineInfo.pDepthStencilState = &dsInfo;
  pipelineInfo.pColorBlendState = &cbInfo;
  pipelineInfo.pDynamicState = &dynInfo;
  pipelineInfo.layout = layout;
  pipelineInfo.renderPass = key.renderPass;
  pipelineInfo.subpass = key.subpass;
  pipelineInfo.basePipelineIndex = -1;

  VkPipeline pipeline = VK_NULL_HANDLE;
  result = vkCreateGraphicsPipelines(m_device, m_pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
  if (result != VK_SUCCESS) {
    VKWIND11_LOG_ERROR("vkCreateGraphicsPipelines failed: %d", result);
    vkDestroyPipelineLayout(m_device, layout, nullptr);
    vkDestroyDescriptorSetLayout(m_device, dsl, nullptr);
    return VK_NULL_HANDLE;
  }

  // Store in cache
  {
    std::lock_guard lock(m_mutex);
    VulkanGraphicsPipeline cached{};
    cached.pipeline = pipeline;
    cached.layout = layout;
    cached.key = key;
    m_pipelines[hash] = cached;
  }

  return pipeline;
}

// ============================================================================
// Synchronous path
// ============================================================================

VkPipeline VulkanPipelineManager::getOrCreateGraphicsPipeline(const PipelineKey& key) {
  size_t hash = computeKeyHash(key);

  {
    std::lock_guard lock(m_mutex);
    auto it = m_pipelines.find(hash);
    if (it != m_pipelines.end() && it->second.pipeline) {
      m_cacheHits++;
      return it->second.pipeline;
    }
  }
  m_cacheMisses++;

  return createPipelineInternal(key, hash);
}

// ============================================================================
// Async path — returns VK_NULL_HANDLE if not ready, caller should skip draw
// ============================================================================

VkPipeline VulkanPipelineManager::getOrCreateGraphicsPipelineAsync(const PipelineKey& key) {
  size_t hash = computeKeyHash(key);

  // 1. Check if already compiled
  {
    std::lock_guard lock(m_mutex);
    auto it = m_pipelines.find(hash);
    if (it != m_pipelines.end() && it->second.pipeline) {
      m_cacheHits++;
      return it->second.pipeline;
    }
  }

  // 2. Check if already pending
  {
    std::lock_guard lock(m_pendingMutex);
    auto it = m_pendingPipelines.find(hash);
    if (it != m_pendingPipelines.end()) {
      // Already being compiled — return VK_NULL_HANDLE to skip this draw
      m_cacheMisses++;
      return VK_NULL_HANDLE;
    }
  }

  // 3. Queue for async compilation
  {
    std::lock_guard lock(m_pendingMutex);
    m_pendingPipelines[hash] = key;
  }

  m_cacheMisses++;
  VKWIND11_LOG_DEBUG("Pipeline queued for async compile: hash=%zu", hash);

  // 4. Try synchronous compile for this first frame (fast path for warm cache)
  //    On subsequent calls with same key, it'll hit the pending map and return null
  VkPipeline result = createPipelineInternal(key, hash);

  // 5. Remove from pending
  {
    std::lock_guard lock(m_pendingMutex);
    m_pendingPipelines.erase(hash);
  }

  return result;
}

bool VulkanPipelineManager::isAsyncPipelineReady(const PipelineKey& key) const {
  size_t hash = computeKeyHash(key);
  std::lock_guard lock(m_mutex);
  auto it = m_pipelines.find(hash);
  return it != m_pipelines.end() && it->second.pipeline != VK_NULL_HANDLE;
}

// ============================================================================
// Pipeline cache disk I/O
// ============================================================================

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
  VkResult result = vkMergePipelineCaches(m_device, m_pipelineCache, 1, (const VkPipelineCache*)data);
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
  if (result != VK_SUCCESS) return {};
  return data;
}

void VulkanPipelineManager::clear() {
  std::lock_guard lock(m_mutex);
  for (auto& [hash, pipeline] : m_pipelines) {
    if (pipeline.pipeline) vkDestroyPipeline(m_device, pipeline.pipeline, nullptr);
    if (pipeline.layout) vkDestroyPipelineLayout(m_device, pipeline.layout, nullptr);
  }
  m_pipelines.clear();
  m_cacheHits = 0;
  m_cacheMisses = 0;
}
