#include "SwapChain.hpp"

#include "Context.hpp"

namespace VkMana
{
    auto SwapChain::New(Context* pContext, vk::SurfaceKHR surface, uint32_t width, uint32_t height) -> IntrusivePtr<SwapChain>
    {
        auto pNewSwapChain = IntrusivePtr(new SwapChain(pContext, surface));
        if(!pNewSwapChain->Recreate(width, height, true))
        {
            return nullptr;
        }
        return pNewSwapChain;
    }

    SwapChain::~SwapChain()
    {
        m_pContext->GetGraphicsQueue().waitIdle();
        m_pContext->GetDevice().destroy(m_presentFence);
        m_pContext->GetDevice().destroy(m_swapChain);
        m_pContext->GetInstance().destroy(m_surface);
    }

    bool SwapChain::Recreate(uint32_t width, uint32_t height, bool vsync)
    {
        m_pContext->GetGraphicsQueue().waitIdle();

        auto surfaceCaps = m_pContext->GetPhysicalDevice().getSurfaceCapabilitiesKHR(m_surface);

        uint32_t minImageCount = surfaceCaps.minImageCount + 1;
        if(surfaceCaps.maxImageCount != 0 && minImageCount < surfaceCaps.maxImageCount)
        {
            minImageCount = surfaceCaps.maxImageCount;
        }

        auto surfaceFormat = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb); // #TODO: Pick best available format

        m_width = std::clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        m_height = std::clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

        auto presentMode = vk::PresentModeKHR::eFifo; // #TODO: Select best available present mode (VSync)

        auto oldSwapChain = m_swapChain;

        vk::SwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.setSurface(m_surface);
        swapchainInfo.setMinImageCount(minImageCount);
        swapchainInfo.setImageFormat(surfaceFormat.format);
        swapchainInfo.setImageColorSpace(surfaceFormat.colorSpace);
        swapchainInfo.setImageExtent({ m_width, m_height });
        swapchainInfo.setImageArrayLayers(1);
        swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
        swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
        swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
        swapchainInfo.setPresentMode(presentMode);
        swapchainInfo.setClipped(VK_TRUE);
        swapchainInfo.setOldSwapchain(oldSwapChain);
        m_swapChain = m_pContext->GetDevice().createSwapchainKHR(swapchainInfo);
        if(m_swapChain == nullptr)
        {
            VM_ERR("Failed to create SwapChain");
            return false;
        }

        if(oldSwapChain)
        {
            m_pContext->GetDevice().destroy(oldSwapChain);
        }

        auto swapchainImages = m_pContext->GetDevice().getSwapchainImagesKHR(m_swapChain);
        assert(swapchainImages.size() > 0);

        m_backBufferImages.resize(swapchainImages.size());
        for(auto i = 0u; i < swapchainImages.size(); ++i)
        {
            m_backBufferImages[i] = IntrusivePtr(new Image(m_pContext, swapchainImages[i], width, height, surfaceFormat.format));
            if(m_backBufferImages[i] == nullptr)
            {
                VM_ERR("Failed to create SwapChain (BackBuffer image {} was not created)", i);
                return false;
            }
            m_backBufferImages[i]->SetDebugName("BackBuffer " + std::to_string(i));
        }

        m_backBufferFormat = surfaceFormat.format;

        AcquireNextImage();

        return true;
    }

    void SwapChain::Present()
    {
        vk::PresentInfoKHR presentInfo{};
        presentInfo.setImageIndices(m_backBufferIndex);
        presentInfo.setSwapchains(m_swapChain);

        UNUSED(m_pContext->GetGraphicsQueue().presentKHR(presentInfo)); // #TODO: Handle result. e.g. Resize SwapChain?

        AcquireNextImage();
    }

    auto SwapChain::GetRenderPass() -> RenderPassInfo
    {
        auto& pBackBufferImage = m_backBufferImages.at(m_backBufferIndex);

        RenderPassInfo renderPassInfo{
            .targets = {
                RenderPassTarget{
                    .pImage = pBackBufferImage->GetImageView(ImageViewType::RenderTarget),
                    .isDepthStencil = false,
                    .clear = true,
                    .store = true,
                    .clearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                    .preLayout = vk::ImageLayout::eUndefined,
                    .postLayout = vk::ImageLayout::ePresentSrcKHR,
                },
            },
        };
        return renderPassInfo;
    }

    SwapChain::SwapChain(Context* pContext, vk::SurfaceKHR surface)
        : m_pContext(pContext)
        , m_surface(surface)
    {
        m_presentFence = m_pContext->GetDevice().createFence({});
    }

    void SwapChain::AcquireNextImage()
    {
        auto device = m_pContext->GetDevice();
        m_backBufferIndex = device.acquireNextImageKHR(m_swapChain, UINT64_MAX, nullptr, m_presentFence).value;
        UNUSED(device.waitForFences(m_presentFence, true, UINT64_MAX));
        device.resetFences(m_presentFence);
    }

} // namespace VkMana