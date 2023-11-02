#pragma once

#include "Vulkan_Headers.hpp"
#include "Vulkan_Common.hpp"

#include <array>

namespace VkMana
{
	struct DeviceFeatures
	{
		bool SupportsDebugUtils = false;
	};

	struct QueueInfo
	{
		QueueInfo();
		std::array<vk::Queue, QueueIndex_Count> Queues{};
		std::array<std::uint32_t, QueueIndex_Count> FamilyIndices{};
		std::array<std::uint32_t, QueueIndex_Count> Counts{};
	};

	/**
	 * The context is responsible for:
	 * 	- Creating VkInstance.
	 * 	- Creating VkDevice.
	 * 	- Setting up queues for graphics, compute & transfer.
	 * 	- Setting up validation layers.
	 * 	- Creating debug callback.
	 */
	class Context
	{
	public:
		Context();
		Context(const Context&) = delete;
		void operator=(const Context&) = delete;
		~Context();

		bool InitInstance(const std::vector<const char*>& exts);
		bool InitDevice(vk::PhysicalDevice gpu, vk::SurfaceKHR surface, const std::vector<const char*>& exts);

		bool InitInstanceAndDevice(const std::vector<const char*>& instanceExts, const std::vector<const char*>& deviceExts);

	private:
		bool CreateInstance(const std::vector<const char*>& exts);
		bool CreateDevice(vk::PhysicalDevice gpu,
			vk::SurfaceKHR surface,
			const std::vector<const char*>& requiredExts,
			const vk::PhysicalDeviceFeatures* requiredFeatures);

		void Destroy();

		static bool DoesPhysicalDeviceSupportSurface(vk::PhysicalDevice gpu, vk::SurfaceKHR surface);

	private:
		vk::Instance m_instance = nullptr;
		vk::PhysicalDevice m_physicalDevice = nullptr;
		vk::Device m_device = nullptr;
		vma::Allocator m_alloc;

#ifdef VULKAN_DEBUG
		vk::DebugUtilsMessengerEXT m_debugMessenger = nullptr;
#endif

		DeviceFeatures m_ext;
		QueueInfo m_queueInfo;
	};

} // namespace VkMana
