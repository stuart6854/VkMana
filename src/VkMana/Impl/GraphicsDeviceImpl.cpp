#include "GraphicsDeviceImpl.hpp"

#include "CommandListImpl.hpp"
#include "FenceImpl.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <cassert>
#include <set>

namespace VkMana
{
	GraphicsDevice::Impl::~Impl()
	{
		while (!m_freeFences.empty())
		{
			auto fence = m_freeFences.front();
			m_freeFences.pop();

			m_device.destroy(fence);
			GetVulkanStats().NumFences--;
		}

		if (m_device != nullptr)
			m_device.destroy();
		if (m_instance != nullptr)
			m_instance.destroy();
	}

	void GraphicsDevice::Impl::CreateInstance(bool debug, const std::vector<const char*>& instanceExtensions)
	{
		auto extensions = instanceExtensions;
		std::vector<const char*> layers;

		if (debug)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // #TODO: Check for DebugUtils extension support
			layers.push_back("VK_LAYER_KHRONOS_validation");		 // #TODO: Check for validation layer support
		}

		extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME); // #TODO: Check extensions supported
#ifdef _WIN32
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

		vk::ApplicationInfo appInfo{};
		appInfo.pEngineName = "__engine_name__";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pApplicationName = "__app_name__";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		vk::InstanceCreateInfo instanceInfo{};
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.setPEnabledExtensionNames(extensions);
		instanceInfo.setPEnabledLayerNames(layers);

		m_instance = vk::createInstance(instanceInfo);
		// #TODO: Handle failure

		// #TODO: Debug messenger
	}

	void GraphicsDevice::Impl::CreatePhysicalDevice()
	{
		auto physicalDevices = m_instance.enumeratePhysicalDevices();
		m_physicalDevice = physicalDevices.front();
		// #TODO: Pick best physical device (GPU)
	}

	void GraphicsDevice::Impl::CreateLogicalDevice(vk::SurfaceKHR surface, const std::vector<const char*>& deviceExtensions)
	{
		GetQueueFamilyIndices(surface);

		std::set<std::int32_t> familyIndices = { m_graphicsQueueIndex, m_presentQueueIndex };
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

		const float priority = 1.0f;
		for (auto index : familyIndices)
		{
			if (index < 0)
				continue;

			auto& queueInfo = queueCreateInfos.emplace_back();
			queueInfo.queueFamilyIndex = index;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &priority;
		}

		auto extensions = deviceExtensions;
		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME); // #TODO: Check extensions supported

		vk::DeviceCreateInfo deviceInfo{};
		deviceInfo.setQueueCreateInfos(queueCreateInfos);
		deviceInfo.setPEnabledExtensionNames(deviceExtensions);
		m_device = m_physicalDevice.createDevice(deviceInfo);
		// #TODO: Handle failure

		m_graphicsQueue = m_device.getQueue(m_graphicsQueueIndex, 0);
	}

	void GraphicsDevice::Impl::SubmitCommands(CommandList& commandList, Fence* fence)
	{
		CheckSubmittedFences();

		bool extraFence = fence != nullptr;

		auto submitFence = GetNextFreeFence();
		assert(submitFence != nullptr);

		auto cmdBuffer = commandList.GetImpl()->GetCmdBuffer();

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBuffers(cmdBuffer);
		m_graphicsQueue.submit(submitInfo, submitFence);

		if (extraFence)
		{
			m_graphicsQueue.submit({}, fence->GetImpl()->GetFence());
		}

		auto& fenceSubmitInfo = m_submittedFences.emplace();
		fenceSubmitInfo.fence = submitFence;
		fenceSubmitInfo.cmdList = &commandList;
		fenceSubmitInfo.cmdBuffer = cmdBuffer;

		commandList.GetImpl()->CommandBufferSubmitted(cmdBuffer);
	}

	void GraphicsDevice::Impl::GetQueueFamilyIndices(vk::SurfaceKHR surface)
	{
		auto queueFamilies = m_physicalDevice.getQueueFamilyProperties();

		bool foundGraphics = false;
		bool foundPresent = surface == VK_NULL_HANDLE;

		for (auto i = 0; i < queueFamilies.size(); ++i)
		{
			const auto& family = queueFamilies[i];
			if (family.queueFlags & vk::QueueFlagBits::eGraphics)
			{
				m_graphicsQueueIndex = i;
				foundGraphics = true;
			}

			if (!foundPresent)
			{
				bool presentSupported = m_physicalDevice.getSurfaceSupportKHR(i, surface) == VK_TRUE;
				if (presentSupported)
				{
					m_presentQueueIndex = i;
					foundPresent = true;
				}
			}

			if (foundGraphics && foundPresent)
			{
				return;
			}
		}
	}

	auto GraphicsDevice::Impl::GetNextFreeFence() -> vk::Fence
	{
		if (!m_freeFences.empty())
		{
			auto fence = m_freeFences.front();
			m_freeFences.pop();
			m_device.resetFences(fence);
			return fence;
		}

		vk::FenceCreateInfo fenceInfo{};
		auto fence = m_device.createFence(fenceInfo);

		GetVulkanStats().NumFences++;
		return fence;
	}

	void GraphicsDevice::Impl::CheckSubmittedFences()
	{
		while (!m_submittedFences.empty())
		{
			auto& submittedFence = m_submittedFences.front();

			auto fence = submittedFence.fence;
			auto status = m_device.getFenceStatus(fence);
			if (status != vk::Result::eSuccess)
			{
				// No need to check other fences if this is not signaled.
				break;
			}

			CompleteFenceSubmission(submittedFence);
			m_submittedFences.pop();
		}
	}

	void GraphicsDevice::Impl::CompleteFenceSubmission(GraphicsDevice::Impl::FenceSubmitInfo info)
	{
		m_freeFences.push(info.fence);
		info.cmdList->GetImpl()->CommandBufferCompleted(info.cmdBuffer);
	}

} // namespace VkMana
