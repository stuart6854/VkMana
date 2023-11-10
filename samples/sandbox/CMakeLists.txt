add_executable(sample_sandbox
        src/main.cpp
        src/Renderer.cpp
        src/StaticMesh.cpp
        src/Material.hpp
        src/ModelLoader.cpp
        src/ModelLoader.hpp
)

target_link_libraries(sample_sandbox PRIVATE VkMana tinygltf tinyobjloader stb)

target_compile_features(sample_sandbox PRIVATE cxx_std_20)