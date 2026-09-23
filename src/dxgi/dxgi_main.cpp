// ============================================================================
// Standalone dxgi.dll — exports CreateDXGIFactory/CreateDXGIFactory1/2
// Minimal self-contained DXGI implementation for Winlator.
// No dependency on d3d11_device.h — fully standalone.
// ============================================================================

#include <atomic>
#include <cstring>
#include <cstdlib>
#include "../d3d11/d3d11_interfaces.h"
#include "../common/logging.h"

// ============================================================================
// DXGIAdapter
// ============================================================================

class DXGIAdapterImpl : public IDXGIAdapter {
public:
  DXGIAdapterImpl() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IDXGIAdapter || riid == IID_IDXGIObject || riid == IID_IUnknown) {
      AddRef();
      *ppvObject = static_cast<IDXGIAdapter*>(this);
      return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetParent(REFIID, void** ppParent) override {
    if (ppParent) *ppParent = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_ADAPTER_DESC* pDesc) override {
    if (!pDesc) return E_POINTER;
    memset(pDesc, 0, sizeof(DXGI_ADAPTER_DESC));
    const wchar_t desc[] = L"VKWIND11 Vulkan Adapter";
    memcpy(pDesc->Description, desc, sizeof(desc));
    pDesc->VendorId = 0x10DE;
    pDesc->DeviceId = 0x0000;
    pDesc->SubSysId = 0;
    pDesc->Revision = 0;
    pDesc->DedicatedVideoMemory = 256 * 1024 * 1024;
    pDesc->DedicatedSystemMemory = 256 * 1024 * 1024;
    pDesc->SharedSystemMemory = 512 * 1024 * 1024;
    pDesc->Luid.LowPart = 0;
    pDesc->Luid.HighPart = 0;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE EnumOutputs(UINT, void** ppOutput) override {
    if (ppOutput) *ppOutput = nullptr;
    return static_cast<HRESULT>(DXGI_ERROR_NOT_FOUND);
  }

  HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFIID, LARGE_INTEGER* pUMDVersion) override {
    if (pUMDVersion) pUMDVersion->QuadPart = 0;
    return E_FAIL;
  }
};

// ============================================================================
// Forward declaration
// ============================================================================

class DXGIFactoryImpl;

// ============================================================================
// DXGISwapChain (standalone — no D3D11 device dependency)
// ============================================================================

class DXGISwapChainImpl : public IDXGISwapChain1 {
public:
  DXGISwapChainImpl(IDXGIFactory* factory, const DXGI_SWAP_CHAIN_DESC* desc)
    : m_factory(factory), m_desc(*desc) {
    if (m_factory) m_factory->AddRef();
    m_width = desc->BufferDesc.Width;
    m_height = desc->BufferDesc.Height;
    m_format = desc->BufferDesc.Format;
    m_bufferCount = desc->BufferCount;
    if (m_bufferCount == 0) m_bufferCount = 1;
  }

  ~DXGISwapChainImpl() override {
    if (m_factory) m_factory->Release();
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IDXGISwapChain || riid == IID_IDXGISwapChain1 ||
        riid == IID_IDXGIDeviceSubObject || riid == IID_IDXGIObject || riid == IID_IUnknown) {
      AddRef();
      *ppvObject = static_cast<IDXGISwapChain1*>(this);
      return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return S_OK; }

  HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void** ppParent) override {
    if (!ppParent) return E_POINTER;
    if (riid == IID_IDXGIFactory) {
      *ppParent = m_factory;
      if (m_factory) m_factory->AddRef();
      return S_OK;
    }
    *ppParent = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE Present(UINT, UINT) override {
    m_frameCount++;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID, void** ppSurface) override {
    if (!ppSurface) return E_POINTER;
    *ppSurface = nullptr;
    if (Buffer >= m_bufferCount) return E_INVALIDARG;
    return E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE SetFullscreenState(BOOL Fullscreen, void*) override {
    m_fullscreen = Fullscreen;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetFullscreenState(BOOL* pFullscreen, void** ppTarget) override {
    if (pFullscreen) *pFullscreen = m_fullscreen;
    if (ppTarget) *ppTarget = nullptr;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) override {
    if (!pDesc) return E_POINTER;
    *pDesc = m_desc;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT) override {
    if (Width > 0) m_width = Width;
    if (Height > 0) m_height = Height;
    if (BufferCount > 0) m_bufferCount = BufferCount;
    if (NewFormat != DXGI_FORMAT_UNKNOWN) m_format = NewFormat;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) override {
    if (pNewTargetParameters) {
      if (pNewTargetParameters->Width > 0) m_width = pNewTargetParameters->Width;
      if (pNewTargetParameters->Height > 0) m_height = pNewTargetParameters->Height;
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetContainingOutput(void** ppOutput) override {
    if (ppOutput) *ppOutput = nullptr;
    return E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE GetFrameStatistics(void*) override { return E_FAIL; }

  HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount) override {
    if (pLastPresentCount) *pLastPresentCount = m_frameCount;
    return S_OK;
  }

  // IDXGISwapChain1
  HRESULT STDMETHODCALLTYPE GetDesc1(DXGI_SWAP_CHAIN_DESC1* pDesc) override {
    if (!pDesc) return E_POINTER;
    memset(pDesc, 0, sizeof(DXGI_SWAP_CHAIN_DESC1));
    pDesc->Width = m_width;
    pDesc->Height = m_height;
    pDesc->Format = m_format;
    pDesc->Stereo = FALSE;
    pDesc->SampleDesc.Count = 1;
    pDesc->SampleDesc.Quality = 0;
    pDesc->BufferUsage = m_desc.BufferUsage;
    pDesc->BufferCount = m_bufferCount;
    pDesc->Scaling = 0;
    pDesc->SwapEffect = m_desc.SwapEffect;
    pDesc->AlphaMode = 0;
    pDesc->Flags = m_desc.Flags;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetFullscreenDesc(DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pDesc) override {
    if (!pDesc) return E_POINTER;
    memset(pDesc, 0, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
    pDesc->Windowed = !m_fullscreen;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetHwnd(HWND* pHwnd) override {
    if (pHwnd) *pHwnd = m_desc.OutputWindow;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetCoreWindow(REFIID, void** ppCoreWindow) override {
    if (ppCoreWindow) *ppCoreWindow = nullptr;
    return E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE Present1(UINT SyncInterval, UINT Flags, const void*) override {
    return Present(SyncInterval, Flags);
  }

  BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported() override { return FALSE; }
  HRESULT STDMETHODCALLTYPE GetRestrictToOutput(void** pp) override { if (pp) *pp = nullptr; return E_FAIL; }
  HRESULT STDMETHODCALLTYPE SetBackgroundColor(const float*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetBackgroundColor(float* pColor) override {
    if (pColor) { pColor[0] = 0; pColor[1] = 0; pColor[2] = 0; pColor[3] = 1; }
    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION* pRot) override {
    if (pRot) *pRot = DXGI_MODE_ROTATION_IDENTITY;
    return S_OK;
  }

private:
  IDXGIFactory* m_factory = nullptr;
  DXGI_SWAP_CHAIN_DESC m_desc = {};
  UINT m_width = 0;
  UINT m_height = 0;
  DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
  UINT m_bufferCount = 1;
  BOOL m_fullscreen = FALSE;
  UINT m_frameCount = 0;
};

// ============================================================================
// DXGIFactory
// ============================================================================

class DXGIFactoryImpl : public IDXGIFactory {
public:
  DXGIFactoryImpl() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IDXGIFactory || riid == IID_IDXGIObject || riid == IID_IUnknown) {
      AddRef();
      *ppvObject = static_cast<IDXGIFactory*>(this);
      return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetParent(REFIID, void** ppParent) override {
    if (ppParent) *ppParent = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) override {
    if (!ppAdapter) return E_POINTER;
    *ppAdapter = nullptr;
    if (Adapter > 0) return static_cast<HRESULT>(DXGI_ERROR_NOT_FOUND);
    *ppAdapter = new DXGIAdapterImpl();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND, UINT Flags) override {
    VKWIND11_LOG_TRACE("MakeWindowAssociation: flags=0x%x", Flags);
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND* pWindowHandle) override {
    if (pWindowHandle) *pWindowHandle = nullptr;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE CreateSwapChain(void* pDevice, const DXGI_SWAP_CHAIN_DESC* pDesc, void** ppSwapChain) override {
    if (!pDesc || !ppSwapChain) return E_INVALIDARG;
    *ppSwapChain = nullptr;
    auto* swapchain = new DXGISwapChainImpl(this, pDesc);
    *ppSwapChain = swapchain;
    return S_OK;
  }
};

// ============================================================================
// DXGI Exports
// ============================================================================

extern "C" {

HRESULT CreateDXGIFactory(REFIID riid, void** ppFactory) {
  logInitFromEnv();
  VKWIND11_LOG_INFO("CreateDXGIFactory called");
  if (!ppFactory) return E_INVALIDARG;
  *ppFactory = nullptr;

  auto* factory = new DXGIFactoryImpl();
  HRESULT hr = factory->QueryInterface(riid, ppFactory);
  factory->Release();
  return hr;
}

HRESULT CreateDXGIFactory1(REFIID riid, void** ppFactory) {
  logInitFromEnv();
  VKWIND11_LOG_INFO("CreateDXGIFactory1 called");
  return CreateDXGIFactory(riid, ppFactory);
}

HRESULT CreateDXGIFactory2(UINT, REFIID riid, void** ppFactory) {
  logInitFromEnv();
  VKWIND11_LOG_INFO("CreateDXGIFactory2 called");
  return CreateDXGIFactory(riid, ppFactory);
}

} // extern "C"
