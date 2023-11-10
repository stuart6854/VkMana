find_package(Vulkan REQUIRED COMPONENTS shaderc_combined GLOBAL)

# vma-hpp
add_subdirectory(vma-hpp)

# ShaderC
#[[option(SHADERC_SKIP_INSTALL "Skip installation" ON)
option(SHADERC_SKIP_TESTS "Skip building tests" ON)
option(SHADERC_SKIP_EXAMPLES "Skip building examples" ON)
option(SHADERC_SKIP_COPYRIGHT_CHECK "Skip copyright check" ON)
option(SHADERC_ENABLE_SHARED_CRT "Use the shared CRT instead of the static CRT" ON)
add_subdirectory(shaderc)]]

# sdl2
option(SDL2_DISABLE_SDL2MAIN ON)
option(SDL2_DISABLE_INSTALL ON)
option(SDL2_DISABLE_UNINSTALL ON)
option(SDL_SHARED OFF)
option(SDL_TEST OFF)
option(SDL_TESTS OFF)
option(SDL_OPENGL OFF)
option(SDL_OPENGLES OFF)
option(SDL_DIRECTX OFF)
option(SDL_METAL OFF)
option(SDL_AUDIO OFF)
option(SDL_HAPTIC OFF)
option(SDL_Hidapi OFF)
option(SDL_Sensor OFF)
option(SDL_Locale OFF)
add_subdirectory(sdl2)
set_property(TARGET SDL2-static PROPERTY FOLDER "ThirdParty")

# GLM
# add_subdirectory(glm)

add_subdirectory(stb)
add_subdirectory(tinygltf)
add_subdirectory(tinyobjloader)
