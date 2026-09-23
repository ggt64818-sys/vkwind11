# VKWIND11

D3D10/11/12 to Vulkan translation layer for Android/Winlator. Companion project to [VKWIND](https://github.com/ggt64818-sys/vkwind) (D3D9).

## Status

**v0.2.0** — First release.

| Component | Status |
|---|---|
| D3D11 Device/Context | Resource creation, draw calls, state binding |
| D3D12 Device | Pipeline state, command lists, descriptor heaps |
| DXGI | Factory, adapter enumeration, standalone dxgi.dll |
| SM4/DXBC Shader Translator | DXBC parsing, SPIR-V generation |
| D3DX Math | Matrix, quaternion, vector operations |
| Texture Combiners | SRV, RTV, DSV, texture arrays, mipmaps |
| State Objects | Blend, Rasterizer, DepthStencil, Sampler (with GetDesc roundtrip) |
| Tests | **35/35 passing** |
| Winlator Integration | dxvk-vkwind11 + vkd3d-vkwind11 packages |

## What's in v0.2.0

- **D3D11**: Device creation, Buffer/Texture2D/View/Shader/State creation, Map/Unmap, CopyResource, UpdateSubresource
- **D3D12**: Device creation, Graphics/Compute pipeline state, Root signatures, Command lists, Draw/Dispatch
- **DXGI**: CreateDXGIFactory, EnumAdapters, GetDesc, QueryInterface hierarchy
- **SM4 → SPIR-V**: DXBC bytecode parsing, shader translation to SPIR-V
- **Standalone dxgi.dll**: Separate from d3d11.dll for proper Wine DLL loading
- **Winlator packages**: `dxvk-vkwind11-0.2.0.tzst` (D3D11+DXGI), `vkd3d-vkwind11-0.2.0.tzst` (D3D12)

## How to use

### With Winlator (Android)

1. Install [Winlator](https://github.com/ggt64818-sys/winlator/releases/tag/winlator-vkwind)
2. In container settings:
   - **DXVK Version** → `vkwind11-0.2.0` (D3D11 + DXGI)
   - **VKD3D Version** → `vkwind11-0.2.0` (D3D12)
3. Launch your DirectX 10/11/12 game

### With Wine (Linux / Windows)

Copy `d3d11.dll`, `d3d12.dll`, and `dxgi.dll` next to the game executable.

## Build instructions

### Requirements

- [Meson](https://mesonbuild.com/) build system (>= 0.58)
- [Ninja](https://ninja-build.org/) backend
- [MinGW-w64](https://www.mingw-w64.org/) cross-compiler (for Windows/Android builds)
- [Vulkan SDK](https://vulkan.lunarg.com/) headers and libraries

### Windows (testing)

```bash
meson setup build
ninja -C build
# libd3d11.dll, libd3d12.dll, libdxgi.dll are in build/
```

### Android (ARM64)

```bash
meson setup build-android --cross-file android-arm64.txt --buildtype release
ninja -C build-android
# libd3d11.so, libd3d12.so, libdxgi.so are in build-android/
```

## Running tests

```bash
meson setup build && ninja -C build

# All test suites
meson test -C build

# Individual tests
./build/test_d3d11_comprehensive   # 35 tests: textures, buffers, states, shaders
./build/test_d3d11_integration     # 35 tests: real D3D11 API objects
./build/test_d3d12_device          # D3D12 types, IIDs, pipeline desc
./build/test_dxgi                  # DXGI factory, adapters, formats
./build/test_sm4_translate         # D3DX math, SM4 translator, SPIR-V builder
./build/test_sm4_spirv             # SPIR-V validation
./build/test_pipeline              # Vulkan pipeline integration
./build/test_d3d11_device          # D3D11 types and Vulkan device
```

## Project structure

```
vkwind11/
├── src/
│   ├── d3d11/             Direct3D 11 implementation
│   │   ├── d3d11_device.*       ID3D11Device (create resources)
│   │   ├── d3d11_context.*      ID3D11DeviceContext (draw, state)
│   │   ├── d3d11_types.h        D3D11 type definitions
│   │   ├── d3d11_interfaces.h   COM interface definitions
│   │   ├── d3d11_shader.*       Shader objects (VS, PS, CS, GS, HS, DS)
│   │   ├── d3d11_state.*        Blend, Rasterizer, DepthStencil, Sampler
│   │   ├── d3d11_view.*         SRV, RTV, DSV
│   │   ├── d3d11_resource.*     Buffer, Texture2D implementations
│   │   └── d3d11_dxgi.cpp       DXGI (factory, adapter, swapchain)
│   ├── d3d12/             Direct3D 12 implementation
│   │   ├── d3d12_device.*       ID3D12Device (pipelines, heaps)
│   │   ├── d3d12_cmdlist.*      ID3D12GraphicsCommandList
│   │   ├── d3d12_cmdqueue.*     ID3D12CommandQueue
│   │   ├── d3d12_pipeline.*     Root signature, PSO creation
│   │   ├── d3d12_memory.*       Resource allocation, Map/Unmap
│   │   ├── d3d12_heap.*         Heap management
│   │   ├── d3d12_descriptor.*   Descriptor heaps
│   │   └── d3d12_types.h        D3D12 type definitions
│   ├── dxgi/              Standalone DXGI (dxgi.dll)
│   │   └── dxgi_main.cpp        CreateDXGIFactory exports
│   ├── shader/            SM4/DXBC → SPIR-V shader translator
│   │   ├── dxbc_parser.*        DXBC bytecode parser
│   │   ├── sm4_translator.*     SM4 instruction → SPIR-V
│   │   └── spirv_builder.*      SPIR-V binary builder
│   ├── vulkan/            Vulkan backend
│   │   ├── vk_device.*          Device, instance management
│   │   ├── vk_pipeline.*        Graphics/compute pipeline
│   │   ├── vk_swapchain.*       Swapchain management
│   │   ├── vk_descriptor.*      Descriptor set management
│   │   ├── vk_memory.*          Memory allocation
│   │   └── vk_cmd_buffer.*      Command buffer recording
│   ├── d3dx/              D3DX math library
│   │   └── d3dx_math.*          Matrix, quaternion, vector ops
│   └── common/            Shared utilities
│       ├── config.*              Configuration
│       ├── logging.*             Logging
│       ├── refcount.h            COM reference counting
│       └── dxgi_utils.h          DXGI format conversion
├── tests/                 Test suites (8 files, 35+ tests)
├── android-arm64.txt      Android cross-compilation file
└── meson.build            Build configuration
```

## Related projects

- [VKWIND](https://github.com/ggt64818-sys/vkwind) — D3D9 to Vulkan translation layer
- [Winlator VKWIND](https://github.com/ggt64818-sys/winlator/releases/tag/winlator-vkwind) — Winlator with VKWIND/VKWIND11 integrated

## Contributing

Contributions are welcome. Please open an issue before submitting large changes.

## License

MIT
