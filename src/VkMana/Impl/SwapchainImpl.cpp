#include "SwapchainImpl.hpp"

#include "GraphicsDeviceImpl.hpp"
#include "SurfaceUtil.hpp"

#undef min

namespace VkMana
{
	Swapchain::Impl::Impl(GraphicsDevice& graphicsDevice, const SwapchainDescription& desc)
		: m_graphicsDevice(graphicsDevice)
		, m_syncToVerticalBlank(desc.SyncToVerticalBlank)
		, m_source(desc.Source)
		, m_colorSrgb(desc.ColorSrgb)
	{
		m_surface = SurfaceUtil::CreateSurface(*graphicsDevice.GetImpl(), graphicsDevice.GetImpl()->GetInstance(), *desc.Source);

		// #TODO: Get actual present queue index. (Check graphics queue + check if queue actually supports present)
		m_presentQueueIndex = m_graphicsDevice.GetImpl()->GetGraphicsFamilyIndex();
		m_presentQueue = m_graphicsDevice.GetImpl()->GetLogicalDevice().getQueue(m_presentQueueIndex, 0);

		// #TODO: Create framebuffer

		CreateSwapchain(desc.Width, desc.Height);

		vk::FenceCreateInfo fenceInfo{};
		m_imageReadyFence = m_graphicsDevice.GetImpl()->GetLogicalDevice().createFence(fenceInfo);

		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();
		AcquireNextImage(device, {}, m_imageReadyFence);
		void(device.waitForFences(m_imageReadyFence, VK_TRUE, std::uint64_t(-1)));
		device.resetFences(m_imageReadyFence);
	}

	Swapchain::Impl::~Impl()
	{
		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();

		device.destroy(m_swapchain);
	}

	void Swapchain::Impl::SetSurface(vk::SurfaceKHR newSurface) {}

	void Swapchain::Impl::SetSyncToVerticalBlank(bool syncToVerticalBlank) {}

	void Swapchain::Impl::Resize(std::uint32_t width, std::uint32_t height) {}

	void Swapchain::Impl::Present()
	{
		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(m_swapchain);
		presentInfo.setImageIndices(m_currentImageIndex);
		void(m_presentQueue.presentKHR(presentInfo));

		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();
		AcquireNextImage(device, {}, m_imageReadyFence);
		void(device.waitForFences(m_imageReadyFence, VK_TRUE, std::uint64_t(-1)));
		device.resetFences(m_imageReadyFence);
	}

	void Swapchain::Impl::CreateSwapchain(std::uint32_t width, std::uint32_t height)
	{
		auto physicalDevice = m_graphicsDevice.GetImpl()->GetPhysicalDevice();

		auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_surface);
		/* if (result == vk::Result::eErrorSurfaceLostKHR)
		{
			// #TODO: Error.
			return; // return false;
		} */

		if (capabilities.minImageExtent.width == 0 && capabilities.minImageExtent.height == 0 && capabilities.maxImageExtent.width == 0
			&& capabilities.maxImageExtent.height == 0)
		{
			return; // return false;
		}

		m_graphicsDevice.WaitForIdle();

		auto formats = physicalDevice.getSurfaceFormatsKHR(m_surface);

		auto desiredFormat = m_colorSrgb ? vk::Format::eB8G8R8A8Srgb : vk::Format::eB8G8R8A8Unorm;
		vk::SurfaceFormatKHR surfaceFormat;
		if (formats.size() == 1 && formats[0].format == vk::Format::eUndefined)
		{
			surfaceFormat = vk::SurfaceFormatKHR(desiredFormat, vk::ColorSpaceKHR::eSrgbNonlinear);
		}
		else
		{
			for (auto& format : formats)
			{
				if (format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && format.format == desiredFormat)
				{
					surfaceFormat = format;
					break;
				}
			}
			if (surfaceFormat.format == vk::Format::eUndefined)
			{
				if (m_colorSrgb && surfaceFormat.format != vk::Format::eR8G8B8A8Srgb)
				{
					// #TODO: Error. Unable to create sRGB swapchain for surface.
				}
				surfaceFormat = formats[0];
			}
		}

		auto presentModes = physicalDevice.getSurfacePresentModesKHR(m_surface);
		vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
		if (m_syncToVerticalBlank)
		{
			const auto it = std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eFifoRelaxed);
			if (it != presentModes.end())
			{
				presentMode = vk::PresentModeKHR::eFifoRelaxed;
			}
		}
		else
		{
			const auto it = std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox);
			if (it != presentModes.end())
			{
				presentMode = vk::PresentModeKHR::eMailbox;
			}
			else
			{
				const auto it = std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eImmediate);
				if (it != presentModes.end())
				{
					presentMode = vk::PresentModeKHR::eImmediate;
				}
			}
		}

		auto maxImageCount = capabilities.maxImageCount == 0 ? std::uint32_t(-1) : capabilities.maxImageCount;
		auto imageCount = std::min(maxImageCount, capabilities.minImageCount + 1);

		auto clampedWidth = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		auto clampedHeight = std::clamp(width, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		std::vector<std::uint32_t> queueFamilyIndices = {
			std::uint32_t(m_graphicsDevice.GetImpl()->GetGraphicsFamilyIndex()),
			std::uint32_t(m_graphicsDevice.GetImpl()->GetPresentFamilyIndex()),
		};

		auto oldSwapchain = m_swapchain;

		vk::SwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.setSurface(m_surface);
		swapchainInfo.setPresentMode(presentMode);
		swapchainInfo.setImageFormat(surfaceFormat.format);
		swapchainInfo.setImageColorSpace(surfaceFormat.colorSpace);
		swapchainInfo.setImageExtent({ clampedWidth, clampedHeight });
		swapchainInfo.setMinImageCount(imageCount);
		swapchainInfo.setImageArrayLayers(1);
		swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
		if (m_graphicsDevice.GetImpl()->GetGraphicsFamilyIndex() != m_graphicsDevice.GetImpl()->GetPresentFamilyIndex())
		{
			swapchainInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
			swapchainInfo.setQueueFamilyIndices(queueFamilyIndices);
		}
		else
		{
			swapchainInfo.setImageSharingMode(vk::SharingMode::eExclusive);
		}
		swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
		swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapchainInfo.setClipped(VK_FALSE);
		swapchainInfo.setOldSwapchain(oldSwapchain);
		m_swapchain = m_graphicsDevice.GetImpl()->GetLogicalDevice().createSwapchainKHR(swapchainInfo);

		if (oldSwapchain)
		{
			m_graphicsDevice.GetImpl()->GetLogicalDevice().destroy(oldSwapchain);
		}

		//		m_framebuffer.SetNewSwapchain(m_swapchain, width, height, surfaceFormat, swapchainInfo.imageExtent);
	}

	void Swapchain::Impl::AcquireNextImage(vk::Device device, vk::Semaphore semaphore, vk::Fence fence)
	{
		if (m_isDirty)
		{
			// RecreateAndReacquire(m_framebuffer.GetWidth(), m_framebuffer.GetHeight());
			return;
		}

		auto result = m_graphicsDevice.GetImpl()->GetLogicalDevice().acquireNextImageKHR(m_swapchain, std::uint64_t(-1), semaphore, fence);
		m_currentImageIndex = result.value;
		// m_framebuffer.SetImageIndex(m_currentImageIndex);
		if (result.result == vk::Result::eErrorOutOfDateKHR || result.result == vk::Result::eSuboptimalKHR)
		{
			// CreateSwapchain(m_framebuffer.GetWidth(), m_framebuffer.GetHeight());
		}
		else if (result.result != vk::Result::eSuccess)
		{
			// #TODO: Error.
		}
	}

	void Swapchain::Impl::RecreateAndReacquire(std::uint32_t width, std::uint32_t height)
	{
		CreateSwapchain(width, height);

		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();
		AcquireNextImage(device, {}, m_imageReadyFence);
		void(device.waitForFences(m_imageReadyFence, VK_TRUE, std::uint64_t(-1)));
		device.resetFences(m_imageReadyFence);

		m_isDirty = false;
	}

} // namespace VkMana