add_subdirectory(Vulkan-Headers)

add_subdirectory(SPIRV-Headers)
add_subdirectory(SPIRV-Tools)
add_subdirectory(glslang)

# vma-hpp
add_subdirectory(vma-hpp)

# ShaderC
option(SHADERC_SKIP_INSTALL "Skip installation" ON)
option(SHADERC_SKIP_TESTS "Skip building tests" ON)
option(SHADERC_SKIP_EXAMPLES "Skip building examples" ON)
option(SHADERC_SKIP_COPYRIGHT_CHECK "Skip copyright check" ON)
option(SHADERC_ENABLE_SHARED_CRT "Use the shared CRT instead of the static CRT" ON)
add_subdirectory(shaderc)

# FmtLib
option(FMT_INSTALL "Generate the install target." OFF)
add_subdirectory(fmt)