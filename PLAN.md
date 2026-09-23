# VKWIND11 — D3D10/11/12 → Vulkan Translation Layer

## Target
GTA V LITE (OptiJuegos) on Android ARM64/Mali-G57 MC2 at 30 FPS.

## Architecture

D3D11 games → `d3d11.dll` (VKWIND11) → Vulkan API

This is a **native D3D11→Vulkan** translation layer, NOT a wrapper around D3D9.
Similar to DXVK but built from scratch for our VKWIND ecosystem.

## Scope — API Surface

### Core D3D11 Interfaces (~200 methods total)

| Interface | Methods | Priority | Notes |
|-----------|---------|----------|-------|
| `ID3D11Device` | ~50 | P0 | Resource creation, shader compilation |
| `ID3D11DeviceContext` | ~100 | P0 | Draw, dispatch, state setting |
| `ID3D11Buffer` | - | P0 | Vertex/index/constant/upload buffers |
| `ID3D11Texture2D` | - | P0 | Render targets, depth stencils, textures |
| `ID3D11ShaderResourceView` | - | P0 | Texture/buffer SRVs |
| `ID3D11RenderTargetView` | - | P0 | Render target views |
| `ID3D11DepthStencilView` | - | P0 | Depth/stencil views |
| `ID3D11VertexShader` | - | P0 | VS bytecode → SPIR-V |
| `ID3D11PixelShader` | - | P0 | PS bytecode → SPIR-V |
| `ID3D11ComputeShader` | - | P1 | CS bytecode → SPIR-V |
| `ID3D11GeometryShader` | - | P1 | GS bytecode → SPIR-V |
| `ID3D11HullShader` | - | P2 | HS bytecode → SPIR-V |
| `ID3D11DomainShader` | - | P2 | DS bytecode → SPIR-V |
| `ID3D11BlendState` | - | P0 | VkPipelineColorBlendState |
| `ID3D11DepthStencilState` | - | P0 | VkPipelineDepthStencilState |
| `ID3D11RasterizerState` | - | P0 | VkPipelineRasterizationState |
| `ID3D11SamplerState` | - | P0 | VkSampler |
| `ID3D11InputLayout` | - | P0 | VkVertexInputAttributeDescription |
| `ID3D11Query` | - | P1 | VkQueryPool |
| `ID3D11Predicate` | - | P2 | Occlusion/conditional rendering |
| `ID3D11UnorderedAccessView` | - | P1 | UAV for compute |
| `ID3D11ClassInstance` | - | P2 | Effect framework (rare) |
| `IDXGISwapChain` | - | P0 | Present, swapchain management |
| `IDXGIFactory` | - | P0 | Adapter enumeration |
| `IDXGIAdapter` | - | P0 | GPU info |

### D3DX11 Library (~50 functions)

| Category | Functions | Priority |
|----------|-----------|----------|
| Math | D3DXVec3/4/Matrix multiply, transform, etc. | P0 |
| Shader compile | D3DCompileFromFile, D3DCompile | P0 |
| Texture load | D3DX11CreateShaderResourceViewFromFile | P1 |
| Math types | D3DXVECTOR2/3/4, D3DXMATRIX | P0 |
| Misc | D3DX11FilterTexture, D3DX11SaveTextureToFile | P2 |

### D3D12 Core (~80 methods)

| Interface | Methods | Priority |
|-----------|---------|----------|
| `ID3D12Device` | ~40 | P1 |
| `ID3D12CommandQueue` | ~5 | P1 |
| `ID3D12CommandAllocator` | ~3 | P1 |
| `ID3D12GraphicsCommandList` | ~50 | P1 |
| `ID3D12Resource` | ~10 | P1 |
| `ID3D12DescriptorHeap` | ~3 | P1 |
| `ID3D12PipelineState` | ~2 | P1 |
| `ID3D12RootSignature` | - | P1 |

### D3DX12 (~30 helpers)

| Category | Functions | Priority |
|----------|-----------|----------|
| Memory | D3D12_HEAP_PROPERTIES, resource placement | P1 |
| Pipeline | D3D12_GRAPHICS_PIPELINE_STATE_DESC | P1 |
| Root signature | D3D12_ROOT_PARAMETER, D3D12_DESCRIPTOR_RANGE | P1 |

## Project Structure

```
vkwind11/
├── meson.build                    # Build system
├── android-arm64.txt              # Android cross-compile
├── README.md
├── PLAN.md
├── src/
│   ├── d3d11/                     # D3D11 API implementation
│   │   ├── d3d11_types.h          # D3D11 enums, structs, defines
│   │   ├── d3d11_device.h/cpp     # ID3D11Device implementation
│   │   ├── d3d11_context.h/cpp    # ID3D11DeviceContext implementation
│   │   ├── d3d11_resource.h       # Resource wrapper types
│   │   ├── d3d11_view.h           # SRV/RTV/DSV/UAV implementations
│   │   ├── d3d11_state.h          # Blend/DS/Rasterizer/Sampler states
│   │   ├── d3d11_shader.h/cpp     # Shader objects + DXBC→SPIR-V
│   │   ├── d3d11_main.cpp         # D3D11CreateDevice export
│   │   └── d3d11_dxgi.h/cpp       # DXGI swapchain/adapter
│   ├── d3d12/                     # D3D12 API implementation
│   │   ├── d3d12_types.h
│   │   ├── d3d12_device.h/cpp
│   │   ├── d3d12_cmdqueue.h/cpp
│   │   ├── d3d12_cmdlist.h/cpp
│   │   ├── d3d12_pipeline.h/cpp
│   │   ├── d3d12_memory.h/cpp
│   │   └── d3d12_main.cpp
│   ├── d3dx/                      # D3DX10/11/12 helper library
│   │   ├── d3dx_math.h            # Vector/Matrix math (no deps)
│   │   ├── d3dx_math.cpp
│   │   ├── d3dx_shader.h          # D3DCompile wrapper
│   │   ├── d3dx_shader.cpp
│   │   ├── d3dx_texture.h         # Texture load/save
│   │   └── d3dx_texture.cpp
│   ├── shader/                    # Shader translation (shared)
│   │   ├── dxbc_parser.h/cpp      # DXBC bytecode parser
│   │   ├── sm4_translator.h/cpp   # SM4/SM5 → SPIR-V translator
│   │   └── spirv_builder.h/cpp    # Low-level SPIR-V emission
│   ├── vulkan/                    # Vulkan abstraction (shared with VKWIND)
│   │   ├── vk_device.h/cpp        # VkDevice, VkPhysicalDevice
│   │   ├── vk_cmd_buffer.h/cpp    # Command buffer management
│   │   ├── vk_pipeline.h/cpp      # Pipeline cache + creation
│   │   ├── vk_swapchain.h/cpp     # Swapchain management
│   │   ├── vk_descriptor.h/cpp    # Descriptor set management
│   │   └── vk_memory.h/cpp        # Memory allocation (VMA or custom)
│   └── common/                    # Shared utilities
│       ├── logging.h              # VKWIND11_LOG
│       ├── refcount.h             # COM IUnknown implementation
│       └── config.h               # Runtime config (env vars)
├── tests/
│   ├── test_d3d11_device.cpp      # Device creation tests
│   ├── test_sm4_spirv.cpp         # SM4→SPIR-V translation tests
│   └── test_pipeline.cpp          # Full pipeline tests
└── external/
    └── dxbc-spirv/                # Git submodule (future)
```

## Implementation Phases

### Phase 1: Project Skeleton + D3D11 Types (~300 lines)
- meson.build, directory structure
- d3d11_types.h: all D3D11 enums, structs, GUIDs
- Common utilities: logging, refcount, config
- **Verify**: build succeeds, types compile

### Phase 2: DXGI Layer (~400 lines)
- IDXGIFactory, IDXGIAdapter, IDXGISwapChain
- Window/surface management
- VkSurfaceKHR creation from Android ANativeWindow
- **Verify**: can enumerate adapters, create surface

### Phase 3: D3D11 Device + Context Core (~800 lines)
- ID3D11Device: CreateBuffer, CreateTexture2D, CreateShaderResourceView, CreateRenderTargetView, CreateDepthStencilView
- ID3D11DeviceContext: IASetVertexBuffers, IASetIndexBuffer, IASetInputLayout, IASetPrimitiveTopology
- ID3D11DeviceContext: VSSetShader, PSSetShader, VSSetConstantBuffers, PSSetConstantBuffers
- ID3D11DeviceContext: Draw, DrawIndexed
- **Verify**: can create resources and issue draw calls

### Phase 4: DXBC→SPIR-V Shader Translator (~1500 lines)
- DXBC bytecode parser (chunk-based format)
- SM4/SM5 instruction decoder
- SM4→SPIR-V translation (VS, PS, CS)
- Register mapping: cbuffer → UBO, t# → combined sampler, o# → output
- **Verify**: spirv-val passes for test shaders

### Phase 5: State Management (~500 lines)
- BlendState → VkPipelineColorBlendState
- DepthStencilState → VkPipelineDepthStencilState
- RasterizerState → VkPipelineRasterizationState
- SamplerState → VkSampler
- InputLayout → VkVertexInputAttributeDescription
- **Verify**: state objects create correctly

### Phase 6: Full Draw Pipeline (~600 lines)
- Draw/DrawIndexed with all state bindings
- Render pass management (load/store ops)
- Framebuffer creation from RTV+DSV
- Pipeline creation (VkGraphicsPipelineCreateInfo)
- **Verify**: renders a triangle to screen

### Phase 7: D3DX11 Math + Shader Compile (~500 lines)
- D3DXVECTOR2/3/4, D3DXMATRIX operations
- D3DXMatrixMultiply, D3DXVec3TransformCoord
- D3DCompile wrapper (GLSL/HLSL → SM4/SM5 bytecode)
- **Verify**: math tests pass, shader compilation works

### Phase 8: D3D12 Core (~1000 lines)
- ID3D12Device, ID3D12CommandQueue, ID3D12CommandList
- Root signature parsing
- Descriptor heap management
- Pipeline state creation
- Resource states and barriers
- **Verify**: can create device, command list, draw

### Phase 9: Texture Loading + Resource Management (~400 lines)
- DDS texture loading
- Mipmap generation
- Texture upload via staging buffers
- Resource state tracking
- **Verify**: textures load and display correctly

### Phase 10: Optimization + Mali Tuning (~400 lines)
- Async pipeline compilation
- Pipeline caching (disk)
- Descriptor set pooling
- Vertex buffer ring buffers
- Mali-specific workarounds
- **Verify**: performance benchmarks

## Total Estimated: ~6000-8000 lines

## Key Technical Decisions

### 1. DXBC→SPIR-V (use dxbc-spirv or build custom?)
**Decision**: Build custom SM4 translator initially. dxbc-spirv is MIT and can be integrated later for SM5.
- SM4 is well-documented (D3D11.0 feature level 10.0/10.1/11.0)
- GTA V uses SM4.0/SM5.0 — start with SM4, add SM5 later
- Custom translator gives full control over Mali-specific optimizations

### 2. Memory Management
**Decision**: Use Vulkan Memory Allocator (VMA) for now.
- VMA handles HOST_VISIBLE, DEVICE_LOCAL, staging
- Can replace with custom allocator later

### 3. Threading Model
**Decision**: D3D11 immediate context → Vulkan primary command buffer (single-threaded).
- D3D11 deferred contexts → Vulkan secondary command buffers (future)
- Most games use only immediate context

### 4. Descriptor Management
**Decision**: Push descriptors + descriptor pooling.
- D3D11 binds descriptors per-draw → Vulkan push descriptors
- Pool allocation for descriptor sets

### 5. Swapchain
**Decision**: VkSwapchainKHR via Android ANativeWindow.
- VK_KHR_android_surface extension
- Triple buffering for mobile

## Dependencies
- Vulkan SDK (headers + loader)
- spirv-val (for validation)
- VMA (Vulkan Memory Allocator) — header-only
- Android NDK (for ARM64 build)

## Verification Strategy
1. Unit tests for each phase (spirv-val, pipeline tests)
2. Render a triangle (Phase 6)
3. Render textured quads (Phase 9)
4. Run GTA V LITE on Android (Phase 10)
