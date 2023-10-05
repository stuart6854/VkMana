#include "VkMana/VkMana.hpp"

#include "InternalFunctions.hpp"

#include <vulkan/vulkan.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

#include <queue>
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
		Internal::QueueFamilyIndices QueueFamilyIndices;
		vk::Queue GraphicsQueue;
		vma::Allocator Allocator;
		std::queue<vk::Fence> AvailableFences;
		std::vector<std::unique_ptr<DeviceBuffer_T>> Buffers;
		std::vector<std::unique_ptr<Texture_T>> Textures;
		std::vector<std::unique_ptr<CommandList_T>> CmdLists;
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
		// TextureType Type = {};
	};
	struct SubmittedCmdInfo
	{
		vk::Fence Fence;
		vk::CommandBuffer CmdBuffer;
	};
	/** A CommandList should only be used by one thread. */
	struct CommandList_T
	{
		GraphicsDevice GraphicsDevice = nullptr;
		vk::CommandPool CmdPool;
		vk::CommandBuffer CmdBuffer;
		std::queue<vk::CommandBuffer> AvailableCmdLists;
		std::queue<SubmittedCmdInfo> SubmittedCmdBuffers;
		bool HasBegun = false;
		bool HasEnded = false;
	};

	namespace
	{
		void CheckSubmittedCmdBuffers(CommandList commandList);
	}

	auto CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) -> GraphicsDevice
	{
		auto& context = GetContext();
		std::lock_guard lock(context.Mutex);

		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		context.GraphicsDevices.push_back(std::make_unique<GraphicsDevice_T>());
		auto& graphicsDevice = *context.GraphicsDevices.back();
		if (!Internal::CreateInstance(graphicsDevice.Instance, createInfo.Debug, {}))
		{
			// #TODO: Error. Failed to create Vulkan instance.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(graphicsDevice.Instance);

		if (!Internal::CreatePhysicalDevice(graphicsDevice.PhysicalDevice, graphicsDevice.Instance))
		{
			// #TODO: Error. Failed to create Vulkan physical device.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}

		graphicsDevice.QueueFamilyIndices = Internal::GetQueueFamilyIndices(graphicsDevice.PhysicalDevice, {});

		if (!CreateLogicalDevice(graphicsDevice.Device, graphicsDevice.PhysicalDevice, graphicsDevice.QueueFamilyIndices, {}))
		{
			// #TODO: Error. Failed to create Vulkan logical device.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}
		VULKAN_HPP_DEFAULT_DISPATCHER.init(graphicsDevice.Device);

		graphicsDevice.GraphicsQueue = graphicsDevice.Device.getQueue(graphicsDevice.QueueFamilyIndices.Graphics, 0);

		if (!Internal::CreateAllocator(
				graphicsDevice.Allocator, graphicsDevice.Instance, graphicsDevice.PhysicalDevice, graphicsDevice.Device))
		{
			// #TODO: Error. Failed to create Vulkan allocator.
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}

		return &graphicsDevice;
	}

	auto CreateBuffer(GraphicsDevice graphicsDevice, const BufferCreateInfo& createInfo) -> DeviceBuffer
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::lock_guard lock(graphicsDevice->Mutex);

		graphicsDevice->Buffers.push_back(std::make_unique<DeviceBuffer_T>());
		auto& buffer = *graphicsDevice->Buffers.back();

		buffer.graphicsDevice = graphicsDevice;

		if (!Internal::CreateDeviceBuffer(buffer.Buffer, buffer.Allocation, graphicsDevice->Allocator, createInfo.Size, createInfo.Usage))
		{
			// #TODO: Error. Failed to create Vulkan device buffer.
			DestroyBuffer(&buffer);
			return nullptr;
		}

		return &buffer;
	}

	auto CreateTexture(GraphicsDevice graphicsDevice, const TextureCreateInfo& createInfo) -> Texture
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::lock_guard lock(graphicsDevice->Mutex);

		graphicsDevice->Textures.push_back(std::make_unique<Texture_T>());
		auto& texture = *graphicsDevice->Textures.back();

		texture.graphicsDevice = graphicsDevice;

		if (!Internal::CreateTexture(texture.Image,
				texture.Allocation,
				graphicsDevice->Allocator,
				createInfo.Width,
				createInfo.Height,
				createInfo.Depth,
				createInfo.MipLevels,
				createInfo.ArrayLayers,
				createInfo.Format,
				createInfo.Usage))
		{
			// #TODO: Error. Failed to create Vulkan texture.
			DestroyTexture(&texture);
			return nullptr;
		}

		return &texture;
	}

	auto CreateCommandList(GraphicsDevice graphicsDevice) -> CommandList
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::lock_guard lock(graphicsDevice->Mutex);

		graphicsDevice->CmdLists.push_back(std::make_unique<CommandList_T>());
		auto& cmdList = *graphicsDevice->CmdLists.back();

		cmdList.GraphicsDevice = graphicsDevice;

		if (!Internal::CreateCommandPool(cmdList.CmdPool, graphicsDevice->Device))
		{
			// #TODO: Error. Failed to create Vulkan command pool.
			DestroyCommandList(&cmdList);
			return nullptr;
		}

		return &cmdList;
	}

	bool DestroyCommandList(CommandList commandList)
	{
		if (commandList == nullptr)
			return false;

		while (!commandList->SubmittedCmdBuffers.empty())
		{
			auto submission = commandList->SubmittedCmdBuffers.front();
			commandList->SubmittedCmdBuffers.pop();
			void(commandList->GraphicsDevice->Device.waitForFences(submission.Fence, VK_TRUE, std::uint64_t(-1)));

			std::lock_guard lock(commandList->GraphicsDevice->Mutex);
			commandList->GraphicsDevice->AvailableFences.push(submission.Fence);
		}

		commandList->GraphicsDevice->Device.destroy(commandList->CmdPool);

		{
			std::lock_guard lock(commandList->GraphicsDevice->Mutex);
			for (auto i = 0; i < commandList->GraphicsDevice->CmdLists.size(); ++i)
			{
				auto* gdBuffer = commandList->GraphicsDevice->CmdLists[i].get();
				if (gdBuffer == commandList)
				{
					commandList->GraphicsDevice->CmdLists.erase(commandList->GraphicsDevice->CmdLists.begin() + i);
					break;
				}
			}
		}
		return true;
	}

	bool DestroyTexture(Texture texture)
	{
		if (texture == nullptr)
			return false;

		{
			std::lock_guard lock(texture->Mutex);
			texture->graphicsDevice->Allocator.destroyImage(texture->Image, texture->Allocation);
		}

		{
			std::lock_guard lock(texture->graphicsDevice->Mutex);
			for (auto i = 0; i < texture->graphicsDevice->Textures.size(); ++i)
			{
				auto* gdBuffer = texture->graphicsDevice->Textures[i].get();
				if (gdBuffer == texture)
				{
					texture->graphicsDevice->Textures.erase(texture->graphicsDevice->Textures.begin() + i);
					break;
				}
			}
		}
		return true;
	}

	bool DestroyBuffer(DeviceBuffer buffer)
	{
		if (buffer == nullptr)
			return false;

		{
			std::lock_guard lock(buffer->Mutex);
			buffer->graphicsDevice->Allocator.destroyBuffer(buffer->Buffer, buffer->Allocation);
		}

		{
			std::lock_guard lock(buffer->graphicsDevice->Mutex);
			for (auto i = 0; i < buffer->graphicsDevice->Buffers.size(); ++i)
			{
				auto* gdBuffer = buffer->graphicsDevice->Buffers[i].get();
				if (gdBuffer == buffer)
				{
					buffer->graphicsDevice->Buffers.erase(buffer->graphicsDevice->Buffers.begin() + i);
					break;
				}
			}
		}
		return true;
	}

	bool DestroyGraphicDevice(GraphicsDevice graphicsDevice)
	{
		if (graphicsDevice == nullptr)
			return false;

		WaitForIdle(graphicsDevice); // #TODO: Can we only wait for the devices own CommandLists?

		auto& context = GetContext();

		{
			for (auto& cmdList : graphicsDevice->CmdLists)
			{
				DestroyCommandList(cmdList.get());
			}

			std::lock_guard lock(graphicsDevice->Mutex);
			while (!graphicsDevice->AvailableFences.empty())
			{
				auto fence = graphicsDevice->AvailableFences.front();
				graphicsDevice->AvailableFences.pop();
				graphicsDevice->Device.destroy(fence);
			}
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

	void WaitForIdle(GraphicsDevice graphicsDevice)
	{
		if (graphicsDevice == nullptr)
		{
			// #TODO: Error. GraphicsDevice is null.
			return;
		}

		graphicsDevice->Device.waitIdle();
	}

	void CommandListBegin(CommandList commandList)
	{
		if (commandList == nullptr)
		{
			// #TODO: Error. Cannot record on null CommandList.
			return;
		}

		if (commandList->HasBegun)
		{
			// #TODO: Error. CommandList has already begun.
			return;
		}

		commandList->HasBegun = true;

		if (!commandList->AvailableCmdLists.empty())
		{
			commandList->CmdBuffer = commandList->AvailableCmdLists.front();
			commandList->AvailableCmdLists.pop();
			commandList->CmdBuffer.reset();
		}
		else
		{
			if (!Internal::AllocateCommandBuffer(commandList->CmdBuffer, commandList->GraphicsDevice->Device, commandList->CmdPool))
			{
				// #TODO: Error. Failed to allocate Vulkan command buffer.
				return;
			}
		}

		vk::CommandBufferBeginInfo beginInfo{};
		commandList->CmdBuffer.begin(beginInfo);
	}

	void CommandListEnd(CommandList commandList)
	{
		if (commandList == nullptr)
		{
			// #TODO: Error. Cannot record on null CommandList.
			return;
		}

		if (!commandList->HasBegun)
		{
			// #TODO: Error. CommandList has already not been begun.
			return;
		}

		commandList->HasBegun = false;
		commandList->HasEnded = true;

		commandList->CmdBuffer.end();
	}

	void SubmitCommandList(CommandList commandList)
	{
		if (commandList == nullptr)
		{
			// #TODO: Error. Cannot submit null CommandList.
			return;
		}

		auto* graphicsDevice = commandList->GraphicsDevice;

		CheckSubmittedCmdBuffers(commandList);

		vk::Fence submitFence;
		if (!graphicsDevice->AvailableFences.empty())
		{
			submitFence = graphicsDevice->AvailableFences.front();
			graphicsDevice->AvailableFences.pop();
			graphicsDevice->Device.resetFences(submitFence);
		}
		else
		{
			if (!Internal::CreateFence(submitFence, graphicsDevice->Device))
			{
				// #TODO: Error. Failed to create Vulkan fence.
				return;
			}
		}

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBuffers(commandList->CmdBuffer);
		auto queue = graphicsDevice->GraphicsQueue;
		queue.submit(submitInfo, submitFence);

		/*auto& fenceSubmitInfo = graphicsDevice->SubmittedFences.emplace();
		fenceSubmitInfo.Fence = submitFence;
		fenceSubmitInfo.CmdList = commandList;
		fenceSubmitInfo.CmdBuffer = commandList->CmdBuffer;*/

		auto& submittedCmdInfo = commandList->SubmittedCmdBuffers.emplace();
		submittedCmdInfo.Fence = submitFence;
		submittedCmdInfo.CmdBuffer = commandList->CmdBuffer;
	}

	namespace
	{
		void CheckSubmittedCmdBuffers(CommandList commandList)
		{
			while (!commandList->SubmittedCmdBuffers.empty())
			{
				auto submittedFence = commandList->SubmittedCmdBuffers.front();
				if (commandList->GraphicsDevice->Device.getFenceStatus(submittedFence.Fence) == vk::Result::eSuccess)
				{
					commandList->SubmittedCmdBuffers.pop();

					commandList->AvailableCmdLists.push(submittedFence.CmdBuffer);

					std::lock_guard lock(commandList->GraphicsDevice->Mutex);
					commandList->GraphicsDevice->AvailableFences.push(submittedFence.Fence);
				}
			}
		}

	} // namespace

} // namespace VkMana