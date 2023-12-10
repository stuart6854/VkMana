# BEGIN Dependencies
CPMAddPackage(NAME glm
        GITHUB_REPOSITORY g-truc/glm
        GIT_TAG 0.9.9.8
        SYSTEM ON
)

CPMAddPackage(
        NAME glfw
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.8
        SYSTEM ON
        OPTIONS
        "GLFW_BUILD_DOCS OFF"
        "GLFW_INSTALL OFF"
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
        src/core/Input.cpp
        src/core/stb_impls.cpp
        src/samples/HelloTriangle/HelloTriangle.cpp
        src/samples/ModelLoading/ModelLoading.cpp
        src/samples/Sandbox/Sandbox.cpp
        src/samples/Sandbox/Renderer.cpp
        src/samples/Sandbox/StaticMesh.cpp
        src/samples/Sandbox/ModelLoader.cpp
)
target_include_directories(samples_app PRIVATE src)

target_link_libraries(samples_app PRIVATE VkMana glm glfw stb tinygltf tinyobjloader)

target_compile_features(samples_app PRIVATE cxx_std_17)