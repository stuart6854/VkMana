#include "SwapChain.hpp"

#include "Context.hpp"

namespace VkMana
{
    auto SwapChain::New(Context* pContext, vk::SurfaceKHR surface, uint32_t width, uint32_t height) -> IntrusivePtr<SwapChain>
    {
        auto surfaceCaps = pContext->GetPhysicalDevice().getSurfaceCapabilitiesKHR(surface);

        uint32_t minImageCount = surfaceCaps.minImageCount + 1;
        if(surfaceCaps.maxImageCount != 0 && minImageCount < surfaceCaps.maxImageCount)
        {
            minImageCount = surfaceCaps.maxImageCount;
        }

        auto surfaceFormat = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb); // #TODO: Pick best available format

        width = std::clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
        height = std::clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

        auto presentMode = vk::PresentModeKHR::eFifo;

        vk::SwapchainCreateInfoKHR swapchainInfo{};
        swapchainInfo.setSurface(surface);
        swapchainInfo.setMinImageCount(minImageCount);
        swapchainInfo.setImageFormat(surfaceFormat.format);
        swapchainInfo.setImageColorSpace(surfaceFormat.colorSpace);
        swapchainInfo.setImageExtent({ width, height });
        swapchainInfo.setImageArrayLayers(1);
        swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
        swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
        swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
        swapchainInfo.setPresentMode(presentMode);
        swapchainInfo.setClipped(VK_TRUE);
        swapchainInfo.setOldSwapchain(nullptr);
        auto swapChain = pContext->GetDevice().createSwapchainKHR(swapchainInfo);
        if(swapChain == nullptr)
        {
            VM_ERR("Failed to create SwapChain");
            return nullptr;
        }

        auto swapchainImages = pContext->GetDevice().getSwapchainImagesKHR(swapChain);
        std::vector<ImageHandle> backBufferImages(swapchainImages.size());
        for(auto i = 0u; i < swapchainImages.size(); ++i)
        {
            backBufferImages[i] = IntrusivePtr(new Image(pContext, swapchainImages[i], width, height, surfaceFormat.format));
            if(backBufferImages[i] == nullptr)
            {
                VM_ERR("Failed to create SwapChain (BackBuffer image {} was not created)", i);
                return nullptr;
            }
        }
        assert(backBufferImages.size() > 0);

        auto pNewSwapChain = IntrusivePtr(new SwapChain(pContext, surface, swapChain, width, height, surfaceFormat.format, backBufferImages));
        return pNewSwapChain;
    }

    SwapChain::~SwapChain()
    {
        m_pContext->GetGraphicsQueue().waitIdle();
        m_pContext->GetDevice().destroy(m_presentFence);
        m_pContext->GetDevice().destroy(m_swapChain);
        m_pContext->GetInstance().destroy(m_surface);
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
            .Targets = {
                RenderPassTarget{
                    .Image = pBackBufferImage->GetImageView(ImageViewType::RenderTarget),
                    .IsDepthStencil = false,
                    .Clear = true,
                    .Store = true,
                    .ClearValue = { 0.0f, 0.0f, 0.0f, 1.0f },
                    .PreLayout = vk::ImageLayout::eUndefined,
                    .PostLayout = vk::ImageLayout::ePresentSrcKHR,
                },
            },
        };
        return renderPassInfo;
    }

    SwapChain::SwapChain(
        Context* pContext,
        vk::SurfaceKHR surface,
        vk::SwapchainKHR swapChain,
        uint32_t width,
        uint32_t height,
        vk::Format backBufferFormat,
        const std::vector<ImageHandle>& backBufferImages
    )
        : m_pContext(pContext)
        , m_surface(surface)
        , m_swapChain(swapChain)
        , m_width(width)
        , m_height(height)
        , m_backBufferFormat(backBufferFormat)
        , m_backBufferImages(backBufferImages)
    {
        m_presentFence = m_pContext->GetDevice().createFence({});
        AcquireNextImage();
    }

    void SwapChain::AcquireNextImage()
    {
        auto device = m_pContext->GetDevice();
        m_backBufferIndex = device.acquireNextImageKHR(m_swapChain, UINT64_MAX, nullptr, m_presentFence).value;
        UNUSED(device.waitForFences(m_presentFence, true, UINT64_MAX));
        device.resetFences(m_presentFence);
    }

} // namespace VkMana