#include "Context.hpp"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace VkMana
{
#ifdef VULKAN_DEBUG
	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		auto* context = static_cast<Context*>(pUserData);

		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
				{
					LOG_ERR("[Vulkan]: Validation Error: {}", pCallbackData->pMessage);
					// context->notify_validation_error(pCallbackData->pMessage);
				}
				else
					LOG_ERR("[Vulkan]: Other Error: {}", pCallbackData->pMessage);
				break;

			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
					LOG_WARN("[Vulkan]: Validation Warning: {}", pCallbackData->pMessage);
				else
					LOG_WARN("[Vulkan]: Other Warning: {}", pCallbackData->pMessage);
				break;

	#if 0
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			LOGI("[Vulkan]: Validation Info: {}", pCallbackData->pMessage);
		else
			LOGI("[Vulkan]: Other Info: {}", pCallbackData->pMessage);
		break;
	#endif

			default:
				return VK_FALSE;
		}

		bool log_object_names = false;
		for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
		{
			auto* name = pCallbackData->pObjects[i].pObjectName;
			if (name)
			{
				log_object_names = true;
				break;
			}
		}

		if (log_object_names)
		{
			for (uint32_t i = 0; i < pCallbackData->objectCount; i++)
			{
				auto* name = pCallbackData->pObjects[i].pObjectName;
				LOG_INFO("  Object #{}: {}", i, name ? name : "N/A");
			}
		}

		return VK_FALSE;
	}
#endif

	static std::uint32_t GetGPUScore(vk::PhysicalDevice gpu)
	{
		auto props = gpu.getProperties();

		if (props.apiVersion < VK_API_VERSION_1_3)
			return 0;

		switch (props.deviceType)
		{
			case vk::PhysicalDeviceType::eDiscreteGpu:
				return 3;
			case vk::PhysicalDeviceType::eIntegratedGpu:
				return 2;
			case vk::PhysicalDeviceType::eCpu:
				return 1;
			default:
				return 0;
		}
	}

	QueueInfo::QueueInfo()
	{
		for (auto& index : FamilyIndices)
			index = VK_QUEUE_FAMILY_IGNORED;
	}

	Context::Context() = default;

	Context::~Context()
	{
		Destroy();
	}

	bool Context::InitInstance(const std::vector<const char*>& exts)
	{
		Destroy();

		if (!CreateInstance(exts))
		{
			Destroy();
			LOG_ERR("Failed to create Vulkan instance.");
			return false;
		}
		return true;
	}

	bool Context::InitDevice(vk::PhysicalDevice gpu, vk::SurfaceKHR surface, const std::vector<const char*>& exts)
	{
		vk::PhysicalDeviceFeatures features{};
		if (!CreateDevice(gpu, surface, exts, &features))
		{
			Destroy();
			LOG_ERR("Failed to create Vulkan device.");
			return false;
		}

		return true;
	}

	bool Context::InitInstanceAndDevice(const std::vector<const char*>& instanceExts, const std::vector<const char*>& deviceExts)
	{
		if (!InitInstance(instanceExts))
			return false;
		if (!InitDevice({}, {}, deviceExts))
			return false;

		return true;
	}

	bool Context::CreateInstance(const std::vector<const char*>& exts)
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		vk::ApplicationInfo appInfo{};
		appInfo.setApiVersion(VK_API_VERSION_1_3);

		vk::InstanceCreateInfo instanceInfo{};
		instanceInfo.setPApplicationInfo(&appInfo);

		std::vector<const char*> instanceExts;
		std::vector<const char*> instanceLayers;
		instanceExts.insert(instanceExts.end(), exts.begin(), exts.end());

		auto supportedLayers = vk::enumerateInstanceLayerProperties();
		auto supportedExts = vk::enumerateInstanceExtensionProperties();

		LOG_INFO("Layer count: {}", supportedLayers.size());
		for (auto& layer : supportedLayers)
			LOG_INFO("Found layer: {}.", layer.layerName.data());

		const auto hasExtension = [&](const char* name) -> bool {
			auto itr = std::find_if(supportedExts.begin(), supportedExts.end(), [name](const vk::ExtensionProperties& e) -> bool {
				return std::strcmp(e.extensionName, name) == 0;
			});
			return itr != supportedExts.end();
		};

		for (auto i = 0; i < instanceExts.size(); ++i)
			if (!hasExtension(instanceExts[i]))
				return false;

		if (hasExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
		{
			instanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			m_ext.SupportsDebugUtils = true;
		}

#ifdef VULKAN_DEBUG
		const auto hasLayer = [&](const char* name) -> bool {
			auto itr = std::find_if(supportedLayers.begin(), supportedLayers.end(), [name](const vk::LayerProperties& l) -> bool {
				return std::strcmp(l.layerName, name) == 0;
			});
			return itr != supportedLayers.end();
		};

		vk::ValidationFeaturesEXT validationFeatures{};

		if (hasLayer("VK_LAYER_KHRONOS_validation"))
		{
			instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
			LOG_INFO("Enabling VK_LAYER_KHRONOS_validation.");

			auto layerExts = vk::enumerateInstanceExtensionProperties({ "VK_LAYER_KHRONOS_validation" });
			if (std::find_if(layerExts.begin(),
					layerExts.end(),
					[](const vk::ExtensionProperties& e) {
						return std::strcmp(e.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0;
					})
				!= layerExts.end())
			{
				instanceExts.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
				static const std::vector<vk::ValidationFeatureEnableEXT> validationSyncFeatures{
					vk::ValidationFeatureEnableEXT::eSynchronizationValidation,
				};
				LOG_INFO("Enabling VK_EXT_validation_features for synchronization validation.");
				validationFeatures.setEnabledValidationFeatures(validationSyncFeatures);
				instanceInfo.setPNext(&validationFeatures);
			}

			if (!m_ext.SupportsDebugUtils && std::find_if(layerExts.begin(), layerExts.end(), [](const vk::ExtensionProperties& e) {
					return std::strcmp(e.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
				}) != layerExts.end())
			{
				instanceExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				m_ext.SupportsDebugUtils = true;
			}
		}
#endif

		instanceInfo.setPEnabledExtensionNames(instanceExts);
		instanceInfo.setPEnabledLayerNames(instanceLayers);

		for (auto* extName : instanceExts)
			LOG_INFO("Enabling instance extention: {}.", extName);

		m_instance = vk::createInstance(instanceInfo);
		if (!m_instance)
			return false;

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_instance);

#ifdef VULKAN_DEBUG
		if (m_ext.SupportsDebugUtils)
		{
			vk::DebugUtilsMessengerCreateInfoEXT debugInfo{};
			debugInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
				| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning);
			debugInfo.setPfnUserCallback(&VulkanMessengerCallback);
			debugInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
				| vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral);
			debugInfo.setPUserData(this);

			m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugInfo);
		}
#endif

		return true;
	}

	bool Context::CreateDevice(vk::PhysicalDevice gpu,
		vk::SurfaceKHR surface,
		const std::vector<const char*>& requiredExts,
		const vk::PhysicalDeviceFeatures* requiredFeatures)
	{
		m_physicalDevice = gpu;
		if (!m_physicalDevice)
		{
			auto gpus = m_instance.enumeratePhysicalDevices();
			if (gpus.empty())
				return false;

			for (auto gpu : gpus)
			{
				auto props = gpu.getProperties();
				LOG_INFO("Found Vulkan GPU {}", props.deviceName.data());
				LOG_INFO("	API: {}.{}.{}",
					VK_VERSION_MAJOR(props.apiVersion),
					VK_VERSION_MINOR(props.apiVersion),
					VK_VERSION_PATCH(props.apiVersion));
				LOG_INFO("	Driver: {}.{}.{}",
					VK_VERSION_MAJOR(props.driverVersion),
					VK_VERSION_MINOR(props.driverVersion),
					VK_VERSION_PATCH(props.driverVersion));
			}

			std::uint32_t maxScore = 0;
			// Prefer earlier entries in list
			for (auto i = gpus.size(); i; --i)
			{
				auto score = GetGPUScore(gpus[i - 1]);
				if (score >= maxScore && DoesPhysicalDeviceSupportSurface(gpus[i - 1], surface))
				{
					maxScore = score;
					m_physicalDevice = gpus[i - 1];
				}
			}

			if (!m_physicalDevice)
			{
				LOG_ERR("No GPU found which supports surface.");
				return false;
			}
		}
		else if (!DoesPhysicalDeviceSupportSurface(m_physicalDevice, surface))
		{
			LOG_ERR("Selected GPU does not support surface.");
			return false;
		}

		auto supportedExts = m_physicalDevice.enumerateDeviceExtensionProperties();

		auto hasExtension = [&](const char* name) -> bool {
			auto itr = std::find_if(supportedExts.begin(), supportedExts.end(), [name](const vk::ExtensionProperties& e) {
				return std::strcmp(e.extensionName, name) == 0;
			});
			return itr != supportedExts.end();
		};

		for (const auto& ext : requiredExts)
			if (!hasExtension(ext))
				return false;

		std::vector<const char*> enabledExts;
		bool requiresSwapchain = false;
		for (auto i = 0; i < requiredExts.size(); ++i)
		{
			enabledExts.push_back(requiredExts[i]);
			if (std::strcmp(requiredExts[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
				requiresSwapchain = true;
		}

		auto gpuProps = m_physicalDevice.getProperties();
		LOG_INFO("Using Vulkan GPU: {}.", gpuProps.deviceName.data());

		auto queueProps = m_physicalDevice.getQueueFamilyProperties();
		std::vector<std::uint32_t> queueOffsets(queueProps.size());
		std::vector<std::vector<float>> queuePriorities(queueProps.size());
		std::array<std::uint32_t, QueueIndex_Count> queueIndices{};

		const auto findVacantQueue = [&](std::uint32_t& family,
										 std::uint32_t& index,
										 vk::QueueFlags requiredFlags,
										 vk::QueueFlags ignoreFlags,
										 float priority) -> bool {
			for (auto familyIndex = 0; familyIndex < queueProps.size(); ++familyIndex)
			{
				if (queueProps[familyIndex].queueFlags & ignoreFlags)
					continue;

				// A graphics queue candidate must support present for us to select it
				if (requiredFlags & vk::QueueFlagBits::eGraphics && surface)
				{
					if (m_physicalDevice.getSurfaceSupportKHR(familyIndex, surface))
						continue;
				}

				if (queueProps[familyIndex].queueCount && queueProps[familyIndex].queueFlags & requiredFlags)
				{
					family = familyIndex;
					queueProps[familyIndex].queueCount--;
					index = queueOffsets[familyIndex]++;
					queuePriorities[familyIndex].push_back(priority);
					return true;
				}
			}
			return false;
		};

		if (!findVacantQueue(m_queueInfo.FamilyIndices[QueueIndex_Graphics],
				queueIndices[QueueIndex_Graphics],
				vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute,
				{},
				0.5f))
		{
			LOG_ERR("Could not find suitable graphics queue.");
			return false;
		}

		// Prefer another graphics queue since we can do async graphics that way.
		// The compute queue is to be treated as high priority since we also do async graphics on it.
		if (!findVacantQueue(m_queueInfo.FamilyIndices[QueueIndex_Compute],
				queueIndices[QueueIndex_Compute],
				vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute,
				{},
				1.0f)
			&& !findVacantQueue(
				m_queueInfo.FamilyIndices[QueueIndex_Compute], queueIndices[QueueIndex_Compute], vk::QueueFlagBits::eCompute, {}, 1.0f))
		{
			// Fallback to the graphics queue if we must
			m_queueInfo.FamilyIndices[QueueIndex_Compute] = m_queueInfo.FamilyIndices[QueueIndex_Graphics];
			queueIndices[QueueIndex_Compute] = queueIndices[QueueIndex_Graphics];
		}

		// For transfer, try to find a queue which only supports transfer, e.g. DMA queue.
		// If not, fallback to a dedicated compute queue.
		// Finally, fallback to same queue as compute.
		if (!findVacantQueue(m_queueInfo.FamilyIndices[QueueIndex_Transfer],
				queueIndices[QueueIndex_Transfer],
				vk::QueueFlagBits::eTransfer,
				vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute,
				0.5f)
			&& !findVacantQueue(m_queueInfo.FamilyIndices[QueueIndex_Transfer],
				queueIndices[QueueIndex_Transfer],
				vk::QueueFlagBits::eCompute,
				vk::QueueFlagBits::eCompute,
				0.5f))
		{
			// Fallback to the compute queue if we must
			m_queueInfo.FamilyIndices[QueueIndex_Transfer] = m_queueInfo.FamilyIndices[QueueIndex_Compute];
			queueIndices[QueueIndex_Transfer] = queueIndices[QueueIndex_Compute];
		}

		std::vector<vk::DeviceQueueCreateInfo> queueInfos{};
		for (auto familyIndex = 0; familyIndex < queueProps.size(); ++familyIndex)
		{
			if (queueOffsets[familyIndex] == 0)
				continue;

			auto& info = queueInfos.emplace_back();
			info.setQueueFamilyIndex(familyIndex);
			info.setQueueCount(queueOffsets[familyIndex]);
			info.setQueuePriorities(queuePriorities[familyIndex]);
		}

		vk::DeviceCreateInfo deviceInfo{};
		deviceInfo.setPEnabledExtensionNames(enabledExts);
		deviceInfo.setQueueCreateInfos(queueInfos);

		for (auto* ext : enabledExts)
			LOG_INFO("Enabling device extension: {}.", ext);

		m_device = m_physicalDevice.createDevice(deviceInfo);
		if (!m_device)
		{
			return false;
		}

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_device);

		for (auto i = 0; i < QueueIndex_Count; ++i)
		{
			if (m_queueInfo.FamilyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
			{
				m_queueInfo.Queues[i] = m_device.getQueue(m_queueInfo.FamilyIndices[i], queueIndices[i]);
			}
			else
			{
				m_queueInfo.Queues[i] = nullptr;
			}
		}

#ifdef VULKAN_DEBUG
		static const char* familyNames[QueueIndex_Count] = { "Graphics", "Compute", "Transfer" };

		for (auto i = 0; i < QueueIndex_Count; ++i)
			if (m_queueInfo.FamilyIndices[i] != VK_QUEUE_FAMILY_IGNORED)
				LOG_INFO("{} queue: family {}, index {}.", familyNames[i], m_queueInfo.FamilyIndices[i], queueIndices[i]);
#endif

		return true;
	}

	void Context::Destroy()
	{
		if (m_device)
			m_device.waitIdle();

		if (m_device)
			m_device.destroy();

#ifdef VULKAN_DEBUG
		if (m_debugMessenger)
			m_instance.destroy(m_debugMessenger);
		m_debugMessenger = nullptr;
#endif

		if (m_instance)
			m_instance.destroy();
	}

	bool Context::DoesPhysicalDeviceSupportSurface(vk::PhysicalDevice gpu, vk::SurfaceKHR surface)
	{
		if (!surface)
			return true;

		auto queueFamilyProps = gpu.getQueueFamilyProperties();
		for (auto i = 0; i < queueFamilyProps.size(); ++i)
		{
			// A graphics queue must support present
			if (queueFamilyProps[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				if (gpu.getSurfaceSupportKHR(i, surface))
					return true;
			}
		}
		return false;
	}

} // namespace VkMana