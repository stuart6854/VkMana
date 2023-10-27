#cmake_minimum_required(VERSION 3.10)

#project(third_party LANGUAGES C CXX)

add_library(vulkan INTERFACE)
set(VULKAN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include)
target_sources(vulkan INTERFACE ${VULKAN_INCLUDE_DIR}/vulkan/vulkan.h)
target_include_directories(vulkan INTERFACE ${VULKAN_INCLUDE_DIR})

target_compile_definitions(vulkan
        INTERFACE
        VK_NO_PROTOTYPES
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
        VK_USE_PLATFORM_WIN32_KHR
        # VK_USE_PLATFORM_XLIB_KHR // May not work. vulkan.h tries to include "X11/Xlib.h"
        VK_USE_PLATFORM_WAYLAND_KHR
        # VK_USE_PLATFORM_METAL_EXT
        # VK_USE_PLATFORM_ANDROID_KHR
)

# vma-hpp
add_library(vma-hpp INTERFACE)
set(VMA_HPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma-hpp/include)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vma-hpp/VulkanMemoryAllocator/include)
target_include_directories(vma-hpp INTERFACE ${VMA_HPP_DIR} ${VMA_DIR})
target_link_libraries(vma-hpp INTERFACE vulkan)

# ShaderC
option(SHADERC_SKIP_INSTALL "Skip installation" ON)
option(SHADERC_SKIP_TESTS "Skip building tests" ON)
option(SHADERC_SKIP_EXAMPLES "Skip building examples" ON)
option(SHADERC_SKIP_COPYRIGHT_CHECK "Skip copyright check" ON)
add_subdirectory(shaderc)

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
