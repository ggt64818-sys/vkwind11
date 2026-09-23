#include "d3d11_device.h"
#include "d3d11_context.h"
#include "../common/logging.h"
#include "../common/config.h"

// ============================================================================
// Platform-specific Win32 surface creation (manual definitions to avoid
// windows.h / vulkan_win32.h header conflicts with d3d11_types.h)
// ============================================================================

#if defined(_WIN32)
// GetModuleHandleW — imported from kernel32.dll at link time
extern "C" void* __stdcall GetModuleHandleW(const wchar_t* lpModuleName);
#define GetModuleHandle GetModuleHandleW

// VkWin32SurfaceCreateInfoKHR (identical layout to the Vulkan SDK definition)
struct VkWin32SurfaceCreateInfoKHR_manual {
  VkStructureType sType;
  const void* pNext;
  uint32_t flags;
  void* hinstance;
  void* hwnd;
};

typedef VkResult(VKAPI_CALL* PFN_vkCreateWin32SurfaceKHR_manual)(
  VkInstance, const VkWin32SurfaceCreateInfoKHR_manual*,
  const VkAllocationCallbacks*, VkSurfaceKHR*);
#endif

// ============================================================================
// D3D11CreateDevice
// ============================================================================

extern "C" {

HRESULT D3D11CreateDevice(
  void* pAdapter,            // IDXGIAdapter*
  D3D11_DRIVER_TYPE DriverType,
  void* Software,            // HMODULE
  UINT Flags,
  const uint32_t* pFeatureLevels,
  UINT FeatureLevels,
  UINT SDKVersion,
  ID3D11Device** ppDevice,
  D3D_FEATURE_LEVEL* pFeatureLevel,
  ID3D11DeviceContext** ppImmediateContext
) {
  logInitFromEnv();
  configInit();

  VKWIND11_LOG_INFO("D3D11CreateDevice called");
  VKWIND11_LOG_INFO("  DriverType: %d, Flags: 0x%x", DriverType, Flags);
  VKWIND11_LOG_INFO("  FeatureLevels: %u, SDKVersion: %u", FeatureLevels, SDKVersion);

  if (!ppDevice) return E_INVALIDARG;

  // Create Vulkan device
  auto device = new D3D11Device();
  if (!device->getVulkanDevice().initialize(false)) {
    VKWIND11_LOG_ERROR("Failed to initialize Vulkan device");
    delete device;
    return E_FAIL;
  }

  // Create immediate context
  auto context = new D3D11DeviceContext(device);

  // Store raw pointer via setter
  device->setImmediateContext(context);

  *ppDevice = device;
  if (ppImmediateContext) *ppImmediateContext = context;
  if (pFeatureLevel) *pFeatureLevel = D3D_FEATURE_LEVEL_11_0;

  VKWIND11_LOG_INFO("D3D11CreateDevice succeeded");
  return S_OK;
}

HRESULT D3D11CreateDeviceAndSwapChain(
  void* pAdapter,
  D3D11_DRIVER_TYPE DriverType,
  void* Software,
  UINT Flags,
  const uint32_t* pFeatureLevels,
  UINT FeatureLevels,
  UINT SDKVersion,
  const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
  ID3D11SwapChain** ppSwapChain,
  ID3D11Device** ppDevice,
  D3D_FEATURE_LEVEL* pFeatureLevel,
  ID3D11DeviceContext** ppImmediateContext
) {
  VKWIND11_LOG_INFO("D3D11CreateDeviceAndSwapChain called");

  // ── Step 1: Create device (identical to D3D11CreateDevice) ──────────────

  HRESULT hr = D3D11CreateDevice(pAdapter, DriverType, Software, Flags,
    pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

  if (FAILED(hr)) return hr;
  if (!pSwapChainDesc) return E_INVALIDARG;
  if (!ppSwapChain) return E_INVALIDARG;

  auto* device = static_cast<D3D11Device*>(*ppDevice);
  auto& vk = device->getVulkanDevice();

  uint32_t width  = pSwapChainDesc->BufferDesc.Width;
  uint32_t height = pSwapChainDesc->BufferDesc.Height;
  if (width == 0 || height == 0) {
    VKWIND11_LOG_ERROR("Swap chain width/height must be > 0");
    return E_INVALIDARG;
  }

  // ── Step 2: Create VkSurfaceKHR from HWND ──────────────────────────────

  VkSurfaceKHR surface = VK_NULL_HANDLE;

#if defined(_WIN32)
  {
    auto pfnCreate = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR_manual>(
      vkGetInstanceProcAddr(vk.getInstance(), "vkCreateWin32SurfaceKHR"));
    if (!pfnCreate) {
      VKWIND11_LOG_ERROR("vkCreateWin32SurfaceKHR not available");
      return E_FAIL;
    }

    VkWin32SurfaceCreateInfoKHR_manual sci{};
    sci.sType      = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance  = GetModuleHandle(nullptr);
    sci.hwnd       = pSwapChainDesc->OutputWindow;

    VkResult vr = pfnCreate(vk.getInstance(), &sci, nullptr, &surface);
    if (vr != VK_SUCCESS || surface == VK_NULL_HANDLE) {
      VKWIND11_LOG_ERROR("Failed to create VkSurfaceKHR: %d", vr);
      return E_FAIL;
    }
  }
#else
  VKWIND11_LOG_WARN("Platform surface creation not implemented — using VK_NULL_HANDLE");
#endif

  // ── Step 3: Create Vulkan swapchain (images + views populated here) ─────

  VulkanSwapchain swapchain;
  if (!vk.createSwapchain(surface, width, height, swapchain)) {
    VKWIND11_LOG_ERROR("VulkanDevice::createSwapchain failed");
    return E_FAIL;
  }

  VKWIND11_LOG_INFO("Vulkan swapchain created: %u images, format %d, %ux%u",
    (uint32_t)swapchain.images.size(), swapchain.format, width, height);

  // ── Step 4: Create default VkRenderPass ────────────────────────────────
  //    Single color attachment: loadOp=CLEAR, storeOp=STORE,
  //    finalLayout = PRESENT_SRC_KHR (ready for presentation).

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format         = swapchain.format;
  colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorRef{};
  colorRef.attachment = 0;
  colorRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments    = &colorRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo rpInfo{};
  rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rpInfo.attachmentCount = 1;
  rpInfo.pAttachments    = &colorAttachment;
  rpInfo.subpassCount    = 1;
  rpInfo.pSubpasses      = &subpass;
  rpInfo.dependencyCount = 1;
  rpInfo.pDependencies   = &dependency;

  VkRenderPass renderPass = vk.createRenderPass(rpInfo);
  if (renderPass == VK_NULL_HANDLE) {
    VKWIND11_LOG_ERROR("Failed to create VkRenderPass");
    return E_FAIL;
  }

  // ── Step 5: Create VkFramebuffer for each swapchain image view ──────────

  swapchain.framebuffers.resize(swapchain.views.size());
  for (size_t i = 0; i < swapchain.views.size(); ++i) {
    std::vector<VkImageView> attachments = { swapchain.views[i] };
    swapchain.framebuffers[i] = vk.createFramebuffer(renderPass, attachments, width, height);
    if (swapchain.framebuffers[i] == VK_NULL_HANDLE) {
      VKWIND11_LOG_ERROR("Failed to create framebuffer for image %zu", i);
      return E_FAIL;
    }
  }

  VKWIND11_LOG_INFO("Created %zu framebuffers", swapchain.framebuffers.size());

  // ── Step 6: Store Vulkan resources on the device ────────────────────────

  device->setSwapchain(std::move(swapchain));
  device->setRenderPass(renderPass);

  // ── Step 7: Create DXGI swap chain wrapper and return it ────────────────

  IDXGISwapChain1* swapChain = createSwapChainImpl(*ppDevice, pSwapChainDesc);
  if (!swapChain) {
    VKWIND11_LOG_ERROR("Failed to create DXGISwapChainImpl");
    return E_OUTOFMEMORY;
  }

  *ppSwapChain = reinterpret_cast<ID3D11SwapChain*>(swapChain);

  VKWIND11_LOG_INFO("D3D11CreateDeviceAndSwapChain succeeded");
  return S_OK;
}

} // extern "C"
