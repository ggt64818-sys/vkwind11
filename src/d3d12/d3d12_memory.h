#pragma once

#include "d3d12_device.h"

// ============================================================================
// D3D12 Memory Management (Stub)
// ============================================================================

class D3D12MemoryAllocator {
public:
  D3D12MemoryAllocator() = default;
  ~D3D12MemoryAllocator() = default;

  bool initialize(D3D12Device* device);
  void shutdown();

  bool allocateUploadBuffer(size_t size, void** mappedData);
  bool allocateDefaultBuffer(size_t size);
  void freeAll();

private:
  D3D12Device* m_device = nullptr;
};
