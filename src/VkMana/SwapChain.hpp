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

        bool Recreate(uint32_t width, uint32_t height, bool vsync);
        void Present();

#pragma region Getters

        bool IsValid() const { return m_pContext != nullptr && m_swapChain != nullptr && m_width > 0 && m_height > 0; }

        auto GetSurface() const -> auto { return m_surface; }
        auto GetSwapChain() const -> auto { return m_swapChain; }
        auto GetBackBufferIndex() const -> auto { return m_backBufferIndex; }

        auto GetBackBufferWidth() const -> auto { return m_width; }
        auto GetBackBufferHeight() const -> auto { return m_height; }
        auto GetBackBufferFormat() const -> auto { return m_backBufferFormat; }

        auto GetRenderPass() -> RenderPassInfo;

        /* Returns float(GetBackBufferWidth()) / float(GetBackBufferHeight()) */
        auto GetAspectRatio() const -> auto { return float(GetBackBufferWidth()) / float(GetBackBufferHeight()); }

#pragma endregion

    private:
        SwapChain(Context* pContext, vk::SurfaceKHR surface);

        void AcquireNextImage();

    private:
        Context* m_pContext = nullptr;
        vk::SurfaceKHR m_surface = nullptr;
        vk::SwapchainKHR m_swapChain = nullptr;
        vk::Fence m_presentFence = nullptr;
        std::vector<ImageHandle> m_backBufferImages;
        uint32_t m_backBufferIndex = 0;

        uint32_t m_width = 0;
        uint32_t m_height = 0;
        vk::Format m_backBufferFormat = {};
    };

    using SwapChainHandle = IntrusivePtr<SwapChain>;

} // namespace VkMana