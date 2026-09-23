project('vkwind11', 'cpp',
  version : '0.2.0',
  default_options : [
    'cpp_std=c++20',
    'cpp_args=-DVKWIND11_EXPORTS',
    'warning_level=2',
    'default_library=both',
  ],
)

add_project_arguments('-DNOMINMAX', '-DWIN32_LEAN_AND_MEAN', language : 'cpp')

cc = meson.get_compiler('cpp')

# Platform detection
host_os = host_machine.system()
is_windows = (host_os == 'windows')
is_android = (host_os == 'android')
is_cross = (host_machine.system() != build_machine.system() or host_machine.cpu_family() != build_machine.cpu_family())

# Vulkan dependency
if is_android
  ndk_sysroot = 'C:/Users/ggt64/AppData/Local/Android/Sdk/ndk/29.0.14206865/toolchains/llvm/prebuilt/windows-x86_64/sysroot'
  vulkan_dep = declare_dependency(
    dependencies : cc.find_library('vulkan', dirs : [ndk_sysroot + '/usr/lib/aarch64-linux-android/24']),
  )
else
  vulkan_dep = dependency('vulkan', required : true)
endif

# Common sources (shared between d3d11 and d3d12)
common_sources = files(
  'src/common/logging.cpp',
  'src/common/config.cpp',
)

# D3DX math (no external deps)
d3dx_sources = files(
  'src/d3dx/d3dx_math.cpp',
)

# Shader translation
shader_sources = files(
  'src/shader/dxbc_parser.cpp',
  'src/shader/sm4_translator.cpp',
  'src/shader/spirv_builder.cpp',
)

# Vulkan abstraction
vulkan_sources = files(
  'src/vulkan/vk_device.cpp',
  'src/vulkan/vk_cmd_buffer.cpp',
  'src/vulkan/vk_pipeline.cpp',
  'src/vulkan/vk_swapchain.cpp',
  'src/vulkan/vk_descriptor.cpp',
  'src/vulkan/vk_memory.cpp',
)

# D3D11 library
d3d11_sources = files(
  'src/d3d11/d3d11_device.cpp',
  'src/d3d11/d3d11_context.cpp',
  'src/d3d11/d3d11_view.cpp',
  'src/d3d11/d3d11_state.cpp',
  'src/d3d11/d3d11_shader.cpp',
  'src/d3d11/d3d11_dxgi.cpp',
  'src/d3d11/d3d11_main.cpp',
  'src/d3d11/d3d11_resource.cpp',
)

# D3D12 library
d3d12_sources = files(
  'src/d3d12/d3d12_device.cpp',
  'src/d3d12/d3d12_cmdqueue.cpp',
  'src/d3d12/d3d12_cmdlist.cpp',
  'src/d3d12/d3d12_pipeline.cpp',
  'src/d3d12/d3d12_memory.cpp',
  'src/d3d12/d3d12_heap.cpp',
  'src/d3d12/d3d12_descriptor.cpp',
  'src/d3d12/d3d12_main.cpp',
)

# Combined sources
all_common = common_sources + d3dx_sources + shader_sources + vulkan_sources

# Build d3d11.dll / libd3d11.so
d3d11_lib = shared_library('d3d11',
  d3d11_sources + all_common,
  dependencies : [vulkan_dep],
  install : true,
  gnu_symbol_visibility : 'default',
)

# Build d3d12.dll / libd3d12.so
d3d12_lib = shared_library('d3d12',
  d3d12_sources + all_common,
  dependencies : [vulkan_dep],
  install : true,
  gnu_symbol_visibility : 'default',
)

# Build dxgi.dll / libdxgi.so (standalone, no d3d11_device dependency)
dxgi_sources = files(
  'src/dxgi/dxgi_main.cpp',
  'src/common/logging.cpp',
)
dxgi_lib = shared_library('dxgi',
  dxgi_sources,
  install : true,
  gnu_symbol_visibility : 'default',
)

# Export libraries for linking
d3d11_dep = declare_dependency(link_with : d3d11_lib)
d3d12_dep = declare_dependency(link_with : d3d12_lib)
dxgi_dep = declare_dependency(link_with : dxgi_lib)

# Tests (skip for cross-compilation)
if not is_cross
  test_spirv = executable('test_sm4_spirv',
    'tests/test_sm4_spirv.cpp',
    dependencies : [d3d11_dep],
  )
  test('SM4→SPIR-V translation', test_spirv)

  test_d3d11_device = executable('test_d3d11_device',
    'tests/test_d3d11_device.cpp',
    dependencies : [d3d11_dep],
  )
  test('D3D11 device creation', test_d3d11_device)

  test_pipeline = executable('test_pipeline',
    'tests/test_pipeline.cpp',
    dependencies : [d3d11_dep],
  )
  test('Pipeline integration', test_pipeline)

  test_d3d11_comprehensive = executable('test_d3d11_comprehensive',
    'tests/test_d3d11_comprehensive.cpp',
    dependencies : [d3d11_dep],
  )
  test('D3D11 comprehensive (device/resources/states)', test_d3d11_comprehensive)

  test_d3d12_device = executable('test_d3d12_device',
    'tests/test_d3d12_device.cpp',
    dependencies : [d3d12_dep],
  )
  test('D3D12 device types & creation', test_d3d12_device)

  test_dxgi = executable('test_dxgi',
    'tests/test_dxgi.cpp',
    dependencies : [d3d11_dep],
  )
  test('DXGI factory & adapters', test_dxgi)

  test_sm4_translate = executable('test_sm4_translate',
    'tests/test_sm4_translate.cpp',
    dependencies : [d3d11_dep],
  )
  test('SM4 translator & D3DX math', test_sm4_translate)

  test_d3d11_integration = executable('test_d3d11_integration',
    'tests/test_d3d11_integration.cpp',
    dependencies : [d3d11_dep],
  )
  test('D3D11 integration (textures/buffers/pipeline)', test_d3d11_integration)
endif

message('VKWIND11 v' + meson.project_version())
message('Platform: ' + host_os)
message('Building d3d11 + d3d12 libraries')
