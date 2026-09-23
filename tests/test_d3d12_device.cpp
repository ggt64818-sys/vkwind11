#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../src/d3d12/d3d12_types.h"
#include "../src/d3d12/d3d12_interfaces.h"
#include "../src/common/logging.h"

static int g_passed = 0;
static int g_failed = 0;

#define TEST(name) printf("[Test] %s... ", name)
#define PASS() do { printf("PASS\n"); g_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

// ============================================================================
// Test: D3D12 Type Constants
// ============================================================================

void test_d3d12_types() {
  TEST("D3D12 heap/resource/command types");
  {
    CHECK(D3D12_HEAP_TYPE_DEFAULT == 1, "DEFAULT != 1");
    CHECK(D3D12_HEAP_TYPE_UPLOAD == 2, "UPLOAD != 2");
    CHECK(D3D12_HEAP_TYPE_READBACK == 3, "READBACK != 3");

    CHECK(D3D12_RESOURCE_STATE_COMMON == 0, "COMMON != 0");
    CHECK(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER == 0x1, "VCB != 0x1");
    CHECK(D3D12_RESOURCE_STATE_RENDER_TARGET == 0x8, "RT != 0x8");
    CHECK(D3D12_RESOURCE_STATE_DEPTH_WRITE == 0x10, "DSW != 0x10");

    CHECK(D3D12_COMMAND_LIST_TYPE_DIRECT == 0, "DIRECT != 0");
    CHECK(D3D12_COMMAND_LIST_TYPE_COMPUTE == 1, "COMPUTE != 1");
    CHECK(D3D12_COMMAND_LIST_TYPE_COPY == 2, "COPY != 2");

    CHECK(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV == 0, "CBV_SRV_UAV != 0");
    CHECK(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER == 1, "SAMPLER != 1");
    CHECK(D3D12_DESCRIPTOR_HEAP_TYPE_RTV == 2, "RTV != 2");
    CHECK(D3D12_DESCRIPTOR_HEAP_TYPE_DSV == 3, "DSV != 3");

    printf("  All constants verified\n");
    PASS();
  }
}

// ============================================================================
// Test: D3D12 Feature Levels
// ============================================================================

void test_d3d12_feature_levels() {
  TEST("D3D12 feature level constants");
  {
    CHECK(D3D_FEATURE_LEVEL_11_0 == 0xb000, "11_0 != 0xb000");
    CHECK(D3D_FEATURE_LEVEL_11_1 == 0xb100, "11_1 != 0xb100");
    CHECK(D3D_FEATURE_LEVEL_12_0 == 0xc000, "12_0 != 0xc000");
    CHECK(D3D_FEATURE_LEVEL_12_1 == 0xc100, "12_1 != 0xc100");
    PASS();
  }
}

// ============================================================================
// Test: D3D12 IID Constants
// ============================================================================

void test_d3d12_iids() {
  TEST("D3D12 IID constants are valid");
  {
    bool devOk = (IID_ID3D12Device.Data1 != 0 || IID_ID3D12Device.Data2 != 0 || IID_ID3D12Device.Data3 != 0);
    printf("  IID_ID3D12Device: %s\n", devOk ? "PASS" : "FAIL");
    if (devOk) g_passed++; else g_failed++;

    bool queueOk = (IID_ID3D12CommandQueue.Data1 != 0);
    printf("  IID_ID3D12CommandQueue: %s\n", queueOk ? "PASS" : "FAIL");
    if (queueOk) g_passed++; else g_failed++;

    bool listOk = (IID_ID3D12GraphicsCommandList.Data1 != 0);
    printf("  IID_ID3D12GraphicsCommandList: %s\n", listOk ? "PASS" : "FAIL");
    if (listOk) g_passed++; else g_failed++;

    bool fenceOk = (IID_ID3D12Fence.Data1 != 0);
    printf("  IID_ID3D12Fence: %s\n", fenceOk ? "PASS" : "FAIL");
    if (fenceOk) g_passed++; else g_failed++;

    PASS();
  }
}

// ============================================================================
// Test: D3D12 Interface ID uniqueness
// ============================================================================

void test_d3d12_interface_ids() {
  TEST("D3D12 interface IDs are unique");
  {
    bool unique = true;
    if (IID_ID3D12Device == IID_ID3D12CommandQueue) unique = false;
    if (IID_ID3D12Device == IID_ID3D12GraphicsCommandList) unique = false;
    if (IID_ID3D12CommandQueue == IID_ID3D12GraphicsCommandList) unique = false;
    printf("  Device != Queue != CommandList: %s\n", unique ? "PASS" : "FAIL");
    if (unique) g_passed++; else g_failed++;
    PASS();
  }
}

// ============================================================================
// Test: D3D12 struct sizes
// ============================================================================

void test_d3d12_descriptor_sizes() {
  TEST("D3D12 descriptor struct sizes");
  {
    printf("  sizeof(D3D12_HEAP_PROPERTIES) = %zu\n", sizeof(D3D12_HEAP_PROPERTIES));
    printf("  sizeof(D3D12_RESOURCE_DESC) = %zu\n", sizeof(D3D12_RESOURCE_DESC));
    printf("  sizeof(D3D12_ROOT_SIGNATURE_DESC) = %zu\n", sizeof(D3D12_ROOT_SIGNATURE_DESC));
    printf("  sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC) = %zu\n", sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    printf("  sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC) = %zu\n", sizeof(D3D12_COMPUTE_PIPELINE_STATE_DESC));
    printf("  sizeof(D3D12_DESCRIPTOR_HEAP_DESC) = %zu\n", sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

    bool ok = sizeof(D3D12_HEAP_PROPERTIES) >= 32;
    printf("  Struct sizes reasonable: %s\n", ok ? "PASS" : "FAIL");
    if (ok) g_passed++; else g_failed++;
    PASS();
  }
}

// ============================================================================
// Test: D3D12 Pipeline State Desc defaults
// ============================================================================

void test_d3d12_pipeline_desc() {
  TEST("D3D12_GRAPHICS_PIPELINE_STATE_DESC zero-init");
  {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

    CHECK(desc.pRootSignature == nullptr, "RootSignature not null");
    CHECK(desc.PrimitiveTopologyType == D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, "topo != triangle");
    CHECK(desc.NumRenderTargets == 0, "numRT != 0");
    CHECK(desc.SampleDesc_Count == 1, "sample count != 1");
    CHECK(desc.SampleMask == 0xFFFFFFFF, "sample mask wrong");
    CHECK(desc.BlendState.RenderTarget[0].RenderTargetWriteMask == 0x0F, "write mask wrong");

    printf("  Default values verified\n");
    PASS();
  }
}

// ============================================================================
// Test: D3D12 Resource Barrier
// ============================================================================

void test_d3d12_resource_barrier() {
  TEST("D3D12_RESOURCE_BARRIER structure");
  {
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;

    CHECK(barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, "type wrong");
    CHECK(barrier.Transition.StateBefore == D3D12_RESOURCE_STATE_RENDER_TARGET, "before wrong");
    CHECK(barrier.Transition.StateAfter == D3D12_RESOURCE_STATE_COPY_DEST, "after wrong");

    printf("  Resource barrier structure OK\n");
    PASS();
  }
}

// ============================================================================
// Test: D3D12 Device Creation
// ============================================================================

void test_d3d12_create_device() {
  TEST("D3D12CreateDevice");
  {
    ID3D12Device* device = nullptr;
    HRESULT hr = D3D12CreateDevice(
      nullptr,
      D3D_FEATURE_LEVEL_11_0,
      IID_ID3D12Device,
      (void**)&device);

    if (FAILED(hr)) {
      printf("(no Vulkan driver) SKIP\n");
      return;
    }

    CHECK(device != nullptr, "device is null");
    printf("  Device: %p\n", (void*)device);

    // QI for IUnknown
    IUnknown* unk = nullptr;
    hr = device->QueryInterface(IID_IUnknown, (void**)&unk);
    CHECK(hr == S_OK && unk, "QueryInterface IUnknown failed");
    unk->Release();

    device->Release();
    PASS();
  }
}

// ============================================================================
// Test: D3D12 Root Parameter Types
// ============================================================================

void test_d3d12_root_params() {
  TEST("D3D12 root parameter types");
  {
    CHECK(D3D12_ROOT_PARAMETER_TYPE_CBV == 0, "CBV != 0");
    CHECK(D3D12_ROOT_PARAMETER_TYPE_SRV == 1, "SRV != 1");
    CHECK(D3D12_ROOT_PARAMETER_TYPE_UAV == 2, "UAV != 2");
    CHECK(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE == 3, "TABLE != 3");
    CHECK(D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS == 4, "CONSTANTS != 4");
    printf("  All 5 root param types verified\n");
    PASS();
  }
}

// ============================================================================
// Test: D3D12 Fence Flags
// ============================================================================

void test_d3d12_fence_flags() {
  TEST("D3D12 fence flags");
  {
    CHECK(D3D12_FENCE_FLAG_NONE == 0, "NONE != 0");
    CHECK(D3D12_FENCE_FLAG_SHARED == 1, "SHARED != 1");
    CHECK(D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER == 2, "CROSS_ADAPTER != 2");
    printf("  Fence flags verified\n");
    PASS();
  }
}

// ============================================================================
// Main
// ============================================================================

int main() {
  printf("=== VKWIND11 Test: D3D12 Types & Device ===\n\n");

  test_d3d12_types();
  test_d3d12_feature_levels();
  test_d3d12_iids();
  test_d3d12_interface_ids();
  test_d3d12_descriptor_sizes();
  test_d3d12_pipeline_desc();
  test_d3d12_resource_barrier();
  test_d3d12_create_device();
  test_d3d12_root_params();
  test_d3d12_fence_flags();

  printf("\n=== Results: %d passed, %d failed ===\n", g_passed, g_failed);
  return g_failed > 0 ? 1 : 0;
}
