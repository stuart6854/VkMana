add_library(VkMana
        VkMana/Context.cpp
        VkMana/Garbage.cpp
        VkMana/CommandPool.cpp
        VkMana/CommandBuffer.cpp
        VkMana/Image.cpp
)

target_include_directories(VkMana PUBLIC "./")

target_link_libraries(VkMana PUBLIC Vulkan::Vulkan vma-hpp SDL2-static)

target_compile_features(VkMana PUBLIC cxx_std_20)
target_compile_definitions(VkMana PRIVATE #[[WIN32_LEAN_AND_MEAN NOMINMAX]])