add_library(VkMana
        VkMana/Impl/GraphicsDeviceImpl.cpp
        VkMana/Impl/CommandListImpl.cpp
        VkMana/Impl/FenceImpl.cpp
        VkMana/Impl/AllocationManager.cpp
        VkMana/Impl/SwapchainImpl.cpp
        VkMana/Impl/SurfaceUtil.cpp
        VkMana/GraphicsDevice.cpp
        VkMana/ResourceFactory.cpp
        VkMana/SwapchainSource.cpp
        VkMana/CommandList.cpp
        VkMana/Fence.cpp
        VkMana/Swapchain.cpp
        VkMana/SwapchainFramebuffer.cpp
)

target_include_directories(VkMana PUBLIC ../include)

target_link_libraries(VkMana PRIVATE vulkan vma-hpp)

target_compile_features(VkMana PRIVATE cxx_std_17)