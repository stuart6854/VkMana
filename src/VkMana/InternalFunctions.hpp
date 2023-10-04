#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include <optional>
#include <unordered_set>
#include <vector>

namespace VkMana
{
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

		vk::PhysicalDeviceFeatures features{};
		features.setFillModeNonSolid(VK_TRUE);
		features.setWideLines(VK_TRUE);

		vk::DeviceCreateInfo deviceInfo{};
		deviceInfo.setQueueCreateInfos(queueInfos);
		deviceInfo.setPEnabledExtensionNames(extensions);
		deviceInfo.setPEnabledFeatures(&features);
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
} // namespace VkMana