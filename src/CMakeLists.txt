add_library(VkMana
        VkMana/Context.cpp
        VkMana/Garbage.cpp
        VkMana/CommandPool.cpp
        VkMana/CommandBuffer.cpp
        VkMana/Descriptors.cpp
        VkMana/DescriptorAllocator.cpp
        VkMana/Pipeline.cpp
        VkMana/Image.cpp
        VkMana/Buffer.cpp
        VkMana/ShaderCompiler.cpp
)

target_include_directories(VkMana PUBLIC "./")

target_link_libraries(VkMana PRIVATE shaderc shaderc_util Vulkan::dxc_lib PUBLIC Vulkan::Vulkan vma-hpp SDL2-static)

target_compile_features(VkMana PUBLIC cxx_std_20)
target_compile_definitions(VkMana PRIVATE #[[WIN32_LEAN_AND_MEAN NOMINMAX]])