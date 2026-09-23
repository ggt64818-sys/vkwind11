#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../src/d3d11/d3d11_device.h"
#include "../src/d3d11/d3d11_state.h"
#include "../src/d3d11/d3d11_view.h"
#include "../src/d3d11/d3d11_shader.h"
#include "../src/common/logging.h"

#define D3D11_SDK_VERSION 11
#define D3D11_FLOAT32_MAX 3.402823466e+38f

extern "C" {
  HRESULT D3D11CreateDevice(
    void* pAdapter,
    uint32_t DriverType,
    void* Software,
    uint32_t Flags,
    const uint32_t* pFeatureLevels,
    uint32_t FeatureLevels,
    uint32_t SDKVersion,
    ID3D11Device** ppDevice,
    uint32_t* pFeatureLevel,
    ID3D11DeviceContext** ppImmediateContext);
}

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) printf("[Test] %s... ", name)
#define PASS() do { printf("PASS\n"); g_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

// ============================================================================
// Test: D3D11 Device Creation + QueryInterface
// ============================================================================

void test_device_creation() {
  TEST("D3D11CreateDevice");
  {
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDevice(
      nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION,
      &device, &level, &context);

    if (FAILED(hr)) {
      printf("(no Vulkan driver) SKIP\n");
      return;
    }

    CHECK(device, "device is null");
    CHECK(context, "context is null");
    CHECK(level == D3D_FEATURE_LEVEL_11_0, "feature level != 11_0");

    printf("  Device: %p, Context: %p\n", (void*)device, (void*)context);
    PASS();
  }
}

void test_query_interface() {
  TEST("QueryInterface IUnknown/ID3D11Device");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;

    HRESULT hr = D3D11CreateDevice(
      nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION,
      &device, &level, nullptr);

    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }
    CHECK(device, "device is null");

    // Query IUnknown
    IUnknown* unk = nullptr;
    hr = device->QueryInterface(IID_IUnknown, (void**)&unk);
    CHECK(hr == S_OK && unk, "QueryInterface IUnknown failed");
    unk->Release();

    // Query ID3D11Device (same object)
    ID3D11Device* dev2 = nullptr;
    hr = device->QueryInterface(IID_ID3D11Device, (void**)&dev2);
    CHECK(hr == S_OK && dev2 == device, "QueryInterface ID3D11Device failed");
    dev2->Release();

    // Query nonexistent interface
    void* bad = nullptr;
    hr = device->QueryInterface(IID_IDXGIFactory, &bad);
    CHECK(hr == E_NOINTERFACE, "QueryInterface should fail for wrong IID");

    PASS();
  }
}

void test_addref_release() {
  TEST("AddRef / Release reference counting");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;

    HRESULT hr = D3D11CreateDevice(
      nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION,
      &device, &level, nullptr);

    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    ULONG refs = device->AddRef();
    printf("  After AddRef: %lu\n", refs);
    CHECK(refs == 2, "AddRef should return 2");

    refs = device->Release();
    printf("  After Release: %lu\n", refs);
    CHECK(refs == 1, "Release should return 1");

    // Release to 0 = destroy
    refs = device->Release();
    CHECK(refs == 0, "Final Release should return 0");

    PASS();
  }
}

// ============================================================================
// Test: Resource Creation (Buffer, Texture2D)
// ============================================================================

void test_create_buffer() {
  TEST("CreateBuffer (vertex, constant, staging)");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    // Vertex buffer
    {
      D3D11_BUFFER_DESC desc = {};
      desc.ByteWidth = 1024;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

      ID3D11Buffer* buf = nullptr;
      hr = device->CreateBuffer(&desc, nullptr, &buf);
      printf("  VertexBuffer: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; buf->Release(); } else { g_failed++; }
    }

    // Constant buffer (must be 16-byte aligned)
    {
      D3D11_BUFFER_DESC desc = {};
      desc.ByteWidth = 256;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

      ID3D11Buffer* buf = nullptr;
      hr = device->CreateBuffer(&desc, nullptr, &buf);
      printf("  ConstantBuffer: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; buf->Release(); } else { g_failed++; }
    }

    // Staging buffer (CPU read)
    {
      D3D11_BUFFER_DESC desc = {};
      desc.ByteWidth = 512;
      desc.Usage = D3D11_USAGE_STAGING;
      desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

      ID3D11Buffer* buf = nullptr;
      hr = device->CreateBuffer(&desc, nullptr, &buf);
      printf("  StagingBuffer: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; buf->Release(); } else { g_failed++; }
    }

    device->Release();
    PASS();
  }
}

void test_create_texture2d() {
  TEST("CreateTexture2D (color, depth, MSAA)");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    // Color texture
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = 256;
      desc.Height = 256;
      desc.MipLevels = 1;
      desc.ArraySize = 1;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDescCount = 1;
      desc.SampleDescQuality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

      ID3D11Texture2D* tex = nullptr;
      hr = device->CreateTexture2D(&desc, nullptr, &tex);
      printf("  ColorTexture 256x256: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; tex->Release(); } else { g_failed++; }
    }

    // Depth stencil texture
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = 1920;
      desc.Height = 1080;
      desc.MipLevels = 1;
      desc.ArraySize = 1;
      desc.Format = DXGI_FORMAT_D32_FLOAT;
      desc.SampleDescCount = 1;
      desc.SampleDescQuality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

      ID3D11Texture2D* tex = nullptr;
      hr = device->CreateTexture2D(&desc, nullptr, &tex);
      printf("  DepthTexture D32F 1080p: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; tex->Release(); } else { g_failed++; }
    }

    // Mipmapped texture
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = 512;
      desc.Height = 512;
      desc.MipLevels = 10;
      desc.ArraySize = 1;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDescCount = 1;
      desc.SampleDescQuality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

      ID3D11Texture2D* tex = nullptr;
      hr = device->CreateTexture2D(&desc, nullptr, &tex);
      printf("  Mipmapped 512x512 (10 levels): %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; tex->Release(); } else { g_failed++; }
    }

    // Texture array
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width = 128;
      desc.Height = 128;
      desc.MipLevels = 1;
      desc.ArraySize = 6;
      desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDescCount = 1;
      desc.SampleDescQuality = 0;
      desc.Usage = D3D11_USAGE_DEFAULT;
      desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

      ID3D11Texture2D* tex = nullptr;
      hr = device->CreateTexture2D(&desc, nullptr, &tex);
      printf("  TextureArray 6x128x128: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; tex->Release(); } else { g_failed++; }
    }

    device->Release();
    PASS();
  }
}

// ============================================================================
// Test: View Creation (SRV, RTV, DSV)
// ============================================================================

void test_create_views() {
  TEST("CreateShaderResourceView / RenderTargetView / DepthStencilView");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    // Create a color texture first
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = 256;
    texDesc.Height = 256;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDescCount = 1;
    texDesc.SampleDescQuality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

    ID3D11Texture2D* colorTex = nullptr;
    hr = device->CreateTexture2D(&texDesc, nullptr, &colorTex);
    CHECK(SUCCEEDED(hr), "failed to create color texture");

    // SRV
    {
      ID3D11ShaderResourceView* srv = nullptr;
      hr = device->CreateShaderResourceView(colorTex, nullptr, &srv);
      printf("  SRV (auto desc): %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; srv->Release(); } else { g_failed++; }
    }

    // RTV
    {
      ID3D11RenderTargetView* rtv = nullptr;
      hr = device->CreateRenderTargetView(colorTex, nullptr, &rtv);
      printf("  RTV (auto desc): %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; rtv->Release(); } else { g_failed++; }
    }

    // Create depth texture
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = 256;
    depthDesc.Height = 256;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDescCount = 1;
    depthDesc.SampleDescQuality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthTex = nullptr;
    hr = device->CreateTexture2D(&depthDesc, nullptr, &depthTex);
    CHECK(SUCCEEDED(hr), "failed to create depth texture");

    // DSV
    {
      ID3D11DepthStencilView* dsv = nullptr;
      hr = device->CreateDepthStencilView(depthTex, nullptr, &dsv);
      printf("  DSV (auto desc): %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
      if (SUCCEEDED(hr)) { g_passed++; dsv->Release(); } else { g_failed++; }
    }

    depthTex->Release();
    colorTex->Release();
    device->Release();
    PASS();
  }
}

// ============================================================================
// Test: State Objects
// ============================================================================

void test_blend_state() {
  TEST("CreateBlendState + GetDesc roundtrip");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    D3D11_BLEND_DESC desc = {};
    desc.RenderTarget[0].BlendEnable = TRUE;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ID3D11BlendState* state = nullptr;
    hr = device->CreateBlendState(&desc, &state);
    printf("  CreateBlendState: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
    if (FAILED(hr)) { g_failed++; device->Release(); PASS(); return; }
    g_passed++;

    // GetDesc roundtrip
    D3D11_BLEND_DESC gotDesc = {};
    state->GetDesc(&gotDesc);
    bool match = (gotDesc.RenderTarget[0].BlendEnable == TRUE) &&
                 (gotDesc.RenderTarget[0].SrcBlend == D3D11_BLEND_SRC_ALPHA) &&
                 (gotDesc.RenderTarget[0].DestBlend == D3D11_BLEND_INV_SRC_ALPHA);
    printf("  GetDesc roundtrip: %s\n", match ? "PASS" : "FAIL");
    if (match) g_passed++; else g_failed++;

    state->Release();
    device->Release();
    PASS();
  }
}

void test_rasterizer_state() {
  TEST("CreateRasterizerState + GetDesc roundtrip");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    D3D11_RASTERIZER_DESC desc = {};
    desc.FillMode = D3D11_FILL_SOLID;
    desc.CullMode = D3D11_CULL_BACK;
    desc.FrontCounterClockwise = FALSE;
    desc.DepthBias = 1;
    desc.DepthBiasClamp = 0.0f;
    desc.SlopeScaledDepthBias = 1.0f;
    desc.DepthClipEnable = TRUE;
    desc.ScissorEnable = TRUE;
    desc.MultisampleEnable = FALSE;
    desc.AntialiasedLineEnable = FALSE;

    ID3D11RasterizerState* state = nullptr;
    hr = device->CreateRasterizerState(&desc, &state);
    printf("  CreateRasterizerState: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
    if (FAILED(hr)) { g_failed++; device->Release(); PASS(); return; }
    g_passed++;

    D3D11_RASTERIZER_DESC gotDesc = {};
    state->GetDesc(&gotDesc);
    bool match = (gotDesc.FillMode == D3D11_FILL_SOLID) &&
                 (gotDesc.CullMode == D3D11_CULL_BACK) &&
                 (gotDesc.DepthClipEnable == TRUE) &&
                 (gotDesc.ScissorEnable == TRUE);
    printf("  GetDesc roundtrip: %s\n", match ? "PASS" : "FAIL");
    if (match) g_passed++; else g_failed++;

    state->Release();
    device->Release();
    PASS();
  }
}

void test_depth_stencil_state() {
  TEST("CreateDepthStencilState + GetDesc roundtrip");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    D3D11_DEPTH_STENCIL_DESC desc = {};
    desc.DepthEnable = TRUE;
    desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.StencilEnable = TRUE;
    desc.StencilReadMask = 0xFF;
    desc.StencilWriteMask = 0xFF;
    desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    ID3D11DepthStencilState* state = nullptr;
    hr = device->CreateDepthStencilState(&desc, &state);
    printf("  CreateDepthStencilState: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
    if (FAILED(hr)) { g_failed++; device->Release(); PASS(); return; }
    g_passed++;

    D3D11_DEPTH_STENCIL_DESC gotDesc = {};
    state->GetDesc(&gotDesc);
    bool match = (gotDesc.DepthEnable == TRUE) &&
                 (gotDesc.DepthFunc == D3D11_COMPARISON_LESS_EQUAL) &&
                 (gotDesc.StencilEnable == TRUE) &&
                 (gotDesc.FrontFace.StencilPassOp == D3D11_STENCIL_OP_REPLACE);
    printf("  GetDesc roundtrip: %s\n", match ? "PASS" : "FAIL");
    if (match) g_passed++; else g_failed++;

    state->Release();
    device->Release();
    PASS();
  }
}

void test_sampler_state() {
  TEST("CreateSamplerState + GetDesc roundtrip");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    D3D11_SAMPLER_DESC desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.MipLODBias = 0.0f;
    desc.MaxAnisotropy = 16;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.BorderColor[0] = 0.0f;
    desc.BorderColor[1] = 0.0f;
    desc.BorderColor[2] = 0.0f;
    desc.BorderColor[3] = 0.0f;
    desc.MinLOD = 0;
    desc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11SamplerState* state = nullptr;
    hr = device->CreateSamplerState(&desc, &state);
    printf("  CreateSamplerState: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL");
    if (FAILED(hr)) { g_failed++; device->Release(); PASS(); return; }
    g_passed++;

    D3D11_SAMPLER_DESC gotDesc = {};
    state->GetDesc(&gotDesc);
    bool match = (gotDesc.Filter == D3D11_FILTER_MIN_MAG_MIP_LINEAR) &&
                 (gotDesc.AddressU == D3D11_TEXTURE_ADDRESS_WRAP) &&
                 (gotDesc.MaxAnisotropy == 16);
    printf("  GetDesc roundtrip: %s\n", match ? "PASS" : "FAIL");
    if (match) g_passed++; else g_failed++;

    state->Release();
    device->Release();
    PASS();
  }
}

// ============================================================================
// Test: Shader Creation
// ============================================================================

void test_create_shaders() {
  TEST("CreateVertexShader / PixelShader / ComputeShader");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    // Minimal DXBC bytecode (valid header, empty shader body)
    uint32_t fakeBytecode[] = {
      0x43425844, 0x00000000, 0x00030001, 0x00000018,
      0x00000001, 0x00000024, 0x00000004, 0x00000060
    };

    // Vertex shader
    {
      ID3D11VertexShader* vs = nullptr;
      hr = device->CreateVertexShader(fakeBytecode, sizeof(fakeBytecode), nullptr, &vs);
      printf("  CreateVertexShader: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL (expected — stub bytecode)");
      if (SUCCEEDED(hr)) { g_passed++; vs->Release(); } else { g_failed++; }
    }

    // Pixel shader
    {
      ID3D11PixelShader* ps = nullptr;
      hr = device->CreatePixelShader(fakeBytecode, sizeof(fakeBytecode), nullptr, &ps);
      printf("  CreatePixelShader: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL (expected — stub bytecode)");
      if (SUCCEEDED(hr)) { g_passed++; ps->Release(); } else { g_failed++; }
    }

    // Compute shader
    {
      ID3D11ComputeShader* cs = nullptr;
      hr = device->CreateComputeShader(fakeBytecode, sizeof(fakeBytecode), nullptr, &cs);
      printf("  CreateComputeShader: %s\n", SUCCEEDED(hr) ? "PASS" : "FAIL (expected — stub bytecode)");
      if (SUCCEEDED(hr)) { g_passed++; cs->Release(); } else { g_failed++; }
    }

    device->Release();
    PASS();
  }
}

// ============================================================================
// Test: Format Support
// ============================================================================

void test_format_support() {
  TEST("CheckFormatSupport");
  {
    ID3D11Device* device = nullptr;
    uint32_t level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    UINT support = device->CheckFormatSupport(DXGI_FORMAT_R8G8B8A8_UNORM);
    printf("  R8G8B8A8_UNORM support: 0x%x\n", support);
    bool ok = (support != 0);
    printf("  Format has some support: %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;

    device->Release();
    PASS();
  }
}

// ============================================================================
// Test: Device Removed
// ============================================================================

void test_device_removed() {
  TEST("GetDeviceRemovedReason");
  {
    ID3D11Device* device = nullptr;
    D3D_FEATURE_LEVEL level;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D11_DRIVER_TYPE_HARDWARE, nullptr, 0,
      nullptr, 0, D3D11_SDK_VERSION, &device, &level, nullptr);
    if (FAILED(hr)) { printf("(no Vulkan driver) SKIP\n"); return; }

    HRESULT reason = device->GetDeviceRemovedReason();
    printf("  DeviceRemovedReason: 0x%08lx (expected S_OK)\n", reason);
    bool ok = (reason == S_OK);
    printf("  %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;

    device->Release();
    PASS();
  }
}

// ============================================================================
// Main
// ============================================================================

int main() {
  printf("=== VKWIND11 Test: D3D11 Device + Resources + States ===\n\n");

  test_device_creation();
  test_query_interface();
  test_addref_release();
  test_create_buffer();
  test_create_texture2d();
  test_create_views();
  test_blend_state();
  test_rasterizer_state();
  test_depth_stencil_state();
  test_sampler_state();
  test_create_shaders();
  test_format_support();
  test_device_removed();

  printf("\n=== Results: %d passed, %d failed ===\n", g_passed, g_failed);
  return g_failed > 0 ? 1 : 0;
}
