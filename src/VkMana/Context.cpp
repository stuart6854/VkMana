#include "Context.hpp"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace VkMana
{
	Context::~Context()
	{
		if (m_device)
			m_device.waitIdle();

		while (!m_surfaces.empty())
		{
			RemoveSurface(m_surfaces.front().WSI);
		}
		m_surfaces.clear();

		if (m_device)
		{
			for (auto& frame : m_frames)
			{
				m_device.destroy(frame.FrameFence);
			}
			m_frames.clear();

			if (m_allocator)
				m_allocator.destroy();

			m_device.destroy();
		}
		if (m_instance)
		{
			m_instance.destroy();
		}
	}

	bool Context::Init(WSI* mainWSI)
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		if (!InitInstance(m_instance))
			return false;
		if (!SelectGPU(m_gpu, m_instance))
			return false;
		if (!InitDevice(m_device, m_queueInfo, m_gpu))
			return false;
		if (!SetupFrames())
			return false;

		if (!AddSurface(mainWSI))
			return false;

		return true;
	}

	bool Context::AddSurface(WSI* wsi)
	{
		auto& newSurfaceInfo = m_surfaces.emplace_back();
		newSurfaceInfo.WSI = wsi;
		newSurfaceInfo.Surface = wsi->CreateSurface(m_instance);
		if (!CreateSwapchain(newSurfaceInfo.Swapchain,
				newSurfaceInfo.Surface,
				m_gpu,
				m_device,
				wsi->GetSurfaceWidth(),
				wsi->GetSurfaceHeight(),
				wsi->IsVSync(),
				true,
				{}))
		{
			m_instance.destroy(newSurfaceInfo.Surface);
			m_surfaces.erase(m_surfaces.end() - 1);
			return false;
		}

		auto swapchainImages = m_device.getSwapchainImagesKHR(newSurfaceInfo.Swapchain);
		newSurfaceInfo.AcquireSemaphores.resize(swapchainImages.size());
		for (auto i = 0; i < swapchainImages.size(); ++i)
		{
			newSurfaceInfo.AcquireSemaphores[i] = m_device.createSemaphore({});
		}

		return true;
	}

	void Context::RemoveSurface(WSI* wsi)
	{
		auto it = std::ranges::find_if(m_surfaces, [&](const SurfaceInfo& surfaceInfo) { return surfaceInfo.WSI == wsi; });
		if (it == m_surfaces.end())
			return;

		auto& surfaceInfo = *it;
		m_device.destroy(surfaceInfo.Swapchain);
		m_instance.destroy(surfaceInfo.Surface);

		m_surfaces.erase(it);
	}

	void Context::BeginFrame()
	{
		auto& frame = GetFrame();

		UNUSED(m_device.waitForFences(frame.FrameFence, true, UINT64_MAX));
		m_device.resetFences(frame.FrameFence);
		frame.CmdPool->ResetPool();

		for (auto& surfaceInfo : m_surfaces)
		{
			surfaceInfo.WSI->PollEvents();

			auto acquireSemaphore = m_device.createSemaphore({});
			auto result = m_device.acquireNextImageKHR(surfaceInfo.Swapchain, UINT64_MAX, acquireSemaphore);
			surfaceInfo.ImageIndex = result.value;
			m_submitWaitSemaphores.push_back(acquireSemaphore);
			m_submitWaitStageMasks.emplace_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		}
	}

	void Context::EndFrame()
	{
		auto& frame = GetFrame();

		m_queueInfo.GraphicsQueue.submit({}, frame.FrameFence);

		m_frameIndex = (m_frameIndex + 1) % m_frames.size();
	}

	void Context::Present()
	{
		std::vector<vk::SwapchainKHR> swapchains;
		std::vector<uint32_t> imageIndices;
		swapchains.reserve(m_surfaces.size());
		imageIndices.reserve(m_surfaces.size());
		for (auto& surface : m_surfaces)
		{
			swapchains.push_back(surface.Swapchain);
			imageIndices.push_back(surface.ImageIndex);
		}

		std::vector<vk::Result> results(swapchains.size());

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setWaitSemaphores({});
		presentInfo.setSwapchains(swapchains);
		presentInfo.setImageIndices(imageIndices);
		presentInfo.setResults(results);

		UNUSED(m_queueInfo.GraphicsQueue.presentKHR(presentInfo));
		for (auto i = 0; i < results.size(); ++i)
		{
			auto result = results[i];
			auto& surface = m_surfaces[i];
			// #TODO: Handle present result.
		}
	}

	auto Context::RequestCmd() -> CmdBuffer
	{
		auto cmd = GetFrame().CmdPool->RequestCmd();
		vk::CommandBufferBeginInfo beginInfo{};
		cmd.begin(beginInfo);
		return IntrusivePtr(new CommandBuffer(this, cmd));
	}

	void Context::Submit(CmdBuffer cmd)
	{
		auto commandBuffer = cmd->GetCmd();
		commandBuffer.end();

		vk::SubmitInfo submitInfo{};
		submitInfo.setWaitSemaphores(m_submitWaitSemaphores);
		submitInfo.setWaitDstStageMask(m_submitWaitStageMasks);
		submitInfo.setCommandBuffers(commandBuffer);
		m_queueInfo.GraphicsQueue.submit(submitInfo);

		m_submitWaitSemaphores.clear();
		m_submitWaitStageMasks.clear();
	}

	void Context::PrintInstanceInfo()
	{
		auto layers = vk::enumerateInstanceLayerProperties();
		LOG_INFO("{} Instance Layers:", layers.size());
		for (auto& layer : layers)
			LOG_INFO("  - {}", layer.layerName.data());

		auto exts = vk::enumerateInstanceExtensionProperties();
		LOG_INFO("{} Instance Extensions:", exts.size());
		for (auto& ext : exts)
			LOG_INFO("  - {}", ext.extensionName.data());
	}

	void Context::PrintDeviceInfo(vk::PhysicalDevice gpu)
	{
		auto exts = gpu.enumerateDeviceExtensionProperties();
		LOG_INFO("{} Device Extensions:", exts.size());
		for (auto& ext : exts)
			LOG_INFO("  - {}", ext.extensionName.data());
	}

	bool Context::FindQueueFamily(uint32_t outFamilyIndex, vk::PhysicalDevice gpu, vk::QueueFlags flags)
	{
		auto families = gpu.getQueueFamilyProperties();
		for (auto i = 0; i < families.size(); ++i)
		{
			if (families[i].queueFlags & flags)
			{
				outFamilyIndex = i;
				return true;
			}
		}

		return false;
	}

	bool Context::InitInstance(vk::Instance& outInstance)
	{
		// PrintInstanceInfo();

		vk::ApplicationInfo appInfo{};

		std::vector<const char*> enabledLayers{
#ifdef _DEBUG
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_KHRONOS_synchronization2",
#endif
		};
		std::vector<const char*> enabledExtensions{
			VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
		};

		vk::InstanceCreateInfo instanceInfo{};
		instanceInfo.setPApplicationInfo(&appInfo);
		instanceInfo.setPEnabledLayerNames(enabledLayers);
		instanceInfo.setPEnabledExtensionNames(enabledExtensions);
		outInstance = vk::createInstance(instanceInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(outInstance);

		LOG_INFO("Enabled Instance layers:");
		for (const auto* layer : enabledLayers)
			LOG_INFO("  - {}", layer);

		LOG_INFO("Enabled Instance extensions:");
		for (const auto* ext : enabledExtensions)
			LOG_INFO("  - {}", ext);

		return outInstance != VK_NULL_HANDLE;
	}

	bool Context::SelectGPU(vk::PhysicalDevice& outGPU, vk::Instance instance)
	{
		auto gpus = instance.enumeratePhysicalDevices();
		if (gpus.empty())
			return false;

		// #TODO: Prefer dedicated gpu with most device memory.
		outGPU = gpus[0];

		auto gpuProps = outGPU.getProperties();
		LOG_INFO("GPU - {}", gpuProps.deviceName.data());
		LOG_INFO("  API - {}.{}.{}",
			VK_API_VERSION_MAJOR(gpuProps.apiVersion),
			VK_API_VERSION_MINOR(gpuProps.apiVersion),
			VK_API_VERSION_PATCH(gpuProps.apiVersion));
		LOG_INFO("  Driver - {}.{}.{}",
			VK_API_VERSION_MAJOR(gpuProps.driverVersion),
			VK_API_VERSION_MINOR(gpuProps.driverVersion),
			VK_API_VERSION_PATCH(gpuProps.driverVersion));

		return outGPU != VK_NULL_HANDLE;
	}

	bool Context::InitDevice(vk::Device& outDevice, QueueInfo& outQueueInfo, vk::PhysicalDevice gpu)
	{
		// PrintDeviceInfo(gpu);

		/* Extensions */

		std::vector<const char*> enabledExtensions{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		/* Queues */

		if (!FindQueueFamily(outQueueInfo.GraphicsFamilyIndex, gpu, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer))
			return false;

		float queuePriority = 0.5f;
		std::vector<vk::DeviceQueueCreateInfo> queueInfos{};
		auto& graphicsQueueInfo = queueInfos.emplace_back();
		graphicsQueueInfo.setQueueFamilyIndex(outQueueInfo.GraphicsFamilyIndex);
		graphicsQueueInfo.setQueuePriorities(queuePriority);
		graphicsQueueInfo.setQueueCount(1);

		/* Features */

		vk::PhysicalDeviceFeatures enabledFeatures{};

		/* Extension Features */

		vk::PhysicalDeviceSynchronization2Features sync2Features{};
		sync2Features.setSynchronization2(VK_TRUE);

		vk::PhysicalDeviceDynamicRenderingFeatures dynRenderFeatures{};
		dynRenderFeatures.setDynamicRendering(VK_TRUE);
		dynRenderFeatures.setPNext(&sync2Features);

		/* Device Create Info */

		vk::DeviceCreateInfo deviceInfo{};
		deviceInfo.setPEnabledExtensionNames(enabledExtensions);
		deviceInfo.setQueueCreateInfos(queueInfos);
		deviceInfo.setPEnabledFeatures(&enabledFeatures);
		deviceInfo.setPNext(&dynRenderFeatures);
		outDevice = gpu.createDevice(deviceInfo);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(outDevice);

		outQueueInfo.GraphicsQueue = outDevice.getQueue(outQueueInfo.GraphicsFamilyIndex, 0);

		LOG_INFO("Enabled Device extensions:");
		for (const auto* ext : enabledExtensions)
			LOG_INFO("  - {}", ext);

		return outDevice != VK_NULL_HANDLE;
	}

	bool Context::SetupFrames()
	{
		m_frames.resize(2);
		for (auto& frame : m_frames)
		{
			frame.FrameFence = m_device.createFence({ vk::FenceCreateFlagBits::eSignaled });
			frame.CmdPool = IntrusivePtr(new CommandPool(this, m_queueInfo.GraphicsFamilyIndex));
		}

		m_frameIndex = 0;

		return true;
	}

	bool Context::CreateSwapchain(vk::SwapchainKHR& outSwapchain,
		vk::SurfaceKHR surface,
		vk::PhysicalDevice gpu,
		vk::Device device,
		uint32_t width,
		uint32_t height,
		bool vsync,
		bool srgb,
		vk::SwapchainKHR oldSwapchain)
	{
		if (!surface || !gpu || !device)
			return false;

		auto surfaceCaps = gpu.getSurfaceCapabilitiesKHR(surface);

		uint32_t minImageCount = surfaceCaps.minImageCount + 1;
		if (surfaceCaps.maxImageCount != 0 && minImageCount < surfaceCaps.maxImageCount)
			minImageCount = surfaceCaps.maxImageCount;

		auto surfaceFormat = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb);

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
		swapchainInfo.setOldSwapchain(oldSwapchain);
		outSwapchain = device.createSwapchainKHR(swapchainInfo);

		return outSwapchain != VK_NULL_HANDLE;
	}

} // namespace VkMana