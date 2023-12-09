add_executable(samples_app
        src/main.cpp
        src/core/App.cpp
        src/samples/HelloTriangle/HelloTriangle.cpp
)
target_include_directories(samples_app PRIVATE src)

target_link_libraries(samples_app PRIVATE VkMana glm SDL2-static stb tinygltf tinyobjloader)

target_compile_features(samples_app PRIVATE cxx_std_17)