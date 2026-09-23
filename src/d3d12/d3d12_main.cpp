#include "d3d12_device.h"
#include "../common/logging.h"
#include "../common/config.h"
#include <chrono>

// ============================================================================
// D3D12 Entry Points
// ============================================================================

HRESULT D3D12CreateDevice(
  IUnknown* pAdapter,
  UINT Flags,
  REFIID riid,
  void** ppDevice
) {
  logInitFromEnv();
  configInit();

  VKWIND11_LOG_INFO("D3D12CreateDevice called");

  if (!ppDevice) return E_INVALIDARG;

  *ppDevice = nullptr;

  auto device = new D3D12Device();
  if (!device->getVulkanDevice().initialize(false)) {
    VKWIND11_LOG_ERROR("Failed to initialize Vulkan device for D3D12");
    delete device;
    return E_FAIL;
  }

  *ppDevice = device;
  VKWIND11_LOG_INFO("D3D12CreateDevice — device created successfully");
  return S_OK;
}

HRESULT D3D12GetDebugInterface(REFIID riid, void** ppDebug) {
  VKWIND11_LOG_INFO("D3D12GetDebugInterface called — debug not available");
  if (ppDebug) *ppDebug = nullptr;
  return E_NOTIMPL;
}

HRESULT D3D12EnableExperimentalFeatures(UINT NumFeatures, const IID* pIIDs, void* pConfigurationStructs, UINT* pEnabledFeatureOut) {
  VKWIND11_LOG_INFO("D3D12EnableExperimentalFeatures called — not implemented");
  return E_NOTIMPL;
}
