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

target_include_directories(VkMana PRIVATE "./")
target_include_directories(VkMana SYSTEM INTERFACE "./")

target_link_libraries(VkMana PRIVATE shaderc shaderc_util PUBLIC fmt::fmt Vulkan::Headers VulkanMemoryAllocator-Hpp SDL2-static)

target_compile_features(VkMana PUBLIC cxx_std_20)

target_compile_options(VkMana PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Werror -pedantic-errors -Wall -Wno-unused -Wno-sign-compare>
        $<$<CXX_COMPILER_ID:MSVC>:/WX /W4 /external:anglebrackets /external:W0 /wd4100>
)