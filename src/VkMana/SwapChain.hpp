#pragma once

#include "Image.hpp"
#include "RenderPass.hpp"

namespace VkMana
{
    class Context;

    class SwapChain : public IntrusivePtrEnabled<SwapChain>
    {
    public:
        static auto New(Context* pContext, vk::SurfaceKHR surface, uint32_t width, uint32_t height) -> IntrusivePtr<SwapChain>;

        ~SwapChain();

        void Present();

#pragma region Getters

        auto GetSurface() const -> auto { return m_surface; }
        auto GetSwapChain() const -> auto { return m_swapChain; }
        auto GetBackBufferIndex() const -> auto { return m_backBufferIndex; }

        auto GetBackBufferWidth() const -> auto { return m_width; }
        auto GetBackBufferHeight() const -> auto { return m_height; }
        auto GetBackBufferFormat() const -> auto { return m_backBufferFormat; }

        auto GetRenderPass() -> RenderPassInfo;

#pragma endregion

    private:
        SwapChain(
            Context* pContext,
            vk::SurfaceKHR surface,
            vk::SwapchainKHR swapChain,
            uint32_t width,
            uint32_t height,
            vk::Format backBufferFormat,
            const std::vector<ImageHandle>& backBufferImages
        );

        void AcquireNextImage();

    private:
        Context* m_pContext = nullptr;
        vk::SurfaceKHR m_surface = nullptr;
        vk::SwapchainKHR m_swapChain = nullptr;
        vk::Fence m_presentFence = nullptr;
        std::vector<ImageHandle> m_backBufferImages;
        uint32_t m_backBufferIndex = 0;

        uint32_t m_width;
        uint32_t m_height;
        vk::Format m_backBufferFormat;
    };

    using SwapChainHandle = IntrusivePtr<SwapChain>;

} // namespace VkMana