#pragma once

#include <atomic>
#include <cstdint>
#include "../d3d11/d3d11_types.h"

// ============================================================================
// COM IUnknown base class
// ============================================================================

class RefCounted {
public:
  virtual ~RefCounted() = default;

  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) = 0;

  virtual ULONG STDMETHODCALLTYPE AddRef() {
    return ++m_refCount;
  }

  virtual ULONG STDMETHODCALLTYPE Release() {
    ULONG count = --m_refCount;
    if (count == 0) {
      delete this;
    }
    return count;
  }

protected:
  std::atomic<ULONG> m_refCount{1};
};

// ============================================================================
// Helper for QueryInterface
// ============================================================================

template<typename Interface, const GUID* iid>
class Implements : public Interface {
public:
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
    if (!ppvObject) return E_POINTER;

    if (riid == *iid || riid == IID_IUnknown) {
      *ppvObject = static_cast<Interface*>(this);
      this->AddRef();
      return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
  }
};
