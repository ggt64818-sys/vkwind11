#include "d3d11_device.h"
#include "../common/logging.h"

// ============================================================================
// DXGI Forward Declarations
// ============================================================================

class DXGIAdapterImpl;
class DXGIDeviceImpl;
class DXGIFactoryImpl;
class DXGISwapChainImpl;

// ============================================================================
// DXGIAdapter
// ============================================================================

class DXGIAdapterImpl : public IDXGIAdapter {
public:
  DXGIAdapterImpl() = default;
  ~DXGIAdapterImpl() = default;

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
    if (!ppParent) return E_POINTER;
    *ppParent = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE EnumOutputs(UINT, void** ppOutput) override {
    if (ppOutput) *ppOutput = nullptr;
    return static_cast<HRESULT>(DXGI_ERROR_NOT_FOUND);
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

  HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFIID, LARGE_INTEGER* pUMDVersion) override {
    if (pUMDVersion) {
      pUMDVersion->QuadPart = 0;
    }
    return E_FAIL;
  }
};

// ============================================================================
// DXGIFactory (declared before DXGISwapChainImpl so it can be referenced)
// ============================================================================

class DXGIFactoryImpl : public IDXGIFactory {
public:
  DXGIFactoryImpl() = default;
  ~DXGIFactoryImpl() = default;

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
    if (!ppParent) return E_POINTER;
    *ppParent = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) override;
  HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND WindowHandle, UINT Flags) override;

  HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND* pWindowHandle) override {
    if (pWindowHandle) *pWindowHandle = nullptr;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE CreateSwapChain(void* pDevice, const DXGI_SWAP_CHAIN_DESC* pDesc, void** ppSwapChain) override;

  ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refCount; }
  ULONG STDMETHODCALLTYPE Release() override {
    ULONG count = --m_refCount;
    if (count == 0) delete this;
    return count;
  }

private:
  std::atomic<ULONG> m_refCount{1};
};

// ============================================================================
// DXGISwapChain
// ============================================================================

class DXGISwapChainImpl : public IDXGISwapChain1 {
public:
  DXGISwapChainImpl(IDXGIFactory* factory, ID3D11Device* device, const DXGI_SWAP_CHAIN_DESC* desc)
    : m_factory(factory), m_device(device), m_desc(*desc) {
    if (m_factory) m_factory->AddRef();
    if (m_device) m_device->AddRef();
    m_width = desc->BufferDesc.Width;
    m_height = desc->BufferDesc.Height;
    m_format = desc->BufferDesc.Format;
    m_bufferCount = desc->BufferCount;
    if (m_bufferCount == 0) m_bufferCount = 1;
  }

  ~DXGISwapChainImpl() override {
    if (m_factory) m_factory->Release();
    if (m_device) m_device->Release();
  }

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IDXGISwapChain || riid == IID_IDXGISwapChain1 || riid == IID_IDXGIDeviceSubObject || riid == IID_IDXGIObject || riid == IID_IUnknown) {
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

  HRESULT STDMETHODCALLTYPE GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) override {
    if (!ppSurface) return E_POINTER;
    *ppSurface = nullptr;
    if (Buffer >= m_bufferCount) return E_INVALIDARG;
    if (!m_device) return E_FAIL;
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = m_width;
    texDesc.Height = m_height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = m_format;
    texDesc.SampleDescCount = 1;
    texDesc.SampleDescQuality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    auto* texture = new D3D11Texture2DImpl(static_cast<D3D11Device*>(m_device), &texDesc, nullptr);
    *ppSurface = texture;
    return S_OK;
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

  HRESULT STDMETHODCALLTYPE GetFrameStatistics(void*) override {
    return E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE GetLastPresentCount(UINT* pLastPresentCount) override {
    if (pLastPresentCount) *pLastPresentCount = m_frameCount;
    return S_OK;
  }

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
  HRESULT STDMETHODCALLTYPE GetRestrictToOutput(void** ppRestrictToOutput) override { if (ppRestrictToOutput) *ppRestrictToOutput = nullptr; return E_FAIL; }
  HRESULT STDMETHODCALLTYPE SetBackgroundColor(const float*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetBackgroundColor(float* pColor) override { if (pColor) { pColor[0] = 0; pColor[1] = 0; pColor[2] = 0; pColor[3] = 1; } return S_OK; }
  HRESULT STDMETHODCALLTYPE SetRotation(DXGI_MODE_ROTATION) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetRotation(DXGI_MODE_ROTATION* pRotation) override { if (pRotation) *pRotation = DXGI_MODE_ROTATION_IDENTITY; return S_OK; }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refCount; }
  ULONG STDMETHODCALLTYPE Release() override {
    ULONG count = --m_refCount;
    if (count == 0) delete this;
    return count;
  }

private:
  std::atomic<ULONG> m_refCount{1};
  IDXGIFactory* m_factory;
  ID3D11Device* m_device;
  DXGI_SWAP_CHAIN_DESC m_desc;
  UINT m_width;
  UINT m_height;
  DXGI_FORMAT m_format;
  UINT m_bufferCount;
  BOOL m_fullscreen = FALSE;
  UINT m_frameCount = 0;
};

// ============================================================================
// DXGIDevice
// ============================================================================

class DXGIDeviceImpl : public IDXGIDevice1 {
public:
  explicit DXGIDeviceImpl(D3D11Device* device) : m_device(device) {}
  ~DXGIDeviceImpl() = default;

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;
    if (riid == IID_IDXGIDevice || riid == IID_IDXGIDevice1 || riid == IID_IDXGIObject || riid == IID_IUnknown) {
      AddRef();
      *ppvObject = static_cast<IDXGIDevice1*>(this);
      return S_OK;
    }
    if (riid == IID_IDXGIAdapter) {
      AddRef();
      *ppvObject = static_cast<IDXGIAdapter*>(&m_adapter);
      return S_OK;
    }
    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID, UINT, const void*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID, const IUnknown*) override { return S_OK; }
  HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID, UINT*, void*) override { return S_OK; }

  HRESULT STDMETHODCALLTYPE GetParent(REFIID, void** ppParent) override {
    if (!ppParent) return E_POINTER;
    *ppParent = nullptr;
    return E_NOINTERFACE;
  }

  HRESULT STDMETHODCALLTYPE GetAdapter(IDXGIAdapter** pAdapter) override {
    if (!pAdapter) return E_POINTER;
    *pAdapter = &m_adapter;
    m_adapter.AddRef();
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE CreateSurface(const void*, UINT, DXGI_USAGE, const void*, void** ppSurface) override {
    if (ppSurface) *ppSurface = nullptr;
    return E_FAIL;
  }

  HRESULT STDMETHODCALLTYPE QueryResourceResidency(IUnknown* const*, void*, UINT) override {
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE SetGPUThreadPriority(UINT) override {
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetGPUThreadPriority(UINT* pPriority) override {
    if (pPriority) *pPriority = 0;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE SetMaximumFrameLatency(UINT MaxLatency) override {
    m_maxFrameLatency = MaxLatency;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetMaximumFrameLatency(UINT* pMaxLatency) override {
    if (pMaxLatency) *pMaxLatency = m_maxFrameLatency;
    return S_OK;
  }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refCount; }
  ULONG STDMETHODCALLTYPE Release() override {
    ULONG count = --m_refCount;
    if (count == 0) delete this;
    return count;
  }

private:
  std::atomic<ULONG> m_refCount{1};
  D3D11Device* m_device;
  DXGIAdapterImpl m_adapter;
  UINT m_maxFrameLatency = 1;
};

// ============================================================================
// DXGIFactory (out-of-class definitions)
// ============================================================================

HRESULT DXGIFactoryImpl::EnumAdapters(UINT Adapter, IDXGIAdapter** ppAdapter) {
  if (!ppAdapter) return E_POINTER;
  *ppAdapter = nullptr;
  if (Adapter > 0) return static_cast<HRESULT>(DXGI_ERROR_NOT_FOUND);
  *ppAdapter = new DXGIAdapterImpl();
  return S_OK;
}

HRESULT DXGIFactoryImpl::MakeWindowAssociation(HWND, UINT Flags) {
  VKWIND11_LOG_TRACE("MakeWindowAssociation: flags=0x%x", Flags);
  return S_OK;
}

HRESULT DXGIFactoryImpl::CreateSwapChain(void* pDevice, const DXGI_SWAP_CHAIN_DESC* pDesc, void** ppSwapChain) {
  if (!pDesc || !ppSwapChain) return E_INVALIDARG;
  *ppSwapChain = nullptr;
  auto* device = static_cast<ID3D11Device*>(pDevice);
  auto* swapchain = new DXGISwapChainImpl(this, device, pDesc);
  *ppSwapChain = swapchain;
  return S_OK;
}

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

// ============================================================================
// Swap Chain Factory (used by D3D11CreateDeviceAndSwapChain)
// ============================================================================

IDXGISwapChain1* createSwapChainImpl(ID3D11Device* device, const DXGI_SWAP_CHAIN_DESC* desc) {
  auto* sc = new DXGISwapChainImpl(nullptr, device, desc);
  return sc;
}

// ============================================================================
// DXGI Initialization
// ============================================================================

void dxgiInit() {
  VKWIND11_LOG_INFO("DXGI initialized");
}
