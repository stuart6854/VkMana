#include "SwapchainFramebuffer.hpp"

#include "Impl/GraphicsDeviceImpl.hpp"
#include "Impl/SwapchainImpl.hpp"

namespace VkMana
{
	SwapchainFramebuffer::SwapchainFramebuffer(GraphicsDevice& graphicsDevice,
		Swapchain& swapchain,
		vk::SurfaceKHR surface,
		std::uint32_t width,
		std::uint32_t height,
		PixelFormat depthFormat)
		: m_graphicsDevice(graphicsDevice)
		, m_swapchain(swapchain)
		, m_surface(surface)
		, m_depthFormat(depthFormat)
	{
	}

	SwapchainFramebuffer::~SwapchainFramebuffer() = default;

	void SwapchainFramebuffer::SetImageIndex(std::uint32_t imageIndex)
	{
		m_currentImageIndex = imageIndex;
	}

	void SwapchainFramebuffer::SetNewSwapchain(vk::SwapchainKHR swapchain,
		std::uint32_t width,
		std::uint32_t height,
		vk::SurfaceFormatKHR surfaceFormat,
		vk::Extent2D swapchainExtent)
	{
		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();

		m_desiredWidth = width;
		m_desiredHeight = height;

		m_swapchainImages = device.getSwapchainImagesKHR(m_swapchain.GetImpl()->GetSwapchain());
		m_swapchainImageFormat = surfaceFormat;
		m_swapchainExtent = swapchainExtent;

		CreateDepthTexture();

		if (m_graphicsDevice.GetImpl()->SupportsDynamicRendering())
		{
		}
		else
		{
			CreateFramebuffers();
		}

		m_outputDescription = OutputDescription::CreateFromFramebuffer(*this);
	}

	void SwapchainFramebuffer::TransitionToIntermediateLayout(vk::CommandBuffer cmdBuffer) {}

	void SwapchainFramebuffer::TransitionToFinalLayout(vk::CommandBuffer cmdBuffer) {}

} // namespace VkMana