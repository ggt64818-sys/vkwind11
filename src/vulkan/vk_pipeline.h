#pragma once

#include "../../src/vulkan/vk_types.h"
#include <vulkan/vulkan.h>
#include <cstring>
#include <mutex>
#include <map>

// ============================================================================
// Vulkan Pipeline Management
// ============================================================================
//
// Handles graphics/compute pipeline creation, caching, and management.
// D3D11 blend/rasterizer/depth states map to Vulkan pipeline states.
//
// PERFORMANCE: PipelineKey is a POD struct with fixed-size arrays — no
// heap allocations during hash/comparison. Critical for Mali-G57 where
// per-draw overhead directly impacts FPS.
//

// D3D11 limits
static constexpr uint32_t kMaxVertexBindings = 16;
static constexpr uint32_t kMaxVertexAttributes = 32;

// Pipeline cache key — POD struct, no heap allocations
struct PipelineKey {
  // Shader modules
  VkShaderModule vertexShader = VK_NULL_HANDLE;
  VkShaderModule pixelShader = VK_NULL_HANDLE;

  // Vertex input (fixed-size arrays — no vector)
  uint32_t numBindings = 0;
  uint32_t numAttributes = 0;
  VkVertexInputBindingDescription vertexBindings[kMaxVertexBindings] = {};
  VkVertexInputAttributeDescription vertexAttributes[kMaxVertexAttributes] = {};

  // Input assembly
  VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  // Dynamic state
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

  // Create or retrieve cached pipeline (synchronous)
  VkPipeline getOrCreateGraphicsPipeline(const PipelineKey& key);

  // Async pipeline: returns immediately, compiles in background
  // Returns VK_NULL_HANDLE if pipeline not ready yet (caller should skip draw)
  VkPipeline getOrCreateGraphicsPipelineAsync(const PipelineKey& key);

  // Check if an async pipeline is ready
  bool isAsyncPipelineReady(const PipelineKey& key) const;

  // Create pipeline layout
  VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout descriptorSetLayout);

  // Pipeline cache for disk caching
  void loadPipelineCache(const void* data, size_t size);
  std::vector<uint8_t> getPipelineCacheData() const;

  void clear();

  // Stats
  size_t cacheHitCount() const { return m_cacheHits; }
  size_t cacheMissCount() const { return m_cacheMisses; }

private:
  VkDevice m_device = VK_NULL_HANDLE;
  VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

  // Pipeline cache
  std::map<size_t, VulkanGraphicsPipeline> m_pipelines;
  mutable std::mutex m_mutex;

  // Async compilation pending pipelines
  std::map<size_t, PipelineKey> m_pendingPipelines;
  mutable std::mutex m_pendingMutex;

  size_t m_cacheHits = 0;
  size_t m_cacheMisses = 0;

  size_t computeKeyHash(const PipelineKey& key) const;
  VkPipeline createPipelineInternal(const PipelineKey& key, size_t hash);
};
