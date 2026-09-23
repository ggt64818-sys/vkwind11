#pragma once

#include "../d3d11/d3d11_types.h"
#include <vulkan/vulkan.h>

// ============================================================================
// DXGI ↔ Vulkan Format Conversion (shared by D3D11 and D3D12)
// ============================================================================

inline VkFormat convertDxgiToVkFormat(DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_R32G32B32A32_FLOAT:    return VK_FORMAT_R32G32B32A32_SFLOAT;
    case DXGI_FORMAT_R32G32B32A32_UINT:     return VK_FORMAT_R32G32B32A32_UINT;
    case DXGI_FORMAT_R32G32B32A32_SINT:     return VK_FORMAT_R32G32B32A32_SINT;
    case DXGI_FORMAT_R32G32B32_FLOAT:       return VK_FORMAT_R32G32B32_SFLOAT;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:    return VK_FORMAT_R16G16B16A16_SFLOAT;
    case DXGI_FORMAT_R16G16B16A16_UNORM:    return VK_FORMAT_R16G16B16A16_UNORM;
    case DXGI_FORMAT_R16G16B16A16_UINT:     return VK_FORMAT_R16G16B16A16_UINT;
    case DXGI_FORMAT_R16G16B16A16_SNORM:    return VK_FORMAT_R16G16B16A16_SNORM;
    case DXGI_FORMAT_R16G16B16A16_SINT:     return VK_FORMAT_R16G16B16A16_SINT;
    case DXGI_FORMAT_R8G8B8A8_UNORM:        return VK_FORMAT_R8G8B8A8_UNORM;
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:   return VK_FORMAT_R8G8B8A8_SRGB;
    case DXGI_FORMAT_R8G8B8A8_UINT:         return VK_FORMAT_R8G8B8A8_UINT;
    case DXGI_FORMAT_R8G8B8A8_SNORM:        return VK_FORMAT_R8G8B8A8_SNORM;
    case DXGI_FORMAT_R8G8B8A8_SINT:         return VK_FORMAT_R8G8B8A8_SINT;
    case DXGI_FORMAT_B8G8R8A8_UNORM:        return VK_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
    case DXGI_FORMAT_B8G8R8X8_UNORM:        return VK_FORMAT_B8G8R8A8_UNORM;
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:   return VK_FORMAT_B8G8R8A8_SRGB;
    case DXGI_FORMAT_R16G16_FLOAT:          return VK_FORMAT_R16G16_SFLOAT;
    case DXGI_FORMAT_R16G16_UNORM:          return VK_FORMAT_R16G16_UNORM;
    case DXGI_FORMAT_R16G16_UINT:           return VK_FORMAT_R16G16_UINT;
    case DXGI_FORMAT_R16G16_SNORM:          return VK_FORMAT_R16G16_SNORM;
    case DXGI_FORMAT_R32_FLOAT:             return VK_FORMAT_R32_SFLOAT;
    case DXGI_FORMAT_R32_UINT:              return VK_FORMAT_R32_UINT;
    case DXGI_FORMAT_R32_SINT:              return VK_FORMAT_R32_SINT;
    case DXGI_FORMAT_R16_FLOAT:             return VK_FORMAT_R16_SFLOAT;
    case DXGI_FORMAT_R16_UNORM:             return VK_FORMAT_R16_UNORM;
    case DXGI_FORMAT_R16_UINT:              return VK_FORMAT_R16_UINT;
    case DXGI_FORMAT_R8_UNORM:              return VK_FORMAT_R8_UNORM;
    case DXGI_FORMAT_R8_UINT:               return VK_FORMAT_R8_UINT;
    case DXGI_FORMAT_R8_SNORM:              return VK_FORMAT_R8_SNORM;
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:    return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case DXGI_FORMAT_BC1_UNORM:             return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    case DXGI_FORMAT_BC1_UNORM_SRGB:        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    case DXGI_FORMAT_BC2_UNORM:             return VK_FORMAT_BC2_UNORM_BLOCK;
    case DXGI_FORMAT_BC2_UNORM_SRGB:        return VK_FORMAT_BC2_SRGB_BLOCK;
    case DXGI_FORMAT_BC3_UNORM:             return VK_FORMAT_BC3_UNORM_BLOCK;
    case DXGI_FORMAT_BC3_UNORM_SRGB:        return VK_FORMAT_BC3_SRGB_BLOCK;
    case DXGI_FORMAT_BC4_UNORM:             return VK_FORMAT_BC4_UNORM_BLOCK;
    case DXGI_FORMAT_BC4_SNORM:             return VK_FORMAT_BC4_SNORM_BLOCK;
    case DXGI_FORMAT_BC5_UNORM:             return VK_FORMAT_BC5_UNORM_BLOCK;
    case DXGI_FORMAT_BC5_SNORM:             return VK_FORMAT_BC5_SNORM_BLOCK;
    case DXGI_FORMAT_BC6H_UF16:             return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case DXGI_FORMAT_BC6H_SF16:             return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case DXGI_FORMAT_BC7_UNORM:             return VK_FORMAT_BC7_UNORM_BLOCK;
    case DXGI_FORMAT_BC7_UNORM_SRGB:        return VK_FORMAT_BC7_SRGB_BLOCK;
    case DXGI_FORMAT_D32_FLOAT:             return VK_FORMAT_D32_SFLOAT;
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:  return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case DXGI_FORMAT_D24_UNORM_S8_UINT:     return VK_FORMAT_D24_UNORM_S8_UINT;
    case DXGI_FORMAT_D16_UNORM:             return VK_FORMAT_D16_UNORM;
    default:                                return VK_FORMAT_R8G8B8A8_UNORM;
  }
}

// ============================================================================
// D3D12 → Vulkan Conversion Helpers
// ============================================================================

inline VkBlendFactor convertBlend(D3D12_BLEND blend) {
  switch (blend) {
    case D3D12_BLEND_ZERO:                return VK_BLEND_FACTOR_ZERO;
    case D3D12_BLEND_ONE:                 return VK_BLEND_FACTOR_ONE;
    case D3D12_BLEND_SRC_COLOR:           return VK_BLEND_FACTOR_SRC_COLOR;
    case D3D12_BLEND_INV_SRC_COLOR:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case D3D12_BLEND_SRC_ALPHA:           return VK_BLEND_FACTOR_SRC_ALPHA;
    case D3D12_BLEND_INV_SRC_ALPHA:       return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case D3D12_BLEND_DEST_ALPHA:          return VK_BLEND_FACTOR_DST_ALPHA;
    case D3D12_BLEND_INV_DEST_ALPHA:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case D3D12_BLEND_DEST_COLOR:          return VK_BLEND_FACTOR_DST_COLOR;
    case D3D12_BLEND_INV_DEST_COLOR:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case D3D12_BLEND_SRC_ALPHA_SAT:       return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case D3D12_BLEND_BLEND_FACTOR:        return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case D3D12_BLEND_INV_BLEND_FACTOR:    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case D3D12_BLEND_SRC1_COLOR:          return VK_BLEND_FACTOR_SRC1_COLOR;
    case D3D12_BLEND_INV_SRC1_COLOR:      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case D3D12_BLEND_SRC1_ALPHA:          return VK_BLEND_FACTOR_SRC1_ALPHA;
    case D3D12_BLEND_INV_SRC1_ALPHA:      return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:                              return VK_BLEND_FACTOR_ONE;
  }
}

inline VkBlendOp convertBlendOp(D3D12_BLEND_OP op) {
  switch (op) {
    case D3D12_BLEND_OP_ADD:          return VK_BLEND_OP_ADD;
    case D3D12_BLEND_OP_SUBTRACT:     return VK_BLEND_OP_SUBTRACT;
    case D3D12_BLEND_OP_REV_SUBTRACT: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case D3D12_BLEND_OP_MIN:          return VK_BLEND_OP_MIN;
    case D3D12_BLEND_OP_MAX:          return VK_BLEND_OP_MAX;
    default:                          return VK_BLEND_OP_ADD;
  }
}

inline VkCompareOp convertComparisonFunc(D3D12_COMPARISON_FUNC func) {
  switch (func) {
    case D3D12_COMPARISON_FUNC_NEVER:         return VK_COMPARE_OP_NEVER;
    case D3D12_COMPARISON_FUNC_LESS:          return VK_COMPARE_OP_LESS;
    case D3D12_COMPARISON_FUNC_EQUAL:         return VK_COMPARE_OP_EQUAL;
    case D3D12_COMPARISON_FUNC_LESS_EQUAL:    return VK_COMPARE_OP_LESS_OR_EQUAL;
    case D3D12_COMPARISON_FUNC_GREATER:       return VK_COMPARE_OP_GREATER;
    case D3D12_COMPARISON_FUNC_NOT_EQUAL:     return VK_COMPARE_OP_NOT_EQUAL;
    case D3D12_COMPARISON_FUNC_GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case D3D12_COMPARISON_FUNC_ALWAYS:        return VK_COMPARE_OP_ALWAYS;
    default:                                  return VK_COMPARE_OP_LESS;
  }
}

inline VkCullModeFlagBits convertCullMode(D3D12_CULL_MODE mode) {
  switch (mode) {
    case D3D12_CULL_MODE_NONE:  return VK_CULL_MODE_NONE;
    case D3D12_CULL_MODE_FRONT: return VK_CULL_MODE_FRONT_BIT;
    case D3D12_CULL_MODE_BACK:  return VK_CULL_MODE_BACK_BIT;
    default:                    return VK_CULL_MODE_BACK_BIT;
  }
}

inline VkSamplerAddressMode convertTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE mode) {
  switch (mode) {
    case D3D12_TEXTURE_ADDRESS_MODE_WRAP:        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case D3D12_TEXTURE_ADDRESS_MODE_MIRROR:      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case D3D12_TEXTURE_ADDRESS_MODE_CLAMP:       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case D3D12_TEXTURE_ADDRESS_MODE_BORDER:      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    default:                                     return VK_SAMPLER_ADDRESS_MODE_REPEAT;
  }
}
