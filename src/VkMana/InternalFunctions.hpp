#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include <optional>
#include <unordered_set>
#include <vector>

namespace VkMana::Internal
{
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

	bool CreateInstance(vk::Instance& outInstance, bool debug, std::vector<const char*> extensions)
	{
		vk::ApplicationInfo appInfo{};
		appInfo.setApiVersion(VK_API_VERSION_1_3);
		appInfo.setPApplicationName("__app_name__");
		appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
		appInfo.setPEngineName("__engine_name__");
		appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

		if (debug)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME); // #TODO: Check for extension support.
		// extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME); // #TODO: Check for extension support.

		std::vector<const char*> layers;
		if (debug)
		{
			layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		vk::InstanceCreateInfo instanceInfo{};
		instanceInfo.setPApplicationInfo(&appInfo);
		instanceInfo.setPEnabledExtensionNames(extensions);
		instanceInfo.setPEnabledLayerNames(layers);
		outInstance = vk::createInstance(instanceInfo);
		return !!outInstance;
	}

	bool CreatePhysicalDevice(vk::PhysicalDevice& outPhysicalDevice, vk::Instance instance)
	{
		auto availablePhysicalDevices = instance.enumeratePhysicalDevices();
		if (availablePhysicalDevices.empty())
			return false;

		vk::PhysicalDevice bestDevice;
		std::uint32_t bestScore = 0;
		for (auto i = 0; i < availablePhysicalDevices.size(); ++i)
		{
			auto physicalDevice = availablePhysicalDevices[i];
			std::uint32_t score = 0;

			auto props = physicalDevice.getProperties();
			if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
				score += 1000;
			else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
				score += 500;

			// score += props.limits.maxImageDimension2D;
			// score += props.limits.maxBoundDescriptorSets;
			// score += props.limits.maxDescriptorSetSampledImages;
			// score += props.limits.maxDescriptorSetUniformBuffers;
			// score += props.limits.maxDescriptorSetStorageBuffers;

			auto memoryProps = physicalDevice.getMemoryProperties();

			std::uint64_t deviceLocalHeapSize = 0;
			for (auto& heap : memoryProps.memoryHeaps)
			{
				if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
				{
					deviceLocalHeapSize += heap.size;
				}
			}
			score += (deviceLocalHeapSize / 1024 / 1024); // Megabytes

			if (score > bestScore)
			{
				bestScore = score;
				bestDevice = physicalDevice;
			}
		}

		outPhysicalDevice = bestDevice;
		return true;
	}

	struct QueueFamilyIndices
	{
		std::int32_t Graphics = -1;
		std::int32_t Present = -1;
	};
	auto GetQueueFamilyIndices(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) -> QueueFamilyIndices
	{
		auto queueFamilyProps = physicalDevice.getQueueFamilyProperties();

		QueueFamilyIndices queueFamilies;
		for (auto i = 0; i < queueFamilyProps.size(); ++i)
		{
			auto family = queueFamilyProps[i];
			if (queueFamilies.Graphics == -1 && family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				queueFamilies.Graphics = i;
			}

			if (surface && queueFamilies.Present == -1)
			{
				bool supportsPresent = physicalDevice.getSurfaceSupportKHR(i, surface);
				if (supportsPresent)
					queueFamilies.Present = i;
			}
		}

		return queueFamilies;
	}

	bool CreateLogicalDevice(vk::Device& outDevice,
		vk::PhysicalDevice physicalDevice,
		QueueFamilyIndices queueFamilyIndices,
		std::vector<const char*> extensions)
	{
		std::unordered_set<std::int32_t> queueIndices{};
		if (queueFamilyIndices.Graphics != -1)
			queueIndices.insert(queueFamilyIndices.Graphics);
		if (queueFamilyIndices.Present != -1)
			queueIndices.insert(queueFamilyIndices.Present);

		float priority = 1.0f;
		std::vector<vk::DeviceQueueCreateInfo> queueInfos;
		for (auto familyIndex : queueIndices)
		{
			auto& queueInfo = queueInfos.emplace_back();
			queueInfo.setQueueFamilyIndex(familyIndex);
			queueInfo.setQueueCount(1);
			queueInfo.setPQueuePriorities(&priority);
		}

		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vk::PhysicalDeviceDynamicRenderingFeatures dynRenderFeature{};
		dynRenderFeature.setDynamicRendering(VK_TRUE);

		vk::PhysicalDeviceFeatures features{};
		features.setFillModeNonSolid(VK_TRUE);
		features.setWideLines(VK_TRUE);

		vk::DeviceCreateInfo deviceInfo{};
		deviceInfo.setQueueCreateInfos(queueInfos);
		deviceInfo.setPEnabledExtensionNames(extensions);
		deviceInfo.setPEnabledFeatures(&features);
		deviceInfo.setPNext(&dynRenderFeature);
		outDevice = physicalDevice.createDevice(deviceInfo);
		return !!outDevice;
	}

	bool CreateAllocator(vma::Allocator& outAllocator, vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device)
	{
		vma::VulkanFunctions functions{};
		functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

		vma::AllocatorCreateInfo allocInfo{};
		allocInfo.setInstance(instance);
		allocInfo.setPhysicalDevice(physicalDevice);
		allocInfo.setDevice(device);
		allocInfo.setVulkanApiVersion(VK_API_VERSION_1_3);
		allocInfo.setPVulkanFunctions(&functions);
		outAllocator = vma::createAllocator(allocInfo);
		return outAllocator;
	}

	auto SelectSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool colorSrgb)
	{
		auto supportedFormats = physicalDevice.getSurfaceFormatsKHR(surface);

		auto desiredFormat = colorSrgb ? vk::Format::eB8G8R8A8Srgb : vk::Format::eB8G8R8A8Unorm;
		if (supportedFormats.size() == 1 && supportedFormats[0].format == vk::Format::eUndefined)
		{
			return vk::SurfaceFormatKHR(desiredFormat, vk::ColorSpaceKHR::eSrgbNonlinear);
		}

		for (auto& supportedFormat : supportedFormats)
		{
			if (supportedFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear && supportedFormat.format == desiredFormat)
			{
				return supportedFormat;
			}
		}

		if (colorSrgb)
		{
			// #TODO: Error. sRGB swapchain surface format not supported.
		}

		return supportedFormats[0];
	}

	auto SelectPresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool vsync) -> vk::PresentModeKHR
	{
		auto supportedPresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

		if (vsync)
		{
			/* VSync On */

			const auto it = std::find(supportedPresentModes.begin(), supportedPresentModes.end(), vk::PresentModeKHR::eFifoRelaxed);
			if (it != supportedPresentModes.end())
				return vk::PresentModeKHR::eFifoRelaxed; // Adaptive VSync

			return vk::PresentModeKHR::eFifo;
		}

		/* VSync Off */

		auto it = std::find(supportedPresentModes.begin(), supportedPresentModes.end(), vk::PresentModeKHR::eMailbox);
		if (it != supportedPresentModes.end())
		{
			return vk::PresentModeKHR::eMailbox; // Fast VSync
		}

		it = std::find(supportedPresentModes.begin(), supportedPresentModes.end(), vk::PresentModeKHR::eImmediate);
		if (it != supportedPresentModes.end())
			return vk::PresentModeKHR::eImmediate;

		return vk::PresentModeKHR::eFifo;
	}

	bool CreateSwapchain(vk::SwapchainKHR& outSwapchain,
		vk::Format& outFormat,
		vk::Device device,
		vk::SurfaceKHR surface,
		std::uint32_t width,
		std::uint32_t height,
		bool vsync,
		vk::SwapchainKHR oldSwapchain,
		bool colorSrgb,
		vk::PhysicalDevice physicalDevice)
	{
		auto surfaceCaps = physicalDevice.getSurfaceCapabilitiesKHR(surface);

		auto surfaceFormat = SelectSurfaceFormat(physicalDevice, surface, colorSrgb);
		auto presentMode = SelectPresentMode(physicalDevice, surface, vsync);

		std::uint32_t maxImageCount = surfaceCaps.maxImageCount == 0 ? std::uint32_t(-1) : surfaceCaps.maxImageCount;
		std::uint32_t imageCount = std::min(surfaceCaps.minImageCount + 1, maxImageCount);

		auto clampedWidth = std::clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
		auto clampedHeight = std::clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

		vk::SwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.setSurface(surface);
		swapchainInfo.setImageFormat(surfaceFormat.format);
		swapchainInfo.setImageColorSpace(surfaceFormat.colorSpace);
		swapchainInfo.setImageExtent({ clampedWidth, clampedHeight });
		swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
		swapchainInfo.setImageArrayLayers(1);
		swapchainInfo.setMinImageCount(imageCount);
		swapchainInfo.setPresentMode(presentMode);
		swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
		swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapchainInfo.setClipped(VK_FALSE);
		swapchainInfo.setOldSwapchain(oldSwapchain);
		outSwapchain = device.createSwapchainKHR(swapchainInfo);
		outFormat = surfaceFormat.format;
		return !!outSwapchain;
	}

	bool CreateDeviceBuffer(
		vk::Buffer& outBuffer, vma::Allocation& outAllocation, vma::Allocator allocator, std::uint64_t size, BufferUsage usage)
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.setSize(size);
		if (usage == BufferUsage::Vertex)
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
		else if (usage == BufferUsage::Index)
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eIndexBuffer);
		else if (usage == BufferUsage::Uniform)
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eUniformBuffer);
		else if (usage == BufferUsage::Storage)
			bufferInfo.setUsage(vk::BufferUsageFlagBits::eStorageBuffer);

		vma::AllocationCreateInfo allocInfo{};
		allocInfo.setUsage(vma::MemoryUsage::eAutoPreferDevice);

		std::tie(outBuffer, outAllocation) = allocator.createBuffer(bufferInfo, allocInfo);
		return outBuffer != nullptr && outAllocation != nullptr;
	}

	bool CreateTexture(vk::Image& outImage,
		vma::Allocation& outAllocation,
		vma::Allocator allocator,
		std::uint32_t width,
		std::uint32_t height,
		std::uint32_t depth,
		std::uint32_t mipLevels,
		std::uint32_t arrayLayers,
		vk::Format format,
		TextureUsage usage)
	{
		vk::ImageCreateInfo imageInfo{};
		imageInfo.setExtent({ width, height, depth });
		imageInfo.setMipLevels(mipLevels);
		imageInfo.setArrayLayers(arrayLayers);
		imageInfo.setFormat(format);

		if (usage == TextureUsage::Sampled)
			imageInfo.setUsage(vk::ImageUsageFlagBits::eSampled);
		else if (usage == TextureUsage::Storage)
			imageInfo.setUsage(vk::ImageUsageFlagBits::eStorage);
		else if (usage == TextureUsage::RenderTarget)
			imageInfo.setUsage(vk::ImageUsageFlagBits::eColorAttachment);
		else if (usage == TextureUsage::DepthStencil)
			imageInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
		//		else if (usage == TextureUsage::CubeMap)
		//			imageInfo.setUsage(vk::ImageUsageFlagBits::eSampled);

		if (width > 1 && height > 1 && depth > 1)
			imageInfo.setImageType(vk::ImageType::e3D);
		else if (width > 1 && height > 1)
			imageInfo.setImageType(vk::ImageType::e2D);
		else if (width > 1)
			imageInfo.setImageType(vk::ImageType::e1D);

		vma::AllocationCreateInfo allocInfo{};
		allocInfo.setUsage(vma::MemoryUsage::eAutoPreferDevice);

		std::tie(outImage, outAllocation) = allocator.createImage(imageInfo, allocInfo);
		return outImage != nullptr && outAllocation != nullptr;
	}

	bool CreateCommandPool(vk::CommandPool& cmdPool, vk::Device device)
	{
		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		cmdPool = device.createCommandPool(poolInfo);
		return !!cmdPool;
	}

	bool AllocateCommandBuffer(vk::CommandBuffer& outCmdBuffer, vk::Device device, vk::CommandPool cmdPool)
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.setCommandPool(cmdPool);
		allocInfo.setCommandBufferCount(1);
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		outCmdBuffer = device.allocateCommandBuffers(allocInfo)[0];
		return !!outCmdBuffer;
	}

	bool CreateFence(vk::Fence& outFence, vk::Device device)
	{
		vk::FenceCreateInfo fenceInfo{};
		outFence = device.createFence(fenceInfo);
		return !!outFence;
	}

	bool CreateImageView(vk::ImageView& outView,
		vk::Device device,
		vk::Image image,
		vk::ImageViewType type,
		vk::Format format,
		vk::ImageSubresourceRange subresource)
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.setImage(image);
		viewInfo.setViewType(type);
		viewInfo.setFormat(format);
		viewInfo.setSubresourceRange(subresource);
		outView = device.createImageView(viewInfo);
		return !!outView;
	}

} // namespace VkMana::Internal