#include "VkManaImpl.hpp"

namespace VkMana
{
	auto GetContext() -> Context_T&
	{
		static Context_T sContext = {};
		return sContext;
	}

	bool CreateGraphicsDevice(GraphicsDevice_T& graphicsDevice, const GraphicsDeviceCreateInfo& createInfo)
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		if (!Vulkan::CreateInstance(graphicsDevice.Instance, createInfo.Debug, {}))
		{
			// #TODO: Error. Failed to create Vulkan instance.
			return false;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(graphicsDevice.Instance);

		if (!Vulkan::CreatePhysicalDevice(graphicsDevice.PhysicalDevice, graphicsDevice.Instance))
		{
			// #TODO: Error. Failed to create Vulkan physical device.
			return false;
		}

		graphicsDevice.QueueFamilyIndices = Vulkan::GetQueueFamilyIndices(graphicsDevice.PhysicalDevice, {});

		if (!CreateLogicalDevice(graphicsDevice.Device, graphicsDevice.PhysicalDevice, graphicsDevice.QueueFamilyIndices, {}))
		{
			// #TODO: Error. Failed to create Vulkan logical device.
			return false;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(graphicsDevice.Device);

		graphicsDevice.GraphicsQueue = graphicsDevice.Device.getQueue(graphicsDevice.QueueFamilyIndices.Graphics, 0);

		if (!Vulkan::CreateAllocator(
				graphicsDevice.Allocator, graphicsDevice.Instance, graphicsDevice.PhysicalDevice, graphicsDevice.Device))
		{
			// #TODO: Error. Failed to create Vulkan allocator.
			return false;
		}

		if (createInfo.MainSwapchainCreateInfo.SurfaceProvider != nullptr)
		{
			graphicsDevice.MainSwapchain = CreateSwapchain(&graphicsDevice, createInfo.MainSwapchainCreateInfo);
			if (graphicsDevice.MainSwapchain == nullptr)
			{
				// #TODO: Error (Fatal?). Failed to create GraphicsDevice main swapchain.
			}
		}

		return true;
	}

	bool CreateSwapchain(Swapchain_T& swapchain, const SwapchainCreateInfo& createInfo)
	{
		if (!CreateSurface(swapchain.Surface, createInfo.SurfaceProvider, swapchain.GraphicsDevice->Instance))
		{
			// #TODO: Error. Failed to create Vulkan surface.
			return false;
		}

		swapchain.Width = createInfo.Width;
		swapchain.Height = createInfo.Height;
		swapchain.VSync = createInfo.vsync;
		swapchain.ColorSrgb = createInfo.Srgb;

		auto oldSwapchain = swapchain.Swapchain;
		if (!Vulkan::CreateSwapchain(swapchain.Swapchain,
				swapchain.Format,
				swapchain.GraphicsDevice->Device,
				swapchain.Surface,
				swapchain.Width,
				swapchain.Height,
				swapchain.VSync,
				oldSwapchain,
				swapchain.ColorSrgb,
				swapchain.GraphicsDevice->PhysicalDevice))
		{
			// #TODO: Error. Failed to create Vulkan swapchain.
			return false;
		}
		swapchain.GraphicsDevice->Device.destroy(oldSwapchain);

		if (!Vulkan::CreateFence(swapchain.AcquireFence, swapchain.GraphicsDevice->Device))
		{
			// #TODO: Error. Failed to create Vulkan fence.
			return false;
		}

		auto swapchainTextures = CreateTexturesFromSwapchainImages(swapchain);

		FramebufferCreateInfo fbInfo{};
		for (auto& texture : swapchainTextures)
		{
			auto& colorTarget = fbInfo.ColorAttachments.emplace_back();
			colorTarget.TargetTexture = texture;
			colorTarget.MipLevel = 0;
			colorTarget.ArrayLayer = 0;
			colorTarget.ClearColor = createInfo.ClearColor;
		}

		swapchain.Framebuffer = CreateFramebuffer(swapchain.GraphicsDevice, fbInfo);
		if (swapchain.Framebuffer == nullptr)
		{
			// #TODO: Error. Failed to create swapchain framebuffer.
			return false;
		}

		for(auto& target : swapchain.Framebuffer->ColorTargets)
		{
			target.FinalLayout = vk::ImageLayout::ePresentSrcKHR;
		}

		auto result =
			swapchain.GraphicsDevice->Device.acquireNextImageKHR(swapchain.Swapchain, std::uint64_t(-1), {}, swapchain.AcquireFence);
		swapchain.Framebuffer->CurrentImageIndex = result.value;

		void(swapchain.GraphicsDevice->Device.waitForFences(swapchain.AcquireFence, VK_TRUE, std::uint64_t(-1)));
		swapchain.GraphicsDevice->Device.resetFences(swapchain.AcquireFence);

		return true;
	}

	bool CreateFramebuffer(Framebuffer_T& framebuffer, const FramebufferCreateInfo& createInfo)
	{
		for (const auto& colorAttachmentInfo : createInfo.ColorAttachments)
		{
			auto& colorTarget = framebuffer.ColorTargets.emplace_back();
			if (colorAttachmentInfo.TargetTexture != nullptr)
			{
				if (colorAttachmentInfo.TargetTexture->Usage != vk::ImageUsageFlagBits::eColorAttachment)
				{
					// #TODO: Error. Framebuffer color attachment texture must be a TextureUsage::RenderTarget texture.
					return false;
				}

				colorTarget.TargetTexture = colorAttachmentInfo.TargetTexture;
				colorTarget.MipLevel = colorAttachmentInfo.MipLevel;
				colorTarget.ArrayLayer = colorAttachmentInfo.ArrayLayer;
				colorTarget.ClearColor = colorAttachmentInfo.ClearColor;
				colorTarget.IntermediateLayout = vk::ImageLayout::eColorAttachmentOptimal;
				colorTarget.FinalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
				vk::ImageSubresourceRange subresource{};
				subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				subresource.baseMipLevel = colorTarget.MipLevel;
				subresource.levelCount = 1;
				subresource.baseArrayLayer = colorTarget.ArrayLayer;
				subresource.layerCount = 1;
				if (!Vulkan::CreateImageView(colorTarget.ImageView,
						framebuffer.GraphicsDevice->Device,
						colorTarget.TargetTexture->Image,
						vk::ImageViewType::e2D,
						colorTarget.TargetTexture->Format,
						subresource))
				{
					// #TODO: Error. Failed to create Vulkan image view.
					return false;
				}
			}
		}
		if (createInfo.DepthAttachment.TargetTexture != nullptr)
		{
			if (framebuffer.DepthTarget.TargetTexture->Usage != vk::ImageUsageFlagBits::eDepthStencilAttachment)
			{
				// #TODO: Error. Framebuffer depth attachment texture must be a TextureUsage::DepthStencil texture.
				return false;
			}

			framebuffer.DepthTarget.TargetTexture = createInfo.DepthAttachment.TargetTexture;
			framebuffer.DepthTarget.MipLevel = createInfo.DepthAttachment.MipLevel;
			framebuffer.DepthTarget.ArrayLayer = createInfo.DepthAttachment.ArrayLayer;
			framebuffer.DepthTarget.IntermediateLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			framebuffer.DepthTarget.FinalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			vk::ImageSubresourceRange subresource{};
			subresource.aspectMask = vk::ImageAspectFlagBits::eDepth;
			subresource.baseMipLevel = framebuffer.DepthTarget.MipLevel;
			subresource.levelCount = 1;
			subresource.baseArrayLayer = framebuffer.DepthTarget.ArrayLayer;
			subresource.layerCount = 1;
			if (!Vulkan::CreateImageView(framebuffer.DepthTarget.ImageView,
					framebuffer.GraphicsDevice->Device,
					framebuffer.DepthTarget.TargetTexture->Image,
					vk::ImageViewType::e2D,
					framebuffer.DepthTarget.TargetTexture->Format,
					subresource))
			{
				// #TODO: Error. Failed to create Vulkan image view.
				return false;
			}
		}
		return true;
	}

	void TransitionFramebufferToIntermediate(CommandList_T& cmdList, Framebuffer_T& framebuffer)
	{
		for (const auto& attachment : framebuffer.ColorTargets)
		{
			if (attachment.IntermediateLayout != vk::ImageLayout::eUndefined)
			{
				vk::ImageSubresourceRange subresource{};
				subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
				subresource.setBaseMipLevel(attachment.MipLevel);
				subresource.setLevelCount(1);
				subresource.setBaseArrayLayer(attachment.ArrayLayer);
				subresource.setLayerCount(1);
				Vulkan::TransitionImage(cmdList.CmdBuffer, attachment.TargetTexture->Image, {}, attachment.IntermediateLayout, subresource);
			}
		}
		if (framebuffer.DepthTarget.IntermediateLayout != vk::ImageLayout::eUndefined)
		{
			vk::ImageSubresourceRange subresource{};
			subresource.setAspectMask(vk::ImageAspectFlagBits::eDepth);
			subresource.setBaseMipLevel(framebuffer.DepthTarget.MipLevel);
			subresource.setLevelCount(1);
			subresource.setBaseArrayLayer(framebuffer.DepthTarget.ArrayLayer);
			subresource.setLayerCount(1);
			Vulkan::TransitionImage(cmdList.CmdBuffer,
				framebuffer.DepthTarget.TargetTexture->Image,
				{},
				framebuffer.DepthTarget.IntermediateLayout,
				subresource);
		}
	}

	void TransitionFramebufferToFinal(CommandList_T& cmdList, Framebuffer_T& framebuffer)
	{
		for (const auto& attachment : framebuffer.ColorTargets)
		{
			if (attachment.FinalLayout != vk::ImageLayout::eUndefined)
			{
				vk::ImageSubresourceRange subresource{};
				subresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
				subresource.setBaseMipLevel(attachment.MipLevel);
				subresource.setLevelCount(1);
				subresource.setBaseArrayLayer(attachment.ArrayLayer);
				subresource.setLayerCount(1);
				Vulkan::TransitionImage(
					cmdList.CmdBuffer, attachment.TargetTexture->Image, attachment.IntermediateLayout, attachment.FinalLayout, subresource);
			}
		}
		if (framebuffer.DepthTarget.FinalLayout != vk::ImageLayout::eUndefined)
		{
			vk::ImageSubresourceRange subresource{};
			subresource.setAspectMask(vk::ImageAspectFlagBits::eDepth);
			subresource.setBaseMipLevel(framebuffer.DepthTarget.MipLevel);
			subresource.setLevelCount(1);
			subresource.setBaseArrayLayer(framebuffer.DepthTarget.ArrayLayer);
			subresource.setLayerCount(1);
			Vulkan::TransitionImage(cmdList.CmdBuffer,
				framebuffer.DepthTarget.TargetTexture->Image,
				framebuffer.DepthTarget.IntermediateLayout,
				framebuffer.DepthTarget.FinalLayout,
				subresource);
		}
	}

	auto ToVkFormat(PixelFormat format) -> vk::Format
	{
		switch (format)
		{
			case PixelFormat::R8_G8_B8_A8_UNorm:
				return vk::Format::eR8G8B8A8Unorm;
			case PixelFormat::B8_G8_R8_A8_UNorm:
				return vk::Format::eB8G8R8A8Unorm;
			case PixelFormat::R8_UNorm:
				return vk::Format::eR8Unorm;
			case PixelFormat::R16_UNorm:
				return vk::Format::eR16Unorm;
			case PixelFormat::R32_G32_B32_A32_Float:
				return vk::Format::eR32G32B32A32Sfloat;
			case PixelFormat::R32_Float:
				return vk::Format::eR32Sfloat;
			case PixelFormat::D24_UNorm_S8_UInt:
				return vk::Format::eD24UnormS8Uint;
			case PixelFormat::D32_Float_S8_UInt:
				return vk::Format::eD32SfloatS8Uint;
			case PixelFormat::R32_G32_B32_A32_UInt:
				return vk::Format::eR32G32B32A32Uint;
			case PixelFormat::R8_G8_SNorm:
				return vk::Format::eR8G8Snorm;
				/*
				case PixelFormat::R8_SNorm:
					return vk::Format::e;
				case PixelFormat::R8_UInt:
					return vk::Format::e;
				case PixelFormat::R8_SInt:
					return vk::Format::e;
				case PixelFormat::R16_SNorm:
					return vk::Format::e;
				case PixelFormat::R16_UInt:
					return vk::Format::e;
				case PixelFormat::R16_SInt:
					return vk::Format::e;
				case PixelFormat::R16_Float:
					return vk::Format::e;
				case PixelFormat::R32_UInt:
					return vk::Format::e;
				case PixelFormat::R32_SInt:
					return vk::Format::e;
				case PixelFormat::R8_G8_UNorm:
					return vk::Format::e;
				case PixelFormat::R8_G8_UInt:
					return vk::Format::e;
				case PixelFormat::R8_G8_SInt:
					return vk::Format::e;
				case PixelFormat::R16_G16_UNorm:
					return vk::Format::e;
				case PixelFormat::R16_G16_SNorm:
					return vk::Format::e;
				case PixelFormat::R16_G16_UInt:
					return vk::Format::e;
				case PixelFormat::R16_G16_SInt:
					return vk::Format::e;
				case PixelFormat::R16_G16_Float:
					return vk::Format::e;
				case PixelFormat::R32_G32_UInt:
					return vk::Format::e;
				case PixelFormat::R32_G32_SInt:
					return vk::Format::e;
				case PixelFormat::R32_G32_Float:
					return vk::Format::e;
				case PixelFormat::R8_G8_B8_A8_SNorm:
					return vk::Format::e;
				case PixelFormat::R8_G8_B8_A8_UInt:
					return vk::Format::e;
				case PixelFormat::R8_G8_B8_A8_SInt:
					return vk::Format::e;
				case PixelFormat::R16_G16_B16_A16_UNorm:
					return vk::Format::e;
				case PixelFormat::R16_G16_B16_A16_SNorm:
					return vk::Format::e;
				case PixelFormat::R16_G16_B16_A16_UInt:
					return vk::Format::e;
				case PixelFormat::R16_G16_B16_A16_SInt:
					return vk::Format::e;
				case PixelFormat::R16_G16_B16_A16_Float:
					return vk::Format::e;
				case PixelFormat::R32_G32_B32_A32_SInt:
					return vk::Format::e;
				case PixelFormat::R8_G8_B8_A8_UNorm_SRgb:
					return vk::Format::e;
				case PixelFormat::B8_G8_R8_A8_UNorm_SRgb:
					return vk::Format::e;
				*/
			case PixelFormat::None:
			default:
				// #TODO: Error. Unknown PixelFormat.
				return vk::Format::eUndefined;
		}
	}

	bool CreateSurface(vk::SurfaceKHR& outSurface, SurfaceProvider* surfaceProvider, vk::Instance instance)
	{
		if (auto* win32Provider = dynamic_cast<Win32Surface*>(surfaceProvider))
		{
			vk::Win32SurfaceCreateInfoKHR surfaceInfo{};
			surfaceInfo.setHinstance(win32Provider->HInstance);
			surfaceInfo.setHwnd(win32Provider->HWnd);
			outSurface = instance.createWin32SurfaceKHR(surfaceInfo);
		}
		if (auto* waylandProvider = dynamic_cast<WaylandSurface*>(surfaceProvider))
		{
			vk::WaylandSurfaceCreateInfoKHR surfaceInfo{};
			surfaceInfo.setDisplay(waylandProvider->Display);
			surfaceInfo.setSurface(waylandProvider->Surface);
			outSurface = instance.createWaylandSurfaceKHR(surfaceInfo);
		}

		return !!outSurface;
	}

	auto CreateTexturesFromSwapchainImages(Swapchain_T& swapchain) -> std::vector<Texture>
	{
		auto swapchainImages = swapchain.GraphicsDevice->Device.getSwapchainImagesKHR(swapchain.Swapchain);

		std::vector<Texture> textures(swapchainImages.size());
		for (auto i = 0; i < swapchainImages.size(); ++i)
		{
			swapchain.GraphicsDevice->Textures.push_back(std::make_unique<Texture_T>());
			auto& texture = *swapchain.GraphicsDevice->Textures.back();
			texture.graphicsDevice = swapchain.GraphicsDevice;
			texture.Image = swapchainImages[i];
			texture.Width = swapchain.Width;
			texture.Height = swapchain.Height;
			texture.Depth = 1;
			texture.MipLevels = 1;
			texture.ArrayLayers = 1;
			texture.Format = swapchain.Format;
			texture.Usage = vk::ImageUsageFlagBits::eColorAttachment;
			textures[i] = &texture;
		}
		return textures;
	}

	void CheckSubmittedCmdBuffers(CommandList_T& commandList)
	{
		while (!commandList.SubmittedCmdBuffers.empty())
		{
			auto submittedFence = commandList.SubmittedCmdBuffers.front();
			if (commandList.GraphicsDevice->Device.getFenceStatus(submittedFence.Fence) == vk::Result::eSuccess)
			{
				commandList.SubmittedCmdBuffers.pop();

				commandList.AvailableCmdLists.push(submittedFence.CmdBuffer);

				std::lock_guard lock(commandList.GraphicsDevice->Mutex);
				commandList.GraphicsDevice->AvailableFences.push(submittedFence.Fence);
			}
		}
	}

	void ClearCachedCmdListState(CommandList_T& commandList)
	{
		commandList.BoundFramebuffer = nullptr;
	}

} // namespace VkMana