# BEGIN Dependencies
CPMAddPackage(NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 0.9.9.8
        SYSTEM ON
)

CPMAddPackage(
        NAME sdl2
        GITHUB_REPOSITORY libsdl-org/SDL
        GIT_TAG release-2.28.5
        SYSTEM ON
        OPTIONS
        "SDL2_DISABLE_SDL2MAIN ON"
        "SDL2_DISABLE_INSTALL ON"
        "SDL2_DISABLE_UNINSTALL ON"
        "SDL_SHARED OFF"
        "SDL_TEST OFF"
        "SDL_TESTS OFF"
        "SDL_OPENGL OFF"
        "SDL_OPENGLES OFF"
        "SDL_DIRECTX OFF"
        "SDL_METAL OFF"
        "SDL_AUDIO OFF"
        "SDL_HAPTIC OFF"
        "SDL_Hidapi OFF"
        "SDL_Sensor OFF"
        "SDL_Locale OFF"
)

CPMAddPackage(
        NAME stb
        GITHUB_REPOSITORY nothings/stb
        GIT_TAG master
        DOWNLOAD_ONLY True
)
if (stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})
endif ()

CPMAddPackage(
        NAME tinyobjloader
        GITHUB_REPOSITORY tinyobjloader/tinyobjloader
        GIT_TAG release
        SYSTEM ON
)

CPMAddPackage(
        NAME tinygltf
        GITHUB_REPOSITORY syoyo/tinygltf
        GIT_TAG v2.8.18
        SYSTEM ON
)
# END Dependencies

add_executable(samples_app
        src/main.cpp
        src/core/App.cpp
        src/samples/HelloTriangle/HelloTriangle.cpp
)
target_include_directories(samples_app PRIVATE src)

target_link_libraries(samples_app PRIVATE VkMana glm SDL2-static stb tinygltf tinyobjloader)

target_compile_features(samples_app PRIVATE cxx_std_17)