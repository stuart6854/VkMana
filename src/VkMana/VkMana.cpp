#include "VkMana/VkMana.hpp"

#include "InternalFunctions.hpp"

#include <vulkan/vulkan.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

#include <mutex>
#include <memory>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace VkMana
{
	struct Context_T
	{
		std::mutex Mutex;
		std::vector<std::unique_ptr<GraphicsDevice_T>> GraphicsDevices;
	};
	auto GetContext() -> Context_T&
	{
		static Context_T sContext = {};
		return sContext;
	}

	struct GraphicsDevice_T
	{
		std::mutex Mutex;
		vk::Instance Instance;
		vk::PhysicalDevice PhysicalDevice;
		vk::Device Device;
		QueueFamilyIndices QueueFamilyIndices;
		vk::Queue GraphicsQueue;
		vma::Allocator Allocator;
	};
	struct DeviceBuffer_T
	{
		std::mutex Mutex;
		GraphicsDevice graphicsDevice = nullptr;
		vk::Buffer Buffer;
		vma::Allocation Allocation;
		std::uint64_t Size = 0;
		BufferUsage Usage = {};
	};
	struct Texture_T
	{
		std::mutex Mutex;
		GraphicsDevice graphicsDevice = nullptr;
		vk::Image Image;
		vma::Allocation Allocation;
		std::uint32_t Width = 0;
		std::uint32_t Height = 0;
		std::uint32_t Depth = 0;
		std::uint32_t MipLevels = 0;
		std::uint32_t ArrayLayers = 0;
		PixelFormat Format = {};
		TextureUsage Usage = {};
		TextureType Type = {};
	};

	auto CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) -> GraphicsDevice
	{
		auto& context = GetContext();
		std::lock_guard lock(context.Mutex);

		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		context.GraphicsDevices.push_back(std::make_unique<GraphicsDevice_T>());
		auto& graphicsDevice = *context.GraphicsDevices.back();
		if (!CreateInstance(graphicsDevice.Instance, createInfo.Debug, {}))
		{
			// #TODO: Error. Failed to create Vulkan instance.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(graphicsDevice.Instance);

		if (!CreatePhysicalDevice(graphicsDevice.PhysicalDevice, graphicsDevice.Instance))
		{
			// #TODO: Error. Failed to create Vulkan physical device.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}

		graphicsDevice.QueueFamilyIndices = GetQueueFamilyIndices(graphicsDevice.PhysicalDevice, {});

		if (!CreateLogicalDevice(graphicsDevice.Device, graphicsDevice.PhysicalDevice, graphicsDevice.QueueFamilyIndices, {}))
		{
			// #TODO: Error. Failed to create Vulkan logical device.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(graphicsDevice.Device);

		if (!CreateAllocator(graphicsDevice.Allocator, graphicsDevice.Instance, graphicsDevice.PhysicalDevice, graphicsDevice.Device))
		{
			// #TODO: Error. Failed to create Vulkan allocator.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}

		return &graphicsDevice;
	}

	auto CreateBuffer(GraphicsDevice graphicsDevice, const BufferCreateInfo& createInfo) -> DeviceBuffer
	{
		return nullptr;
	}

	auto CreateTexture(GraphicsDevice graphicsDevice, const TextureCreateInfo& createInfo) -> Texture
	{
		return nullptr;
	}

	bool DestroyTexture(Texture texture)
	{
		if (texture == nullptr)
			return false;

		std::lock_guard lock(texture->Mutex);
		return true;
	}

	bool DestroyBuffer(DeviceBuffer buffer)
	{
		if (buffer == nullptr)
			return false;

		std::lock_guard lock(buffer->Mutex);
		return true;
	}

	bool DestroyGraphicDevice(GraphicsDevice graphicsDevice)
	{
		if (graphicsDevice == nullptr)
			return false;

		auto& context = GetContext();

		{
			std::lock_guard lock(graphicsDevice->Mutex);
			graphicsDevice->Allocator.destroy();
			graphicsDevice->Device.destroy();
			graphicsDevice->Instance.destroy();
		}

		{
			std::lock_guard lock(context.Mutex);
			for (auto i = 0; i < context.GraphicsDevices.size(); ++i)
			{
				auto* gds = context.GraphicsDevices[i].get();
				if (gds == graphicsDevice)
				{
					context.GraphicsDevices.erase(context.GraphicsDevices.begin() + i);
					break;
				}
			}
		}

		return true;
	}

} // namespace VkMana