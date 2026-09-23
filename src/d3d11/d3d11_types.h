#pragma once

#include <cstdint>
#include <cstring>
#include <atomic>

// ============================================================================
// Minimal Windows type definitions (replaces <windows.h>)
// ============================================================================

using BOOL = int;
using BYTE = uint8_t;
using WORD = uint16_t;
using DWORD = uint32_t;
using LONG = int32_t;
using UINT = uint32_t;
using INT = int32_t;
using UINT8 = uint8_t;
using UINT16 = uint16_t;
using UINT64 = uint64_t;
using FLOAT = float;
using DOUBLE = double;
using ULONG = unsigned long;
using SIZE_T = size_t;
using LPVOID = void*;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

// Calling convention — no-op on non-Windows / ARM64
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif

#ifndef STDAPI
#define STDAPI extern "C" HRESULT STDMETHODCALLTYPE
#endif

#ifndef WINAPI
#define WINAPI
#endif

// Handle types (opaque pointers)
using HWND = void*;
using HMODULE = void*;
using HINSTANCE = void*;
using HANDLE = void*;
using HDC = void*;
using HGLRC = void*;

// ============================================================================
// D3D11 Feature Levels
// ============================================================================

#define D3D_FEATURE_LEVEL_9_1  0x9100
#define D3D_FEATURE_LEVEL_9_2  0x9200
#define D3D_FEATURE_LEVEL_9_3  0x9300
#define D3D_FEATURE_LEVEL_10_0 0xa000
#define D3D_FEATURE_LEVEL_10_1 0xa100
#define D3D_FEATURE_LEVEL_11_0 0xb000
#define D3D_FEATURE_LEVEL_11_1 0xb100
#define D3D_FEATURE_LEVEL_12_0 0xc000
#define D3D_FEATURE_LEVEL_12_1 0xc100

// ============================================================================
// D3D11 Enums
// ============================================================================

enum D3D11_USAGE : uint32_t {
  D3D11_USAGE_DEFAULT   = 0,
  D3D11_USAGE_IMMUTABLE = 1,
  D3D11_USAGE_DYNAMIC   = 2,
  D3D11_USAGE_STAGING   = 3,
};

enum D3D11_BIND_FLAG : uint32_t {
  D3D11_BIND_VERTEX_BUFFER    = 0x1,
  D3D11_BIND_INDEX_BUFFER     = 0x2,
  D3D11_BIND_CONSTANT_BUFFER  = 0x4,
  D3D11_BIND_SHADER_RESOURCE  = 0x8,
  D3D11_BIND_STREAM_OUTPUT    = 0x10,
  D3D11_BIND_RENDER_TARGET    = 0x20,
  D3D11_BIND_DEPTH_STENCIL    = 0x40,
  D3D11_BIND_UNORDERED_ACCESS = 0x80,
};

enum D3D11_CPU_ACCESS_FLAG : uint32_t {
  D3D11_CPU_ACCESS_WRITE = 0x10000,
  D3D11_CPU_ACCESS_READ  = 0x20000,
};

enum D3D11_RESOURCE_MISC_FLAG : uint32_t {
  D3D11_RESOURCE_MISC_GENERATE_MIPS        = 0x1,
  D3D11_RESOURCE_MISC_SHARED                = 0x2,
  D3D11_RESOURCE_MISC_TEXTURECUBE           = 0x4,
  D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS     = 0x10,
  D3D11_RESOURCE_MISC_BUFFER_STRUCTURED     = 0x20,
  D3D11_RESOURCE_MISC_RESOURCE_CLAMP        = 0x40,
  D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX     = 0x100,
  D3D11_RESOURCE_MISC_GDI_COMPATIBLE        = 0x200,
  D3D11_RESOURCE_MISC_SHARED_NTHANDLE       = 0x800,
  D3D11_RESOURCE_MISC_RESTRICTED_CONTENT    = 0x1000,
  D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE = 0x2000,
  D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER = 0x4000,
  D3D11_RESOURCE_MISC_CAPTURED              = 0x10000,
  D3D11_RESOURCE_MISC_TILEABLE              = 0x40000,
  D3D11_RESOURCE_MISC_SHARED_PROTECTED      = 0x100000,
  D3D11_RESOURCE_MISC_VERTEX_BUFFER_CUVIEW  = 0x200000,
};

enum D3D11_MAP : uint32_t {
  D3D11_MAP_READ               = 1,
  D3D11_MAP_WRITE              = 2,
  D3D11_MAP_READ_WRITE         = 3,
  D3D11_MAP_WRITE_DISCARD      = 4,
  D3D11_MAP_WRITE_NO_OVERWRITE = 5,
};

enum D3D11_MAP_FLAG : uint32_t {
  D3D11_MAP_FLAG_DO_NOT_WAIT = 0x100000,
};

enum D3D11_RESOURCE_DIMENSION : uint32_t {
  D3D11_RESOURCE_DIMENSION_UNKNOWN   = 0,
  D3D11_RESOURCE_DIMENSION_BUFFER    = 1,
  D3D11_RESOURCE_DIMENSION_TEXTURE1D = 2,
  D3D11_RESOURCE_DIMENSION_TEXTURE2D = 3,
  D3D11_RESOURCE_DIMENSION_TEXTURE3D = 4,
};

enum D3D11_SRV_DIMENSION : uint32_t {
  D3D11_SRV_DIMENSION_UNKNOWN          = 0,
  D3D11_SRV_DIMENSION_BUFFER           = 1,
  D3D11_SRV_DIMENSION_TEXTURE1D        = 2,
  D3D11_SRV_DIMENSION_TEXTURE1DARRAY   = 3,
  D3D11_SRV_DIMENSION_TEXTURE2D        = 4,
  D3D11_SRV_DIMENSION_TEXTURE2DARRAY   = 5,
  D3D11_SRV_DIMENSION_TEXTURE2DMS      = 6,
  D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY = 7,
  D3D11_SRV_DIMENSION_TEXTURE3D        = 8,
  D3D11_SRV_DIMENSION_TEXTURECUBE      = 9,
  D3D11_SRV_DIMENSION_TEXTURECUBEARRAY = 10,
  D3D11_SRV_DIMENSION_BUFFEREX         = 11,
};

enum D3D11_RTV_DIMENSION : uint32_t {
  D3D11_RTV_DIMENSION_UNKNOWN          = 0,
  D3D11_RTV_DIMENSION_BUFFER           = 1,
  D3D11_RTV_DIMENSION_TEXTURE1D        = 2,
  D3D11_RTV_DIMENSION_TEXTURE1DARRAY   = 3,
  D3D11_RTV_DIMENSION_TEXTURE2D        = 4,
  D3D11_RTV_DIMENSION_TEXTURE2DARRAY   = 5,
  D3D11_RTV_DIMENSION_TEXTURE2DMS      = 6,
  D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY = 7,
  D3D11_RTV_DIMENSION_TEXTURE3D        = 8,
};

enum D3D11_DSV_DIMENSION : uint32_t {
  D3D11_DSV_DIMENSION_UNKNOWN          = 0,
  D3D11_DSV_DIMENSION_TEXTURE1D        = 1,
  D3D11_DSV_DIMENSION_TEXTURE1DARRAY   = 2,
  D3D11_DSV_DIMENSION_TEXTURE2D        = 3,
  D3D11_DSV_DIMENSION_TEXTURE2DARRAY   = 4,
  D3D11_DSV_DIMENSION_TEXTURE2DMS      = 5,
  D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY = 6,
};

enum D3D11_UAV_DIMENSION : uint32_t {
  D3D11_UAV_DIMENSION_UNKNOWN        = 0,
  D3D11_UAV_DIMENSION_BUFFER         = 1,
  D3D11_UAV_DIMENSION_TEXTURE1D      = 2,
  D3D11_UAV_DIMENSION_TEXTURE1DARRAY = 3,
  D3D11_UAV_DIMENSION_TEXTURE2D      = 4,
  D3D11_UAV_DIMENSION_TEXTURE2DARRAY = 5,
  D3D11_UAV_DIMENSION_TEXTURE3D      = 6,
};

// Primitive topology
enum D3D11_PRIMITIVE_TOPOLOGY : uint32_t {
  D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED                  = 0,
  D3D11_PRIMITIVE_TOPOLOGY_POINTLIST                  = 1,
  D3D11_PRIMITIVE_TOPOLOGY_LINELIST                   = 2,
  D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP                  = 3,
  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST               = 4,
  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP              = 5,
  D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ               = 10,
  D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ              = 11,
  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ           = 12,
  D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ          = 13,
  D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST  = 33,
  D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST  = 34,
  D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST  = 35,
  D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST  = 36,
  D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST  = 37,
  D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST  = 38,
  D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST  = 39,
  D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST  = 40,
  D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST  = 41,
  D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST = 42,
  D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST = 43,
  D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST = 44,
  D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST = 45,
  D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST = 46,
  D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST = 47,
  D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST = 48,
  D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST = 49,
  D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST = 50,
  D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST = 51,
  D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST = 52,
  D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST = 53,
  D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST = 54,
  D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST = 55,
  D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST = 56,
  D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST = 57,
  D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST = 58,
  D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST = 59,
  D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST = 60,
  D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST = 61,
  D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST = 62,
  D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST = 63,
  D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST = 64,
};

// Texture format (DXGI_FORMAT subset — full list in dxgi_types.h)
enum DXGI_FORMAT : uint32_t {
  DXGI_FORMAT_UNKNOWN                      = 0,
  DXGI_FORMAT_R32G32B32A32_TYPELESS        = 1,
  DXGI_FORMAT_R32G32B32A32_FLOAT           = 2,
  DXGI_FORMAT_R32G32B32A32_UINT            = 3,
  DXGI_FORMAT_R32G32B32A32_SINT            = 4,
  DXGI_FORMAT_R32G32B32_TYPELESS           = 5,
  DXGI_FORMAT_R32G32B32_FLOAT              = 6,
  DXGI_FORMAT_R32G32B32_UINT               = 7,
  DXGI_FORMAT_R32G32B32_SINT               = 8,
  DXGI_FORMAT_R16G16B16A16_TYPELESS        = 9,
  DXGI_FORMAT_R16G16B16A16_FLOAT           = 10,
  DXGI_FORMAT_R16G16B16A16_UNORM           = 11,
  DXGI_FORMAT_R16G16B16A16_SNORM           = 12,
  DXGI_FORMAT_R16G16B16A16_UINT            = 13,
  DXGI_FORMAT_R16G16B16A16_SINT            = 14,
  DXGI_FORMAT_R32G8X24_TYPELESS            = 15,
  DXGI_FORMAT_D32_FLOAT_S8X24_UINT         = 16,
  DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS     = 17,
  DXGI_FORMAT_X32_TYPELESS_G8X24_UINT      = 18,
  DXGI_FORMAT_R10G10B10A2_TYPELESS         = 19,
  DXGI_FORMAT_R10G10B10A2_UNORM            = 20,
  DXGI_FORMAT_R10G10B10A2_UINT             = 21,
  DXGI_FORMAT_R11G11B10_FLOAT              = 22,
  DXGI_FORMAT_R8G8B8A8_TYPELESS            = 23,
  DXGI_FORMAT_R8G8B8A8_UNORM               = 24,
  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB          = 25,
  DXGI_FORMAT_R8G8B8A8_UINT                = 26,
  DXGI_FORMAT_R8G8B8A8_SNORM               = 27,
  DXGI_FORMAT_R8G8B8A8_SINT                = 28,
  DXGI_FORMAT_R16G16_TYPELESS              = 29,
  DXGI_FORMAT_R16G16_FLOAT                 = 30,
  DXGI_FORMAT_R16G16_UNORM                 = 31,
  DXGI_FORMAT_R16G16_SNORM                 = 32,
  DXGI_FORMAT_R16G16_UINT                  = 33,
  DXGI_FORMAT_R16G16_SINT                  = 34,
  DXGI_FORMAT_R32_TYPELESS                 = 35,
  DXGI_FORMAT_D32_FLOAT                    = 36,
  DXGI_FORMAT_R32_FLOAT                    = 37,
  DXGI_FORMAT_R32_UINT                     = 38,
  DXGI_FORMAT_R32_SINT                     = 39,
  DXGI_FORMAT_R24G8_TYPELESS               = 40,
  DXGI_FORMAT_D24_UNORM_S8_UINT            = 41,
  DXGI_FORMAT_R24_UNORM_X8_TYPELESS        = 42,
  DXGI_FORMAT_X24_TYPELESS_G8_UINT         = 43,
  DXGI_FORMAT_R8G8_TYPELESS                = 44,
  DXGI_FORMAT_R8G8_UNORM                   = 45,
  DXGI_FORMAT_R8G8_SNORM                   = 46,
  DXGI_FORMAT_R8G8_UINT                    = 47,
  DXGI_FORMAT_R8G8_SINT                    = 48,
  DXGI_FORMAT_R16_TYPELESS                 = 49,
  DXGI_FORMAT_R16_FLOAT                    = 50,
  DXGI_FORMAT_D16_UNORM                    = 51,
  DXGI_FORMAT_R16_UNORM                    = 52,
  DXGI_FORMAT_R16_SNORM                    = 53,
  DXGI_FORMAT_R16_UINT                     = 54,
  DXGI_FORMAT_R16_SINT                     = 55,
  DXGI_FORMAT_R8_TYPELESS                  = 56,
  DXGI_FORMAT_R8_UNORM                     = 57,
  DXGI_FORMAT_R8_SNORM                     = 58,
  DXGI_FORMAT_R8_UINT                      = 59,
  DXGI_FORMAT_R8_SINT                      = 60,
  DXGI_FORMAT_A8_UNORM                     = 61,
  DXGI_FORMAT_R1_UNORM                     = 62,
  DXGI_FORMAT_R9G9B9E5_SHAREDEXP           = 63,
  DXGI_FORMAT_R8G8_B8G8_UNORM              = 64,
  DXGI_FORMAT_G8R8_G8B8_UNORM              = 65,
  DXGI_FORMAT_BC1_TYPELESS                 = 66,
  DXGI_FORMAT_BC1_UNORM                    = 67,
  DXGI_FORMAT_BC1_UNORM_SRGB               = 68,
  DXGI_FORMAT_BC2_TYPELESS                 = 69,
  DXGI_FORMAT_BC2_UNORM                    = 70,
  DXGI_FORMAT_BC2_UNORM_SRGB               = 71,
  DXGI_FORMAT_BC3_TYPELESS                 = 72,
  DXGI_FORMAT_BC3_UNORM                    = 73,
  DXGI_FORMAT_BC3_UNORM_SRGB               = 74,
  DXGI_FORMAT_BC4_TYPELESS                 = 75,
  DXGI_FORMAT_BC4_UNORM                    = 76,
  DXGI_FORMAT_BC4_SNORM                    = 77,
  DXGI_FORMAT_BC5_TYPELESS                 = 78,
  DXGI_FORMAT_BC5_UNORM                    = 79,
  DXGI_FORMAT_BC5_SNORM                    = 80,
  DXGI_FORMAT_B5G6R5_UNORM                 = 81,
  DXGI_FORMAT_B5G5R5A1_UNORM               = 82,
  DXGI_FORMAT_B8G8R8A8_UNORM               = 83,
  DXGI_FORMAT_B8G8R8X8_UNORM               = 84,
  DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM   = 85,
  DXGI_FORMAT_B8G8R8A8_TYPELESS            = 86,
  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB          = 87,
  DXGI_FORMAT_B8G8R8X8_TYPELESS            = 88,
  DXGI_FORMAT_B8G8R8X8_UNORM_SRGB          = 89,
  DXGI_FORMAT_BC6H_TYPELESS                = 90,
  DXGI_FORMAT_BC6H_UF16                    = 91,
  DXGI_FORMAT_BC6H_SF16                    = 92,
  DXGI_FORMAT_BC7_TYPELESS                 = 93,
  DXGI_FORMAT_BC7_UNORM                    = 94,
  DXGI_FORMAT_BC7_UNORM_SRGB               = 95,
  DXGI_FORMAT_AYUV                         = 100,
  DXGI_FORMAT_Y410                         = 101,
  DXGI_FORMAT_Y416                         = 102,
  DXGI_FORMAT_NV12                         = 103,
  DXGI_FORMAT_P010                         = 104,
  DXGI_FORMAT_P016                         = 105,
  DXGI_FORMAT_420_OPAQUE                   = 106,
  DXGI_FORMAT_YUY2                         = 107,
  DXGI_FORMAT_Y210                         = 108,
  DXGI_FORMAT_Y216                         = 109,
  DXGI_FORMAT_NV11                         = 110,
  DXGI_FORMAT_AI44                         = 111,
  DXGI_FORMAT_IA44                         = 112,
  DXGI_FORMAT_P8                           = 113,
  DXGI_FORMAT_A8P8                         = 114,
  DXGI_FORMAT_B4G4R4A4_UNORM               = 115,
  DXGI_FORMAT_FORCE_UINT                   = 0xffffffff,
};

// ============================================================================
// D3D11 Structures
// ============================================================================

struct D3D11_BOX {
  uint32_t left, top, front;
  uint32_t right, bottom, back;
};

struct D3D11_RECT {
  int32_t left, top, right, bottom;
};

struct D3D11_VIEWPORT {
  float TopLeftX;
  float TopLeftY;
  float Width;
  float Height;
  float MinDepth;
  float MaxDepth;
};

struct D3D11_MAPPED_SUBRESOURCE {
  void* pData;
  uint32_t RowPitch;
  uint32_t DepthPitch;
};

struct D3D11_SUBRESOURCE_DATA {
  const void* pSysMem;
  uint32_t SysMemPitch;
  uint32_t SysMemSlicePitch;
};

struct D3D11_BUFFER_DESC {
  uint32_t ByteWidth;
  D3D11_USAGE Usage;
  D3D11_BIND_FLAG BindFlags;
  D3D11_CPU_ACCESS_FLAG CPUAccessFlags;
  D3D11_RESOURCE_MISC_FLAG MiscFlags;
  uint32_t StructureByteStride;
};

struct D3D11_TEXTURE1D_DESC {
  uint32_t Width;
  uint32_t MipLevels;
  uint32_t ArraySize;
  DXGI_FORMAT Format;
  D3D11_USAGE Usage;
  D3D11_BIND_FLAG BindFlags;
  D3D11_CPU_ACCESS_FLAG CPUAccessFlags;
  D3D11_RESOURCE_MISC_FLAG MiscFlags;
};

struct D3D11_TEXTURE2D_DESC {
  uint32_t Width;
  uint32_t Height;
  uint32_t MipLevels;
  uint32_t ArraySize;
  DXGI_FORMAT Format;
  uint32_t SampleDescCount;
  uint32_t SampleDescQuality;
  D3D11_USAGE Usage;
  D3D11_BIND_FLAG BindFlags;
  D3D11_CPU_ACCESS_FLAG CPUAccessFlags;
  D3D11_RESOURCE_MISC_FLAG MiscFlags;
};

struct D3D11_TEXTURE3D_DESC {
  uint32_t Width;
  uint32_t Height;
  uint32_t Depth;
  uint32_t MipLevels;
  DXGI_FORMAT Format;
  D3D11_USAGE Usage;
  D3D11_BIND_FLAG BindFlags;
  D3D11_CPU_ACCESS_FLAG CPUAccessFlags;
  D3D11_RESOURCE_MISC_FLAG MiscFlags;
};

struct D3D11_TEXTURE2D_DESC1 {
  uint32_t Width;
  uint32_t Height;
  uint32_t MipLevels;
  uint32_t ArraySize;
  DXGI_FORMAT Format;
  uint32_t SampleDescCount;
  uint32_t SampleDescQuality;
  D3D11_USAGE Usage;
  D3D11_BIND_FLAG BindFlags;
  D3D11_CPU_ACCESS_FLAG CPUAccessFlags;
  D3D11_RESOURCE_MISC_FLAG MiscFlags;
  uint32_t TileMode; // D3D11_TEXTURE_LAYOUT enum
};

// SRV descriptions
struct D3D11_BUFFER_SRV {
  union {
    uint32_t FirstElement;
    uint32_t ElementOffset;
  };
  union {
    uint32_t NumElements;
    uint32_t ElementWidth;
  };
};

struct D3D11_TEX1D_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
};

struct D3D11_TEX1D_ARRAY_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2D_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
};

struct D3D11_TEX2D_ARRAY_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2DMS_SRV {
  uint32_t UnusedField_NothingToDefine;
};

struct D3D11_TEX2DMS_ARRAY_SRV {
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX3D_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
};

struct D3D11_TEXCUBE_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
};

struct D3D11_TEXCUBE_ARRAY_SRV {
  uint32_t MostDetailedMip;
  uint32_t MipLevels;
  uint32_t First2DArrayFace;
  uint32_t NumCubes;
};

struct D3D11_BUFFEREX_SRV {
  uint32_t FirstElement;
  uint32_t NumElements;
  uint32_t Flags;
};

struct D3D11_SHADER_RESOURCE_VIEW_DESC {
  DXGI_FORMAT Format;
  D3D11_SRV_DIMENSION ViewDimension;
  union {
    D3D11_BUFFER_SRV Buffer;
    D3D11_TEX1D_SRV Texture1D;
    D3D11_TEX1D_ARRAY_SRV Texture1DArray;
    D3D11_TEX2D_SRV Texture2D;
    D3D11_TEX2D_ARRAY_SRV Texture2DArray;
    D3D11_TEX2DMS_SRV Texture2DMS;
    D3D11_TEX2DMS_ARRAY_SRV Texture2DMSArray;
    D3D11_TEX3D_SRV Texture3D;
    D3D11_TEXCUBE_SRV TextureCube;
    D3D11_TEXCUBE_ARRAY_SRV TextureCubeArray;
    D3D11_BUFFEREX_SRV BufferEx;
  };
};

// RTV descriptions
struct D3D11_BUFFER_RTV {
  union {
    uint32_t FirstElement;
    uint32_t ElementOffset;
  };
  union {
    uint32_t NumElements;
    uint32_t ElementWidth;
  };
};

struct D3D11_TEX1D_RTV {
  uint32_t MipSlice;
};

struct D3D11_TEX1D_ARRAY_RTV {
  uint32_t MipSlice;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2D_RTV {
  uint32_t MipSlice;
};

struct D3D11_TEX2D_ARRAY_RTV {
  uint32_t MipSlice;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2DMS_RTV {
  uint32_t UnusedField_NothingToDefine;
};

struct D3D11_TEX2DMS_ARRAY_RTV {
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX3D_RTV {
  uint32_t MipSlice;
  uint32_t FirstWSlice;
  uint32_t WSize;
};

struct D3D11_RENDER_TARGET_VIEW_DESC {
  DXGI_FORMAT Format;
  D3D11_RTV_DIMENSION ViewDimension;
  union {
    D3D11_BUFFER_RTV Buffer;
    D3D11_TEX1D_RTV Texture1D;
    D3D11_TEX1D_ARRAY_RTV Texture1DArray;
    D3D11_TEX2D_RTV Texture2D;
    D3D11_TEX2D_ARRAY_RTV Texture2DArray;
    D3D11_TEX2DMS_RTV Texture2DMS;
    D3D11_TEX2DMS_ARRAY_RTV Texture2DMSArray;
    D3D11_TEX3D_RTV Texture3D;
  };
};

// DSV descriptions
struct D3D11_TEX1D_DSV {
  uint32_t MipSlice;
};

struct D3D11_TEX1D_ARRAY_DSV {
  uint32_t MipSlice;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2D_DSV {
  uint32_t MipSlice;
};

struct D3D11_TEX2D_ARRAY_DSV {
  uint32_t MipSlice;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2DMS_DSV {
  uint32_t UnusedField_NothingToDefine;
};

struct D3D11_TEX2DMS_ARRAY_DSV {
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_DEPTH_STENCIL_VIEW_DESC {
  DXGI_FORMAT Format;
  D3D11_DSV_DIMENSION ViewDimension;
  uint32_t Flags;
  union {
    D3D11_TEX1D_DSV Texture1D;
    D3D11_TEX1D_ARRAY_DSV Texture1DArray;
    D3D11_TEX2D_DSV Texture2D;
    D3D11_TEX2D_ARRAY_DSV Texture2DArray;
    D3D11_TEX2DMS_DSV Texture2DMS;
    D3D11_TEX2DMS_ARRAY_DSV Texture2DMSArray;
  };
};

// UAV descriptions
struct D3D11_BUFFER_UAV {
  uint32_t FirstElement;
  uint32_t NumElements;
  uint32_t Flags;
};

struct D3D11_TEX1D_UAV {
  uint32_t MipSlice;
};

struct D3D11_TEX1D_ARRAY_UAV {
  uint32_t MipSlice;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX2D_UAV {
  uint32_t MipSlice;
};

struct D3D11_TEX2D_ARRAY_UAV {
  uint32_t MipSlice;
  uint32_t FirstArraySlice;
  uint32_t ArraySize;
};

struct D3D11_TEX3D_UAV {
  uint32_t MipSlice;
  uint32_t FirstWSlice;
  uint32_t WSize;
};

struct D3D11_UNORDERED_ACCESS_VIEW_DESC {
  DXGI_FORMAT Format;
  D3D11_UAV_DIMENSION ViewDimension;
  union {
    D3D11_BUFFER_UAV Buffer;
    D3D11_TEX1D_UAV Texture1D;
    D3D11_TEX1D_ARRAY_UAV Texture1DArray;
    D3D11_TEX2D_UAV Texture2D;
    D3D11_TEX2D_ARRAY_UAV Texture2DArray;
    D3D11_TEX3D_UAV Texture3D;
  };
};

// ============================================================================
// D3D11 State Descriptions
// ============================================================================

enum D3D11_BLEND : uint32_t {
  D3D11_BLEND_ZERO             = 1,
  D3D11_BLEND_ONE              = 2,
  D3D11_BLEND_SRC_COLOR        = 3,
  D3D11_BLEND_INV_SRC_COLOR    = 4,
  D3D11_BLEND_SRC_ALPHA        = 5,
  D3D11_BLEND_INV_SRC_ALPHA    = 6,
  D3D11_BLEND_DEST_ALPHA       = 7,
  D3D11_BLEND_INV_DEST_ALPHA   = 8,
  D3D11_BLEND_DEST_COLOR       = 9,
  D3D11_BLEND_INV_DEST_COLOR   = 10,
  D3D11_BLEND_SRC_ALPHA_SAT    = 11,
  D3D11_BLEND_BLEND_FACTOR     = 14,
  D3D11_BLEND_INV_BLEND_FACTOR = 15,
  D3D11_BLEND_SRC1_COLOR       = 16,
  D3D11_BLEND_INV_SRC1_COLOR   = 17,
  D3D11_BLEND_SRC1_ALPHA       = 18,
  D3D11_BLEND_INV_SRC1_ALPHA   = 19,
};

enum D3D11_BLEND_OP : uint32_t {
  D3D11_BLEND_OP_ADD          = 1,
  D3D11_BLEND_OP_SUBTRACT     = 2,
  D3D11_BLEND_OP_REV_SUBTRACT = 3,
  D3D11_BLEND_OP_MIN          = 4,
  D3D11_BLEND_OP_MAX          = 5,
};

enum D3D11_COLOR_WRITE_ENABLE : uint32_t {
  D3D11_COLOR_WRITE_ENABLE_RED   = 1,
  D3D11_COLOR_WRITE_ENABLE_GREEN = 2,
  D3D11_COLOR_WRITE_ENABLE_BLUE  = 4,
  D3D11_COLOR_WRITE_ENABLE_ALPHA = 8,
  D3D11_COLOR_WRITE_ENABLE_ALL  = 0xf,
};

enum D3D11_DEPTH_WRITE_MASK : uint32_t {
  D3D11_DEPTH_WRITE_MASK_ZERO = 0,
  D3D11_DEPTH_WRITE_MASK_ALL  = 1,
};

enum D3D11_COMPARISON_FUNC : uint32_t {
  D3D11_COMPARISON_NEVER         = 1,
  D3D11_COMPARISON_LESS          = 2,
  D3D11_COMPARISON_EQUAL         = 3,
  D3D11_COMPARISON_LESS_EQUAL    = 4,
  D3D11_COMPARISON_GREATER       = 5,
  D3D11_COMPARISON_NOT_EQUAL     = 6,
  D3D11_COMPARISON_GREATER_EQUAL = 7,
  D3D11_COMPARISON_ALWAYS        = 8,
};

enum D3D11_STENCIL_OP : uint32_t {
  D3D11_STENCIL_OP_KEEP    = 1,
  D3D11_STENCIL_OP_ZERO    = 2,
  D3D11_STENCIL_OP_REPLACE = 3,
  D3D11_STENCIL_OP_INCR_SAT = 4,
  D3D11_STENCIL_OP_DECR_SAT = 5,
  D3D11_STENCIL_OP_INVERT  = 6,
  D3D11_STENCIL_OP_INCR    = 7,
  D3D11_STENCIL_OP_DECR    = 8,
};

enum D3D11_FILL_MODE : uint32_t {
  D3D11_FILL_WIREFRAME = 2,
  D3D11_FILL_SOLID     = 3,
  D3D11_FILL_POINTS    = 4,
};

enum D3D11_CULL_MODE : uint32_t {
  D3D11_CULL_NONE             = 1,
  D3D11_CULL_FRONT            = 2,
  D3D11_CULL_BACK             = 3,
  D3D11_CULL_FRONT_AND_BACK   = 4,
};

enum D3D11_FILTER : uint32_t {
  D3D11_FILTER_MIN_MAG_MIP_POINT              = 0,
  D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR       = 0x1,
  D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT = 0x4,
  D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR       = 0x5,
  D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT       = 0x10,
  D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR = 0x11,
  D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT       = 0x14,
  D3D11_FILTER_MIN_MAG_MIP_LINEAR             = 0x15,
  D3D11_FILTER_ANISOTROPIC                    = 0x55,
  D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT   = 0x80,
  D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR  = 0x95,
  D3D11_FILTER_COMPARISON_ANISOTROPIC         = 0xd5,
};

enum D3D11_TEXTURE_ADDRESS_MODE : uint32_t {
  D3D11_TEXTURE_ADDRESS_WRAP        = 1,
  D3D11_TEXTURE_ADDRESS_MIRROR      = 2,
  D3D11_TEXTURE_ADDRESS_CLAMP       = 3,
  D3D11_TEXTURE_ADDRESS_BORDER      = 4,
  D3D11_TEXTURE_ADDRESS_MIRRORONCE  = 5,
};

// Blend state
struct D3D11_RENDER_TARGET_BLEND_DESC {
  uint8_t BlendEnable;
  D3D11_BLEND SrcBlend;
  D3D11_BLEND DestBlend;
  D3D11_BLEND_OP BlendOp;
  D3D11_BLEND SrcBlendAlpha;
  D3D11_BLEND DestBlendAlpha;
  D3D11_BLEND_OP BlendOpAlpha;
  uint8_t RenderTargetWriteMask;
};

struct D3D11_BLEND_DESC {
  uint8_t AlphaToCoverageEnable;
  uint8_t IndependentBlendEnable;
  D3D11_RENDER_TARGET_BLEND_DESC RenderTarget[8];
};

// Depth-stencil state
struct D3D11_DEPTH_STENCILOP_DESC {
  D3D11_STENCIL_OP StencilFailOp;
  D3D11_STENCIL_OP StencilDepthFailOp;
  D3D11_STENCIL_OP StencilPassOp;
  D3D11_COMPARISON_FUNC StencilFunc;
};

struct D3D11_DEPTH_STENCIL_DESC {
  uint8_t DepthEnable;
  D3D11_DEPTH_WRITE_MASK DepthWriteMask;
  D3D11_COMPARISON_FUNC DepthFunc;
  uint8_t StencilEnable;
  uint8_t StencilReadMask;
  uint8_t StencilWriteMask;
  D3D11_DEPTH_STENCILOP_DESC FrontFace;
  D3D11_DEPTH_STENCILOP_DESC BackFace;
};

// Rasterizer state
struct D3D11_RASTERIZER_DESC {
  D3D11_FILL_MODE FillMode;
  D3D11_CULL_MODE CullMode;
  uint8_t FrontCounterClockwise;
  int32_t DepthBias;
  float DepthBiasClamp;
  float SlopeScaledDepthBias;
  uint8_t DepthClipEnable;
  uint8_t ScissorEnable;
  uint8_t MultisampleEnable;
  uint8_t AntialiasedLineEnable;
};

struct D3D11_RASTERIZER_DESC1 {
  D3D11_FILL_MODE FillMode;
  D3D11_CULL_MODE CullMode;
  uint8_t FrontCounterClockwise;
  int32_t DepthBias;
  float DepthBiasClamp;
  float SlopeScaledDepthBias;
  uint8_t DepthClipEnable;
  uint8_t ScissorEnable;
  uint8_t MultisampleEnable;
  uint8_t AntialiasedLineEnable;
  uint8_t ForcedSampleCount;
};

// Sampler state
struct D3D11_SAMPLER_DESC {
  D3D11_FILTER Filter;
  D3D11_TEXTURE_ADDRESS_MODE AddressU;
  D3D11_TEXTURE_ADDRESS_MODE AddressV;
  D3D11_TEXTURE_ADDRESS_MODE AddressW;
  float MipLODBias;
  uint32_t MaxAnisotropy;
  D3D11_COMPARISON_FUNC ComparisonFunc;
  float BorderColor[4];
  float MinLOD;
  float MaxLOD;
};

// Input layout
struct D3D11_INPUT_ELEMENT_DESC {
  const char* SemanticName;
  uint32_t SemanticIndex;
  DXGI_FORMAT Format;
  uint32_t InputSlot;
  uint32_t AlignedByteOffset;
  uint32_t InputSlotClass;
  uint32_t InstanceDataStepRate;
};

enum D3D11_INPUT_CLASSIFICATION : uint32_t {
  D3D11_INPUT_PER_VERTEX_DATA   = 0,
  D3D11_INPUT_PER_INSTANCE_DATA = 1,
};

// Query
enum D3D11_QUERY : uint32_t {
  D3D11_QUERY_EVENT                 = 0,
  D3D11_QUERY_OCCLUSION             = 1,
  D3D11_QUERY_TIMESTAMP             = 2,
  D3D11_QUERY_TIMESTAMP_DISJOINT    = 3,
  D3D11_QUERY_PIPELINE_STATISTICS   = 4,
  D3D11_QUERY_OCCLUSION_PREDICATE   = 5,
  D3D11_QUERY_SO_STATISTICS         = 6,
  D3D11_QUERY_SO_OVERFLOW_PREDICATE = 7,
  D3D11_QUERY_SO_STATISTICS_STREAM0 = 8,
  D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM0 = 9,
  D3D11_QUERY_SO_STATISTICS_STREAM1 = 10,
  D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM1 = 11,
  D3D11_QUERY_SO_STATISTICS_STREAM2 = 12,
  D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM2 = 13,
  D3D11_QUERY_SO_STATISTICS_STREAM3 = 14,
  D3D11_QUERY_SO_OVERFLOW_PREDICATE_STREAM3 = 15,
  D3D11_QUERY_GPU_STATISTICS        = 16,
  D3D11_QUERY_TIMESTAMP_DISJOINT_EX = 17,
};

enum D3D11_QUERY_MISC_FLAG : uint32_t {
  D3D11_QUERY_MISC_PREDICATEHINT = 0x1,
};

// Pipeline statistics
struct D3D11_QUERY_DATA_PIPELINE_STATISTICS {
  uint64_t IAVertices;
  uint64_t IAPrimitives;
  uint64_t VSInvocations;
  uint64_t GSInvocations;
  uint64_t GSPrimitives;
  uint64_t CInvocations;
  uint64_t CPrimitives;
  uint64_t PSInvocations;
  uint64_t HSInvocations;
  uint64_t DSInvocations;
  uint64_t CSInvocations;
};

// ============================================================================
// DXGI Types
// ============================================================================

enum DXGI_USAGE : uint32_t {
  DXGI_USAGE_RENDER_TARGET_OUTPUT = 0x00000020,
  DXGI_USAGE_SHADER_INPUT        = 0x00000010,
  DXGI_USAGE_READ_ONLY           = 0x00000001,
  DXGI_USAGE_DISCARD_ON_PRESENT  = 0x00000004,
  DXGI_USAGE_UNORDERED_ACCESS    = 0x00000080,
};

enum DXGI_SWAP_EFFECT : uint32_t {
  DXGI_SWAP_EFFECT_DISCARD         = 0,
  DXGI_SWAP_EFFECT_SEQUENTIAL      = 1,
  DXGI_SWAP_EFFECT_FLIP_DISCARD    = 3,
  DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL = 4,
};

enum DXGI_SWAP_CHAIN_FLAG : uint32_t {
  DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH         = 0x1,
  DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE            = 0x4,
  DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING             = 0x200,
  DXGI_SWAP_CHAIN_FLAG_RESTRICTED_TO_TARGET_HW_SCALING = 0x20000,
};

enum DXGI_ADAPTER_FLAG : uint32_t {
  DXGI_ADAPTER_FLAG_NONE        = 0,
  DXGI_ADAPTER_FLAG_SOFTWARE    = 1,
  DXGI_ADAPTER_FLAG_REMOTE      = 2,
  DXGI_ADAPTER_FLAG_SOFTWARE_EMBEDDED = 4,
};

enum DXGI_MODE_SCANLINE_ORDER : uint32_t {
  DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED      = 0,
  DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE      = 1,
  DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST = 2,
  DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST = 3,
};

enum DXGI_MODE_SCALING : uint32_t {
  DXGI_MODE_SCALING_UNSPECIFIED  = 0,
  DXGI_MODE_SCALING_CENTERED     = 1,
  DXGI_MODE_SCALING_STRETCHED    = 2,
};

enum DXGI_MODE_ROTATION : uint32_t {
  DXGI_MODE_ROTATION_UNSPECIFIED = 0,
  DXGI_MODE_ROTATION_IDENTITY    = 1,
  DXGI_MODE_ROTATION_ROTATE90    = 2,
  DXGI_MODE_ROTATION_ROTATE180   = 3,
  DXGI_MODE_ROTATION_ROTATE270   = 4,
};

struct DXGI_MODE_DESC {
  uint32_t Width;
  uint32_t Height;
  DXGI_FORMAT Format;
  DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
  DXGI_MODE_SCALING Scaling;
  uint32_t RefreshRateNumerator;
  uint32_t RefreshRateDenominator;
};

struct DXGI_MODE_DESC1 {
  uint32_t Width;
  uint32_t Height;
  DXGI_FORMAT Format;
  DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
  DXGI_MODE_SCALING Scaling;
  uint32_t RefreshRateNumerator;
  uint32_t RefreshRateDenominator;
  uint8_t Stereo;
};

struct DXGI_RATIONAL {
  uint32_t Numerator;
  uint32_t Denominator;
};

struct DXGI_SAMPLE_DESC {
  uint32_t Count;
  uint32_t Quality;
};

struct DXGI_SWAP_CHAIN_DESC {
  DXGI_MODE_DESC BufferDesc;
  DXGI_SAMPLE_DESC SampleDesc;
  DXGI_USAGE BufferUsage;
  uint32_t BufferCount;
  void* OutputWindow; // HWND
  uint8_t Windowed;
  DXGI_SWAP_EFFECT SwapEffect;
  uint32_t Flags;
};

struct DXGI_OUTPUT_DESC {
  char DeviceName[32]; // device name
  D3D11_RECT DesktopCoordinates;
  uint8_t AttachedToDesktop;
  uint32_t Rotation; // DXGI_MODE_ROTATION
  uint32_t Monitor; // HMONITOR
};

// LUID (must be before DXGI_ADAPTER_DESC)
struct LUID {
  uint32_t LowPart;
  int32_t HighPart;
};

using DXGI_ADAPTER = LUID;

// LARGE_INTEGER
union LARGE_INTEGER {
  struct {
    uint32_t LowPart;
    int32_t HighPart;
  };
  int64_t QuadPart;
};

struct DXGI_ADAPTER_DESC {
  wchar_t Description[128];
  uint32_t VendorId;
  uint32_t DeviceId;
  uint32_t SubSysId;
  uint32_t Revision;
  size_t DedicatedVideoMemory;
  size_t DedicatedSystemMemory;
  size_t SharedSystemMemory;
  DXGI_ADAPTER Luid;
};

struct DXGI_ADAPTER_DESC1 {
  wchar_t Description[128];
  uint32_t VendorId;
  uint32_t DeviceId;
  uint32_t SubSysId;
  uint32_t Revision;
  size_t DedicatedVideoMemory;
  size_t DedicatedSystemMemory;
  size_t SharedSystemMemory;
  DXGI_ADAPTER Luid;
  DXGI_ADAPTER_FLAG Flags;
};

struct DXGI_ADAPTER_DESC2 {
  wchar_t Description[128];
  uint32_t VendorId;
  uint32_t DeviceId;
  uint32_t SubSysId;
  uint32_t Revision;
  size_t DedicatedVideoMemory;
  size_t DedicatedSystemMemory;
  size_t SharedSystemMemory;
  DXGI_ADAPTER Luid;
  DXGI_ADAPTER_FLAG Flags;
  uint32_t GraphicsPreemptionGranularity;
  uint32_t ComputePreemptionGranularity;
};

// ============================================================================
// D3D11 Resource Type Enum
// ============================================================================

enum D3D11_RESOURCE_RETURN_TYPE : uint32_t {
  D3D11_RETURN_TYPE_UNORM  = 1,
  D3D11_RETURN_TYPE_SNORM  = 2,
  D3D11_RETURN_TYPE_SINT   = 3,
  D3D11_RETURN_TYPE_UINT   = 4,
  D3D11_RETURN_TYPE_FLOAT  = 5,
  D3D11_RETURN_TYPE_MIXED  = 6,
};

enum D3D11_REGISTER_COMPONENT_TYPE : uint32_t {
  D3D11_REGISTER_COMPONENT_UNKNOWN   = 0,
  D3D11_REGISTER_COMPONENT_FLOAT32   = 1,
  D3D11_REGISTER_COMPONENT_FLOAT16_2 = 2,
  D3D11_REGISTER_COMPONENT_FLOAT16_4 = 3,
  D3D11_REGISTER_COMPONENT_UINT32    = 4,
  D3D11_REGISTER_COMPONENT_SINT32    = 5,
  D3D11_REGISTER_COMPONENT_UNORM32   = 6,
  D3D11_REGISTER_COMPONENT_SNORM32   = 7,
};

// ============================================================================
// Return codes (must be before IUnknown)
// ============================================================================

enum HRESULT : uint32_t {
  S_OK          = 0,
  S_FALSE       = 1,
  E_FAIL        = 0x80004005,
  E_INVALIDARG  = 0x80070057,
  E_OUTOFMEMORY = 0x8007000e,
  E_NOTIMPL     = 0x80004001,
  E_NOINTERFACE = 0x80004002,
  E_POINTER     = 0x80004003,
  E_ABORT       = 0x80004004,
  E_ACCESSDENIED = 0x80070005,
};

#define SUCCEEDED(hr) ((hr) < 0x80000000)
#define FAILED(hr)    ((hr) >= 0x80000000)

// ============================================================================
// IUnknown Interface (COM base)
// ============================================================================

struct GUID {
  uint32_t Data1;
  uint16_t Data2;
  uint16_t Data3;
  uint8_t  Data4[8];
};

using REFGUID = const GUID&;
using REFIID = const GUID&;
using REFCLSID = const GUID&;

// Forward declare IUnknown (COM base interface)
struct IUnknown {
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;
  virtual ULONG STDMETHODCALLTYPE AddRef() = 0;
  virtual ULONG STDMETHODCALLTYPE Release() = 0;
};

inline bool operator==(const GUID& a, const GUID& b) {
  return memcmp(&a, &b, sizeof(GUID)) == 0;
}

inline bool operator!=(const GUID& a, const GUID& b) {
  return !(a == b);
}

// {00000000-0000-0000-C000-000000000046}
inline constexpr GUID IID_IUnknown = {0x00000000, 0x0000, 0x0000, {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}};

// ============================================================================
// D3D11 GUIDs (minimal subset for device/swapchain creation)
// ============================================================================

// ID3D11Device
// {DB6F6ddb-AC77-4e88-8253-819DF9BBF140}
inline constexpr GUID IID_ID3D11Device = {0xDB6F6DDB, 0xAC77, 0x4E88, {0x82, 0x53, 0x81, 0x9D, 0xF9, 0xBB, 0xF1, 0x40}};

// ID3D11DeviceContext
// {c0bfa96c-e089-44fb-8eaf-26f8796190da}
inline constexpr GUID IID_ID3D11DeviceContext = {0xC0BFA96C, 0xE089, 0x44FB, {0x8E, 0xAF, 0x26, 0xF8, 0x79, 0x61, 0x90, 0xDA}};

// IDXGIFactory
// {7b7166ec-21c7-41ae-9bb6-4f575d73eaa4}
inline constexpr GUID IID_IDXGIFactory = {0x7B7166EC, 0x21C7, 0x41AE, {0x9B, 0xB6, 0x4F, 0x57, 0x5D, 0x73, 0xEA, 0xA4}};

// IDXGIFactory1
// {770aae78-f26f-4dba-a829-253c83d1b387}
inline constexpr GUID IID_IDXGIFactory1 = {0x770AAE78, 0xF26F, 0x4DBA, {0xA8, 0x29, 0x25, 0x3C, 0x83, 0xD1, 0xB3, 0x87}};

// IDXGIFactory2
// {54ec77fa-1377-44e6-8c32-88fd5f44c84c}
inline constexpr GUID IID_IDXGIFactory2 = {0x54EC77FA, 0x1377, 0x44E6, {0x8C, 0x32, 0x88, 0xFD, 0x5F, 0x44, 0xC8, 0x4C}};

// IDXGIAdapter
// {2411e7e1-1ac3-4962-894d-d1d7779852be}
inline constexpr GUID IID_IDXGIAdapter = {0x2411E7E1, 0x1AC3, 0x4962, {0x89, 0x4D, 0xD1, 0xD7, 0x77, 0x98, 0x52, 0xBE}};

// IDXGISwapChain
// {310d36a0-d2b7-41e6-af50-6c35d1a62b2e}
inline constexpr GUID IID_IDXGISwapChain = {0x310D36A0, 0xD2B7, 0x41E6, {0xAF, 0x50, 0x6C, 0x35, 0xD1, 0xA6, 0x2B, 0x2E}};

// IDXGISwapChain1
// {790a45f7-0d42-4876-983a-0a55cfe6f4aa}
inline constexpr GUID IID_IDXGISwapChain1 = {0x790A45F7, 0x0D42, 0x4876, {0x98, 0x3A, 0x0A, 0x55, 0xCF, 0xE6, 0xF4, 0xAA}};

// IDXGIDevice
// {54ec77fa-1377-44e6-8c32-88fd5f44c84c}
inline constexpr GUID IID_IDXGIDevice = {0x54EC77FA, 0x1377, 0x44E6, {0x8C, 0x32, 0x88, 0xFD, 0x5F, 0x44, 0xC8, 0x4C}};

// IDXGIDevice1
// {77db970f-6276-48ba-ba28-039e3d7503a2}
inline constexpr GUID IID_IDXGIDevice1 = {0x77DB970F, 0x6276, 0x48BA, {0xBA, 0x28, 0x03, 0x9E, 0x3D, 0x75, 0x03, 0xA2}};

// ============================================================================
// Additional missing forward declarations and types
// ============================================================================

// ID3D11DeviceChild
// {9B7E4C04-342C-4106-A19F-4F2704FE5045}
inline constexpr GUID IID_ID3D11DeviceChild = {0x9B7E4C04, 0x342C, 0x4106, {0xA1, 0x9F, 0x4F, 0x27, 0x04, 0xFE, 0x50, 0x45}};

// ID3D11Resource
// {DC8E63F3-D605-4C4B-B7A5-047E113E2B95}
inline constexpr GUID IID_ID3D11Resource = {0xDC8E63F3, 0xD605, 0x4C4B, {0xB7, 0xA5, 0x04, 0x7E, 0x11, 0x3E, 0x2B, 0x95}};

// ID3D11Buffer
// {48570B38-CFE3-4C0B-9D15-D0CC77BBC938}
inline constexpr GUID IID_ID3D11Buffer = {0x48570B38, 0xCFE3, 0x4C0B, {0x9D, 0x15, 0xD0, 0xCC, 0x77, 0xBB, 0xC9, 0x38}};

// ID3D11Texture2D
// {6F15AAF2-D208-401B-83CD-F82C808C4204}
inline constexpr GUID IID_ID3D11Texture2D = {0x6F15AAF2, 0xD208, 0x401B, {0x83, 0xCD, 0xF8, 0x2C, 0x80, 0x8C, 0x42, 0x04}};

// ID3D11View
// {839D120F-DCB4-11D0-8720-00B05A9C3702}
inline constexpr GUID IID_ID3D11View = {0x839D120F, 0xDCB4, 0x11D0, {0x87, 0x20, 0x00, 0xB0, 0x5A, 0x9C, 0x37, 0x02}};

// ID3D11ShaderResourceView
// {B3E06330-04CF-418E-A9CF-F496B4DB412F}
inline constexpr GUID IID_ID3D11ShaderResourceView = {0xB3E06330, 0x04CF, 0x418E, {0xA9, 0xCF, 0xF4, 0x96, 0xB4, 0xDB, 0x41, 0x2F}};

// ID3D11RenderTargetView
// {DFDB010A-2341-4AFD-B42A-F2ACDE69A66F}
inline constexpr GUID IID_ID3D11RenderTargetView = {0xDFDB010A, 0x2341, 0x4AFD, {0xB4, 0x2A, 0xF2, 0xAC, 0xDE, 0x69, 0xA6, 0x6F}};

// ID3D11DepthStencilView
// {9FD3D64C-100C-4164-98C4-1B5E5C78950D}
inline constexpr GUID IID_ID3D11DepthStencilView = {0x9FD3D64C, 0x100C, 0x4164, {0x98, 0xC4, 0x1B, 0x5E, 0x5C, 0x78, 0x95, 0x0D}};

// ID3D11VertexShader
// {3B301427-2CD0-41BE-817F-804527700F3F}
inline constexpr GUID IID_ID3D11VertexShader = {0x3B301427, 0x2CD0, 0x41BE, {0x81, 0x7F, 0x80, 0x45, 0x27, 0x70, 0x0F, 0x3F}};

// ID3D11PixelShader
// {EA82E48D-51A5-43E0-9B3A-515D38FD5D30}
inline constexpr GUID IID_ID3D11PixelShader = {0xEA82E48D, 0x51A5, 0x43E0, {0x9B, 0x3A, 0x51, 0x5D, 0x38, 0xFD, 0x5D, 0x30}};

// ID3D11ComputeShader
// {4DBCD520-480F-4FAC-A06C-30F4E9CAFE6D}
inline constexpr GUID IID_ID3D11ComputeShader = {0x4DBCD520, 0x480F, 0x4FAC, {0xA0, 0x6C, 0x30, 0xF4, 0xE9, 0xCA, 0xFE, 0x6D}};

// ID3D11HullShader
// {153255D2-67A6-44A0-9362-F47E49A27D38}
inline constexpr GUID IID_ID3D11HullShader = {0x153255D2, 0x67A6, 0x44A0, {0x93, 0x62, 0xF4, 0x7E, 0x49, 0xA2, 0x7D, 0x38}};

// ID3D11DomainShader
// {6C8013A1-7EE8-4D60-8193-79D7DF4B7D0A}
inline constexpr GUID IID_ID3D11DomainShader = {0x6C8013A1, 0x7EE8, 0x4D60, {0x81, 0x93, 0x79, 0xD7, 0xDF, 0x4B, 0x7D, 0x0A}};

// ID3D11GeometryShader
// {35C975BB-4D04-468A-BE68-49909458053F}
inline constexpr GUID IID_ID3D11GeometryShader = {0x35C975BB, 0x4D04, 0x468A, {0xBE, 0x68, 0x49, 0x90, 0x94, 0x58, 0x05, 0x3F}};

// ID3D11BlendState
// {959D1718-58D4-4D45-B54E-B1D009B2A954}
inline constexpr GUID IID_ID3D11BlendState = {0x959D1718, 0x58D4, 0x4D45, {0xB5, 0x4E, 0xB1, 0xD0, 0x09, 0xB2, 0xA9, 0x54}};

// ID3D11RasterizerState
// {9BB4AB52-7662-4621-BA46-6D16D549A572}
inline constexpr GUID IID_ID3D11RasterizerState = {0x9BB4AB52, 0x7662, 0x4621, {0xBA, 0x46, 0x6D, 0x16, 0xD5, 0x49, 0xA5, 0x72}};

// ID3D11DepthStencilState
// {074361B0-809E-4C9D-A52D-5871B06AD437}
inline constexpr GUID IID_ID3D11DepthStencilState = {0x074361B0, 0x809E, 0x4C9D, {0xA5, 0x2D, 0x58, 0x71, 0xB0, 0x6A, 0xD4, 0x37}};

// ID3D11SamplerState
// {9C34AC36-5862-4E68-B09A-678FDE421B06}
inline constexpr GUID IID_ID3D11SamplerState = {0x9C38E0B6, 0x5862, 0x4E68, {0xB0, 0x9A, 0x67, 0x8F, 0xDE, 0x42, 0x1B, 0x06}};

// ID3D11InputLayout
// {E873DF62-31ED-4ABE-A7D6-31AC7322E8C3}
inline constexpr GUID IID_ID3D11InputLayout = {0xE873DF62, 0x31ED, 0x4ABE, {0xA7, 0xD6, 0x31, 0xAC, 0x73, 0x22, 0xE8, 0xC3}};

// D3D11_FEATURE enum
enum D3D11_FEATURE : uint32_t {
  D3D11_FEATURE_DOUBLES = 0,
  D3D11_FEATURE_SHADER_ROOT_ACCESS = 1,
  D3D11_FEATURE_D3D11_OPTIONS = 2,
  D3D11_FEATURE_ARCHITECTURE1 = 3,
  D3D11_FEATURE_D3D9_OPTIONS = 4,
  D3D11_FEATURE_D3D9_SHADOW_SUPPORT = 5,
  D3D11_FEATURE_D3D11_OPTIONS1 = 6,
  D3D11_FEATURE_D3D9_SIMPLE_INSTANCING = 7,
  D3D11_FEATURE_D3D9_MMUL_WIDTH_LIMITATION = 8,
  D3D11_FEATURE_D3D9_MULTISAMPLE_RARE_DEFERRED_RESOURCE_RESOLVE = 9,
  D3D11_FEATURE_D3D9_DATA_RESHADOW = 10,
  D3D11_FEATURE_D3D11_OPTIONS2 = 11,
  D3D11_FEATURE_D3D11_OPTIONS3 = 12,
  D3D11_FEATURE_GPU_VIRTUAL_ADDRESS = 13,
};

// D3D11_DRIVER_TYPE
enum D3D11_DRIVER_TYPE : uint32_t {
  D3D11_DRIVER_TYPE_UNKNOWN = 0,
  D3D11_DRIVER_TYPE_HARDWARE = 1,
  D3D11_DRIVER_TYPE_REFERENCE = 2,
  D3D11_DRIVER_TYPE_NULL = 3,
  D3D11_DRIVER_TYPE_SOFTWARE = 4,
  D3D11_DRIVER_TYPE_WARP = 5,
};

// D3D_FEATURE_LEVEL typedef (enum)
using D3D_FEATURE_LEVEL = uint32_t;

// Forward declare interface types
struct ID3D11ClassLinkage;
struct ID3D11CommandList;
struct ID3D11SwapChain;

// DXGI_SWAP_CHAIN_DESC1
struct DXGI_SWAP_CHAIN_DESC1 {
  uint32_t Width;
  uint32_t Height;
  DXGI_FORMAT Format;
  uint8_t Stereo;
  DXGI_SAMPLE_DESC SampleDesc;
  DXGI_USAGE BufferUsage;
  uint32_t BufferCount;
  uint32_t Scaling;
  DXGI_SWAP_EFFECT SwapEffect;
  uint32_t AlphaMode;
  uint32_t Flags;
};

// DXGI_SWAP_CHAIN_FULLSCREEN_DESC
struct DXGI_SWAP_CHAIN_FULLSCREEN_DESC {
  uint32_t RefreshRateNumerator;
  uint32_t RefreshRateDenominator;
  uint32_t Scaling;
  uint32_t ScanlineOrder;
  uint8_t Windowed;
};

// ============================================================================
// DXGI Error Codes
// ============================================================================

enum DXGI_ERROR : uint32_t {
  DXGI_ERROR_NOT_FOUND = 0x887A0002,
  DXGI_ERROR_MORE_DATA = 0x887A0003,
  DXGI_ERROR_NOT_CURRENTLY_AVAILABLE = 0x887A005F,
  DXGI_ERROR_UNSUPPORTED = 0x887A0004,
};

// ============================================================================
// Additional DXGI GUIDs
// ============================================================================

// IDXGIObject
// {aec22fb8-76f3-4659-835d-b3013e36ac22}
inline constexpr GUID IID_IDXGIObject = {0xAEC22FB8, 0x76F3, 0x4659, {0x83, 0x5D, 0xB3, 0x01, 0x3E, 0x36, 0xAC, 0x22}};

// IDXGIDeviceSubObject
// {3c3d48f0-50fb-47c0-9b95-b59e8b23db7c}
inline constexpr GUID IID_IDXGIDeviceSubObject = {0x3C3D48F0, 0x50FB, 0x47C0, {0x9B, 0x95, 0xB5, 0x9E, 0x8B, 0x23, 0xDB, 0x7C}};

// ============================================================================
// End of D3D11 Types
// ============================================================================
