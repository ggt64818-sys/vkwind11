#pragma once

#include "../../src/vulkan/vk_types.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <mutex>

// ============================================================================
// Vulkan Pipeline Management
// ============================================================================
//
// Handles graphics/compute pipeline creation, caching, and management.
// D3D11 blend/rasterizer/depth states map to Vulkan pipeline states.
//

// Pipeline cache key matching D3D11 state combinations
struct PipelineKey {
  // Shader modules
  VkShaderModule vertexShader = VK_NULL_HANDLE;
  VkShaderModule pixelShader = VK_NULL_HANDLE;

  // Vertex input
  std::vector<VkVertexInputBindingDescription> vertexBindings;
  std::vector<VkVertexInputAttributeDescription> vertexAttributes;

  // Input assembly
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Viewport/scissor (dynamic state)
  bool dynamicViewport = true;
  bool dynamicScissor = true;

  // Rasterization
  VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
  VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
  VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;

  // Depth/stencil
  bool depthTestEnable = true;
  bool depthWriteEnable = true;
  VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;

  // Blend
  bool blendEnable = false;

  // Render pass
  VkRenderPass renderPass = VK_NULL_HANDLE;
  uint32_t subpass = 0;

  bool operator==(const PipelineKey& other) const;
};

// Pipeline object
struct VulkanGraphicsPipeline {
  VkPipeline pipeline = VK_NULL_HANDLE;
  VkPipelineLayout layout = VK_NULL_HANDLE;
  PipelineKey key;
};

class VulkanPipelineManager {
public:
  VulkanPipelineManager() = default;
  ~VulkanPipelineManager();

  void initialize(VkDevice device);
  void shutdown();

  // Create or retrieve cached pipeline
  VkPipeline getOrCreateGraphicsPipeline(const PipelineKey& key);

  // Create pipeline layout
  VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout);

  // Pipeline cache for disk caching
  void loadPipelineCache(const void* data, size_t size);
  std::vector<uint8_t> getPipelineCacheData() const;

  void clear();

private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

  // Pipeline cache
  std::map<size_t, VulkanGraphicsPipeline> m_pipelines;
  std::mutex m_mutex;

  size_t computeKeyHash(const PipelineKey& key) const;
};
