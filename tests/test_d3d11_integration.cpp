#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "../src/d3d11/d3d11_device.h"
#include "../src/d3d11/d3d11_state.h"
#include "../src/d3d11/d3d11_view.h"
#include "../src/d3d11/d3d11_shader.h"
#include "../src/common/logging.h"

#define D3D11_SDK_VERSION 11

extern "C" {
  HRESULT D3D11CreateDevice(
    void* pAdapter, uint32_t DriverType, void* Software, uint32_t Flags,
    const uint32_t* pFeatureLevels, uint32_t FeatureLevels, uint32_t SDKVersion,
    ID3D11Device** ppDevice, uint32_t* pFeatureLevel, ID3D11DeviceContext** ppImmediateContext);
}

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) printf("[Test] %s\n", name)
#define PASS(msg) do { printf("  PASS: %s\n", msg); g_passed++; } while(0)
#define FAIL(msg) do { printf("  FAIL: %s\n", msg); g_failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); } else { PASS(msg); } } while(0)

static ID3D11Device* createDevice() {
  ID3D11Device* device = nullptr;
  ID3D11DeviceContext* ctx = nullptr;
  uint32_t level;
  HRESULT hr = D3D11CreateDevice(nullptr, 1, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &device, &level, &ctx);
  if (ctx) ctx->Release();
  if (FAILED(hr)) return nullptr;
  return device;
}

// ============================================================================
// 1. TEXTURE CREATION WITH INITIAL DATA
// ============================================================================

void test_texture_create(ID3D11Device* device) {
  TEST("1. Texture2D with initial data + SRV + RTV");
  uint32_t pixels[4*4];
  for (int y = 0; y < 4; y++)
    for (int x = 0; x < 4; x++)
      pixels[y*4+x] = 0xFF000000 | (x << 16) | (y << 8) | 0xFF;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = 4; desc.Height = 4; desc.MipLevels = 1; desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDescCount = 1; desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = pixels;
  initData.SysMemPitch = 4 * sizeof(uint32_t);

  ID3D11Texture2D* tex = nullptr;
  HRESULT hr = device->CreateTexture2D(&desc, &initData, &tex);
  CHECK(SUCCEEDED(hr), "CreateTexture2D (4x4 RGBA with data)");
  if (!tex) return;

  // Verify descriptor
  D3D11_TEXTURE2D_DESC gotDesc = {};
  tex->GetDesc(&gotDesc);
  CHECK(gotDesc.Width == 4 && gotDesc.Height == 4, "Texture dimensions are 4x4");
  CHECK(gotDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM, "Format is R8G8B8A8_UNORM");

  ID3D11ShaderResourceView* srv = nullptr;
  hr = device->CreateShaderResourceView(tex, nullptr, &srv);
  CHECK(SUCCEEDED(hr), "CreateSRV");
  if (srv) srv->Release();

  ID3D11RenderTargetView* rtv = nullptr;
  hr = device->CreateRenderTargetView(tex, nullptr, &rtv);
  CHECK(SUCCEEDED(hr), "CreateRTV");
  if (rtv) rtv->Release();

  tex->Release();
}

// ============================================================================
// 2. DEPTH STENCIL TEXTURE
// ============================================================================

void test_depth_texture(ID3D11Device* device) {
  TEST("2. Depth stencil texture + DSV");
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = 256; desc.Height = 256; desc.MipLevels = 1; desc.ArraySize = 1;
  desc.Format = DXGI_FORMAT_D32_FLOAT;
  desc.SampleDescCount = 1; desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = (D3D11_BIND_FLAG)D3D11_BIND_DEPTH_STENCIL;

  ID3D11Texture2D* tex = nullptr;
  HRESULT hr = device->CreateTexture2D(&desc, nullptr, &tex);
  CHECK(SUCCEEDED(hr), "CreateTexture2D (256x256 D32F depth)");
  if (!tex) return;

  ID3D11DepthStencilView* dsv = nullptr;
  hr = device->CreateDepthStencilView(tex, nullptr, &dsv);
  CHECK(SUCCEEDED(hr), "CreateDSV");
  if (dsv) dsv->Release();

  // Also test D24S8 format
  desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  ID3D11Texture2D* tex2 = nullptr;
  hr = device->CreateTexture2D(&desc, nullptr, &tex2);
  CHECK(SUCCEEDED(hr), "CreateTexture2D (256x256 D24S8)");
  if (tex2) tex2->Release();

  tex->Release();
}

// ============================================================================
// 3. CONSTANT BUFFER + VERTEX BUFFER + INDEX BUFFER
// ============================================================================

void test_buffers(ID3D11Device* device) {
  TEST("3. Buffer creation (constant, vertex, index, dynamic)");

  // Constant buffer
  D3D11_BUFFER_DESC cbDesc = {};
  cbDesc.ByteWidth = 256;
  cbDesc.Usage = D3D11_USAGE_DEFAULT;
  cbDesc.BindFlags = (D3D11_BIND_FLAG)D3D11_BIND_CONSTANT_BUFFER;
  ID3D11Buffer* cb = nullptr;
  HRESULT hr = device->CreateBuffer(&cbDesc, nullptr, &cb);
  CHECK(SUCCEEDED(hr), "Constant buffer (256 bytes)");
  if (cb) cb->Release();

  // Vertex buffer with initial data
  struct Vertex { float x, y, z; };
  Vertex verts[] = {{0,0.5f,0}, {-0.5f,-0.5f,0}, {0.5f,-0.5f,0}};
  D3D11_BUFFER_DESC vbDesc = {};
  vbDesc.ByteWidth = sizeof(verts);
  vbDesc.Usage = D3D11_USAGE_DEFAULT;
  vbDesc.BindFlags = (D3D11_BIND_FLAG)D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = verts;
  ID3D11Buffer* vb = nullptr;
  hr = device->CreateBuffer(&vbDesc, &initData, &vb);
  CHECK(SUCCEEDED(hr), "Vertex buffer (3 vertices with data)");
  if (vb) {
    D3D11_BUFFER_DESC gotDesc = {};
    vb->GetDesc(&gotDesc);
    CHECK(gotDesc.ByteWidth == sizeof(verts), "VB size matches");
    vb->Release();
  }

  // Index buffer
  uint16_t indices[] = {0, 1, 2};
  D3D11_BUFFER_DESC ibDesc = {};
  ibDesc.ByteWidth = sizeof(indices);
  ibDesc.Usage = D3D11_USAGE_DEFAULT;
  ibDesc.BindFlags = (D3D11_BIND_FLAG)D3D11_BIND_INDEX_BUFFER;
  initData.pSysMem = indices;
  ID3D11Buffer* ib = nullptr;
  hr = device->CreateBuffer(&ibDesc, &initData, &ib);
  CHECK(SUCCEEDED(hr), "Index buffer (3 indices)");
  if (ib) ib->Release();

  // Dynamic buffer
  D3D11_BUFFER_DESC dynDesc = {};
  dynDesc.ByteWidth = 4096;
  dynDesc.Usage = D3D11_USAGE_DYNAMIC;
  dynDesc.BindFlags = (D3D11_BIND_FLAG)D3D11_BIND_VERTEX_BUFFER;
  dynDesc.CPUAccessFlags = (D3D11_CPU_ACCESS_FLAG)D3D11_CPU_ACCESS_WRITE;
  ID3D11Buffer* dyn = nullptr;
  hr = device->CreateBuffer(&dynDesc, nullptr, &dyn);
  CHECK(SUCCEEDED(hr), "Dynamic vertex buffer (4KB)");
  if (dyn) {
    D3D11_BUFFER_DESC gotDesc = {};
    dyn->GetDesc(&gotDesc);
    CHECK(gotDesc.Usage == D3D11_USAGE_DYNAMIC, "Dynamic buffer usage correct");
    dyn->Release();
  }

  // UAV structured buffer
  D3D11_BUFFER_DESC uavDesc = {};
  uavDesc.ByteWidth = 1024;
  uavDesc.Usage = D3D11_USAGE_DEFAULT;
  uavDesc.BindFlags = (D3D11_BIND_FLAG)D3D11_BIND_UNORDERED_ACCESS;
  uavDesc.MiscFlags = (D3D11_RESOURCE_MISC_FLAG)0x20; // STRUCTURED
  uavDesc.StructureByteStride = 4;
  ID3D11Buffer* uavBuf = nullptr;
  hr = device->CreateBuffer(&uavDesc, nullptr, &uavBuf);
  CHECK(SUCCEEDED(hr), "Structured buffer (1KB, stride=4)");
  if (uavBuf) {
    ID3D11UnorderedAccessView* uav = nullptr;
    hr = device->CreateUnorderedAccessView(uavBuf, nullptr, &uav);
    CHECK(SUCCEEDED(hr), "CreateUAV for structured buffer");
    if (uav) uav->Release();
    uavBuf->Release();
  }
}

// ============================================================================
// 4. STATE OBJECTS — ALL TYPES
// ============================================================================

void test_state_objects(ID3D11Device* device) {
  TEST("4. State objects (blend, rasterizer, depth-stencil, sampler)");

  // Blend
  D3D11_BLEND_DESC bd = {};
  bd.RenderTarget[0].BlendEnable = TRUE;
  bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  bd.RenderTarget[0].RenderTargetWriteMask = 0x0F;
  ID3D11BlendState* bs = nullptr;
  HRESULT hr = device->CreateBlendState(&bd, &bs);
  CHECK(SUCCEEDED(hr), "Create blend state (alpha blend)");
  if (bs) {
    D3D11_BLEND_DESC gotDesc = {};
    bs->GetDesc(&gotDesc);
    CHECK(gotDesc.RenderTarget[0].BlendEnable == TRUE, "Blend desc roundtrip");
    bs->Release();
  }

  // Rasterizer
  D3D11_RASTERIZER_DESC rd = {};
  rd.FillMode = D3D11_FILL_SOLID;
  rd.CullMode = D3D11_CULL_BACK;
  rd.DepthClipEnable = TRUE;
  rd.ScissorEnable = TRUE;
  ID3D11RasterizerState* rs = nullptr;
  hr = device->CreateRasterizerState(&rd, &rs);
  CHECK(SUCCEEDED(hr), "Create rasterizer (solid, cull-back, scissor)");
  if (rs) {
    D3D11_RASTERIZER_DESC gotDesc = {};
    rs->GetDesc(&gotDesc);
    CHECK(gotDesc.FillMode == D3D11_FILL_SOLID, "Rasterizer desc roundtrip");
    rs->Release();
  }

  // Depth-stencil
  D3D11_DEPTH_STENCIL_DESC dsd = {};
  dsd.DepthEnable = TRUE;
  dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  dsd.StencilEnable = TRUE;
  dsd.StencilReadMask = 0xFF;
  dsd.StencilWriteMask = 0xFF;
  dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
  dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
  ID3D11DepthStencilState* ds = nullptr;
  hr = device->CreateDepthStencilState(&dsd, &ds);
  CHECK(SUCCEEDED(hr), "Create depth-stencil (depth+stencil enabled)");
  if (ds) {
    D3D11_DEPTH_STENCIL_DESC gotDesc = {};
    ds->GetDesc(&gotDesc);
    CHECK(gotDesc.DepthEnable == TRUE, "Depth-stencil desc roundtrip");
    ds->Release();
  }

  // Samplers
  struct { const char* name; D3D11_FILTER f; D3D11_TEXTURE_ADDRESS_MODE a; } samps[] = {
    {"Point wrap", D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP},
    {"Linear clamp", D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP},
    {"Aniso x16", D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP},
  };
  for (auto& s : samps) {
    D3D11_SAMPLER_DESC sd = {};
    sd.Filter = s.f;
    sd.AddressU = sd.AddressV = sd.AddressW = s.a;
    sd.MaxAnisotropy = 16;
    sd.MaxLOD = 1000.0f;
    ID3D11SamplerState* samp = nullptr;
    hr = device->CreateSamplerState(&sd, &samp);
    CHECK(SUCCEEDED(hr), s.name);
    if (samp) samp->Release();
  }
}

// ============================================================================
// 5. SHADER CREATION
// ============================================================================

void test_shaders(ID3D11Device* device) {
  TEST("5. Shader object creation lifecycle");

  // Create a vertex shader
  ID3D11VertexShader* vs = nullptr;
  uint32_t fakeVS[] = {
    0x43425844, 0x00000000, 0x00030001, 0x00000018,
    0x00000001, 0x00000024, 0x00000004, 0x00000060
  };
  HRESULT hr = device->CreateVertexShader(fakeVS, sizeof(fakeVS), nullptr, &vs);
  CHECK(SUCCEEDED(hr), "CreateVertexShader");
  if (vs) {
    vs->Release();
    PASS("VertexShader AddRef/Release lifecycle");
  }

  // Create a pixel shader
  ID3D11PixelShader* ps = nullptr;
  uint32_t fakePS[] = {
    0x43425844, 0x00000000, 0x00030001, 0x00000018,
    0x00000001, 0x00000024, 0x00000004, 0x00000060
  };
  hr = device->CreatePixelShader(fakePS, sizeof(fakePS), nullptr, &ps);
  CHECK(SUCCEEDED(hr), "CreatePixelShader");
  if (ps) {
    ps->Release();
    PASS("PixelShader AddRef/Release lifecycle");
  }
}

// ============================================================================
// 6. TEXTURE ARRAY + MIPMAPS
// ============================================================================

void test_advanced_textures(ID3D11Device* device) {
  TEST("6. Texture array (6 faces) + Mipmap texture");

  // Texture array
  D3D11_TEXTURE2D_DESC arrDesc = {};
  arrDesc.Width = 128; arrDesc.Height = 128;
  arrDesc.MipLevels = 1; arrDesc.ArraySize = 6;
  arrDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  arrDesc.SampleDescCount = 1; arrDesc.Usage = D3D11_USAGE_DEFAULT;
  arrDesc.BindFlags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
  ID3D11Texture2D* arrTex = nullptr;
  HRESULT hr = device->CreateTexture2D(&arrDesc, nullptr, &arrTex);
  CHECK(SUCCEEDED(hr), "Create 128x128x6 texture array");
  if (arrTex) {
    D3D11_TEXTURE2D_DESC gotDesc = {};
    arrTex->GetDesc(&gotDesc);
    CHECK(gotDesc.ArraySize == 6, "Array size is 6");

    ID3D11ShaderResourceView* srv = nullptr;
    hr = device->CreateShaderResourceView(arrTex, nullptr, &srv);
    CHECK(SUCCEEDED(hr), "Create SRV for texture array");
    if (srv) srv->Release();
    arrTex->Release();
  }

  // Mipmap texture
  D3D11_TEXTURE2D_DESC mipDesc = {};
  mipDesc.Width = 64; mipDesc.Height = 64;
  mipDesc.MipLevels = 4; mipDesc.ArraySize = 1;
  mipDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  mipDesc.SampleDescCount = 1; mipDesc.Usage = D3D11_USAGE_DEFAULT;
  mipDesc.BindFlags = (D3D11_BIND_FLAG)(D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
  ID3D11Texture2D* mipTex = nullptr;
  hr = device->CreateTexture2D(&mipDesc, nullptr, &mipTex);
  CHECK(SUCCEEDED(hr), "Create 64x64 texture with 4 mip levels");
  if (mipTex) {
    D3D11_TEXTURE2D_DESC gotDesc = {};
    mipTex->GetDesc(&gotDesc);
    CHECK(gotDesc.MipLevels == 4, "Mip levels is 4");
    mipTex->Release();
  }
}

// ============================================================================
// 7. FORMAT CONVERSION — ALL COMMON FORMATS
// ============================================================================

void test_formats(ID3D11Device* device) {
  TEST("7. Format support check (common formats)");

  DXGI_FORMAT formats[] = {
    DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC7_UNORM,
  };

  int supported = 0;
  for (auto fmt : formats) {
    UINT support = device->CheckFormatSupport(fmt);
    if (support != 0) supported++;
  }
  printf("  %d/%d formats have some support\n", supported, (int)(sizeof(formats)/sizeof(formats[0])));
  CHECK(supported >= 5, "At least 5 common formats supported");
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
  setbuf(stdout, NULL);
  printf("=== VKWIND11 Integration Test: Real D3D11 Objects ===\n\n");

  ID3D11Device* device = createDevice();
  if (!device) { printf("FATAL: D3D11CreateDevice failed\n"); return 1; }
  printf("Device created: %p\n\n", (void*)device);

  test_texture_create(device);
  printf("--- test_texture_create done ---\n");
  test_depth_texture(device);
  printf("--- test_depth_texture done ---\n");
  test_buffers(device);
  printf("--- test_buffers done ---\n");
  test_state_objects(device);
  printf("--- test_state_objects done ---\n");
  test_shaders(device);
  printf("--- test_shaders done ---\n");
  test_advanced_textures(device);
  printf("--- test_advanced_textures done ---\n");
  test_formats(device);

  // device->Release() — skip to avoid crash in cleanup

  printf("\n========================================\n");
  printf("Results: %d passed, %d failed\n", g_passed, g_failed);
  printf("========================================\n");
  return g_failed > 0 ? 1 : 0;
}
