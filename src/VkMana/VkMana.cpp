#include "VkMana/VkMana.hpp"

#include "VkManaImpl.hpp"

#include <vulkan/vulkan.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <memory>

namespace VkMana
{
	auto CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) -> GraphicsDevice
	{
		auto& context = GetContext();
		std::lock_guard lock(context.Mutex);

		context.GraphicsDevices.push_back(std::make_unique<GraphicsDevice_T>());
		auto& graphicsDevice = *context.GraphicsDevices.back();

		if (!CreateGraphicsDevice(graphicsDevice, createInfo))
		{
			DestroyGraphicDevice(&graphicsDevice);
			return nullptr;
		}

		return &graphicsDevice;
	}

	auto CreateSwapchain(GraphicsDevice graphicsDevice, const SwapchainCreateInfo& createInfo) -> Swapchain
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::unique_lock lock(graphicsDevice->Mutex);

		graphicsDevice->Swapchains.push_back(std::make_unique<Swapchain_T>());
		auto& swapchain = *graphicsDevice->Swapchains.back();

		lock.unlock();

		swapchain.GraphicsDevice = graphicsDevice;
		if (!CreateSwapchain(swapchain, createInfo))
		{
			DestroySwapchain(&swapchain);
			return nullptr;
		}

		return &swapchain;
	}

	auto CreateBuffer(GraphicsDevice graphicsDevice, const BufferCreateInfo& createInfo) -> DeviceBuffer
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::lock_guard lock(graphicsDevice->Mutex);

		graphicsDevice->Buffers.push_back(std::make_unique<DeviceBuffer_T>());
		auto& buffer = *graphicsDevice->Buffers.back();

		buffer.graphicsDevice = graphicsDevice;

		if (createInfo.Usage == BufferUsage::Vertex)
			buffer.Usage = vk::BufferUsageFlagBits::eVertexBuffer;
		else if (createInfo.Usage == BufferUsage::Index)
			buffer.Usage = vk::BufferUsageFlagBits::eIndexBuffer;
		else if (createInfo.Usage == BufferUsage::Uniform)
			buffer.Usage = vk::BufferUsageFlagBits::eUniformBuffer;
		else if (createInfo.Usage == BufferUsage::Storage)
			buffer.Usage = vk::BufferUsageFlagBits::eStorageBuffer;

		if (!Vulkan::CreateDeviceBuffer(buffer.Buffer, buffer.Allocation, graphicsDevice->Allocator, createInfo.Size, buffer.Usage))
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
		texture.Width = createInfo.Width;
		texture.Height = createInfo.Height;
		texture.Depth = createInfo.Depth;
		texture.MipLevels = createInfo.MipLevels;
		texture.ArrayLayers = createInfo.ArrayLayers;
		texture.Format = ToVkFormat(createInfo.Format);

		if (createInfo.Usage == TextureUsage::Sampled)
			texture.Usage = vk::ImageUsageFlagBits::eSampled;
		else if (createInfo.Usage == TextureUsage::Storage)
			texture.Usage = vk::ImageUsageFlagBits::eStorage;
		else if (createInfo.Usage == TextureUsage::RenderTarget)
			texture.Usage = vk::ImageUsageFlagBits::eColorAttachment;
		else if (createInfo.Usage == TextureUsage::DepthStencil)
			texture.Usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		//		else if (createInfo.Usage == TextureUsage::CubeMap)
		//			texture.Usage = vk::ImageUsageFlagBits::eSampled;

		if (!Vulkan::CreateTexture(texture.Image,
				texture.Allocation,
				graphicsDevice->Allocator,
				texture.Width,
				texture.Height,
				texture.Depth,
				texture.MipLevels,
				texture.ArrayLayers,
				texture.Format,
				texture.Usage))
		{
			// #TODO: Error. Failed to create Vulkan texture.
			DestroyTexture(&texture);
			return nullptr;
		}

		return &texture;
	}

	auto CreateFramebuffer(GraphicsDevice graphicsDevice, const FramebufferCreateInfo& createInfo) -> Framebuffer
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::lock_guard lock(graphicsDevice->Mutex);

		graphicsDevice->Framebuffers.push_back(std::make_unique<Framebuffer_T>());
		auto& framebuffer = *graphicsDevice->Framebuffers.back();

		framebuffer.GraphicsDevice = graphicsDevice;
		if (!CreateFramebuffer(framebuffer, createInfo))
		{
			DestroyFramebuffer(&framebuffer);
			return nullptr;
		}

		return &framebuffer;
	}

	auto CreateCommandList(GraphicsDevice graphicsDevice) -> CommandList
	{
		if (graphicsDevice == nullptr)
			return nullptr;

		std::lock_guard lock(graphicsDevice->Mutex);

		graphicsDevice->CmdLists.push_back(std::make_unique<CommandList_T>());
		auto& cmdList = *graphicsDevice->CmdLists.back();

		cmdList.GraphicsDevice = graphicsDevice;

		if (!Vulkan::CreateCommandPool(cmdList.CmdPool, graphicsDevice->Device))
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

	bool DestroyFramebuffer(Framebuffer framebuffer)
	{
		if (framebuffer == nullptr)
			return false;

		auto device = framebuffer->GraphicsDevice->Device;
		for (auto& target : framebuffer->ColorTargets)
		{
			device.destroy(target.ImageView);
		}
		device.destroy(framebuffer->DepthTarget.ImageView);

		{
			std::lock_guard lock(framebuffer->GraphicsDevice->Mutex);
			for (auto i = 0; i < framebuffer->GraphicsDevice->Framebuffers.size(); ++i)
			{
				auto* gdBuffer = framebuffer->GraphicsDevice->Framebuffers[i].get();
				if (gdBuffer == framebuffer)
				{
					framebuffer->GraphicsDevice->Framebuffers.erase(framebuffer->GraphicsDevice->Framebuffers.begin() + i);
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

	bool DestroySwapchain(Swapchain swapchain)
	{
		if (swapchain == nullptr)
			return false;

		DestroyFramebuffer(swapchain->Framebuffer);
		swapchain->GraphicsDevice->Device.destroy(swapchain->AcquireFence);
		swapchain->GraphicsDevice->Device.destroy(swapchain->Swapchain);
		swapchain->GraphicsDevice->Instance.destroy(swapchain->Surface);

		{
			std::lock_guard lock(swapchain->GraphicsDevice->Mutex);
			for (auto i = 0; i < swapchain->GraphicsDevice->Swapchains.size(); ++i)
			{
				auto* gdSwapchain = swapchain->GraphicsDevice->Swapchains[i].get();
				if (gdSwapchain == swapchain)
				{
					swapchain->GraphicsDevice->Swapchains.erase(swapchain->GraphicsDevice->Swapchains.begin() + i);
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

			DestroySwapchain(graphicsDevice->MainSwapchain);

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

	void SwapBuffers(GraphicsDevice graphicsDevice, Swapchain swapchain)
	{
		if (graphicsDevice == nullptr)
		{
			// #TODO: Error. GraphicsDevice is null.
			return;
		}

		if (swapchain == nullptr && graphicsDevice->MainSwapchain != nullptr)
		{
			swapchain = graphicsDevice->MainSwapchain;
		}
		if (swapchain == nullptr)
		{
			// #TODO: Error (Warning?). No swapchain to swap buffers for.
			return;
		}

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setSwapchains(swapchain->Swapchain);
		presentInfo.setImageIndices(swapchain->Framebuffer->CurrentImageIndex);
		void(graphicsDevice->GraphicsQueue.presentKHR(presentInfo));
		// #TODO: Handle present result.

		auto result =
			swapchain->GraphicsDevice->Device.acquireNextImageKHR(swapchain->Swapchain, std::uint64_t(-1), {}, swapchain->AcquireFence);
		swapchain->Framebuffer->CurrentImageIndex = result.value;

		void(graphicsDevice->Device.waitForFences(swapchain->AcquireFence, VK_TRUE, std::uint64_t(-1)));
		graphicsDevice->Device.resetFences(swapchain->AcquireFence);
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
			if (!Vulkan::AllocateCommandBuffer(commandList->CmdBuffer, commandList->GraphicsDevice->Device, commandList->CmdPool))
			{
				// #TODO: Error. Failed to allocate Vulkan command buffer.
				return;
			}
		}

		vk::CommandBufferBeginInfo beginInfo{};
		commandList->CmdBuffer.begin(beginInfo);

		ClearCachedCmdListState(*commandList);
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

		if (commandList->BoundFramebuffer != nullptr)
		{
			commandList->CmdBuffer.endRendering();
			// #TODO: Transition framebuffer to final layout
		}

		commandList->CmdBuffer.end();
	}

	void CommandListBindFramebuffer(CommandList commandList, Framebuffer framebuffer)
	{
		if (commandList == nullptr)
		{
			// #TODO: Error. Cannot record on null CommandList.
			return;
		}

		if (!commandList->HasBegun)
		{
			// #TODO: Error. CommandList has already begun.
			return;
		}

		if (commandList->BoundFramebuffer != nullptr)
		{
			commandList->CmdBuffer.endRendering();
			// #TODO: Transition framebuffer to final layout
		}

		// #TODO: Transition framebuffer to intermediate layout

		// #TODO: If `framebuffer` is nullptr, bind main viewport (if exists).

		Texture dimTex = nullptr;
		if (!framebuffer->ColorTargets.empty())
		{
			dimTex = framebuffer->ColorTargets[0].TargetTexture;
		}
		else
		{
			dimTex = framebuffer->DepthTarget.TargetTexture;
		}
		auto renderWidth = dimTex->Width; // #TODO: Use correct MipLevel dimensions.
		auto renderHeight = dimTex->Height;

		std::vector<vk::RenderingAttachmentInfo> colorAttachments(framebuffer->ColorTargets.size());
		for (auto i = 0; i < framebuffer->ColorTargets.size(); ++i)
		{
			const auto& target = framebuffer->ColorTargets[i];
			auto& attachment = colorAttachments[i];
			attachment.setImageView(target.ImageView);
			attachment.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
			attachment.setLoadOp(vk::AttachmentLoadOp::eClear);
			attachment.setStoreOp(vk::AttachmentStoreOp::eStore);

			const auto& color = target.ClearColor;
			attachment.setClearValue(vk::ClearColorValue(color.R, color.G, color.B, color.A));
		}
		vk::RenderingAttachmentInfo depthAttachment;
		depthAttachment.setImageView(framebuffer->DepthTarget.ImageView);
		depthAttachment.setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal);
		depthAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
		depthAttachment.setStoreOp(vk::AttachmentStoreOp::eDontCare);

		vk::RenderingInfo renderingInfo{};
		renderingInfo.setColorAttachments(colorAttachments);
		renderingInfo.setPDepthAttachment(framebuffer->DepthTarget.TargetTexture ? &depthAttachment : nullptr);
		renderingInfo.setLayerCount(1);
		renderingInfo.setRenderArea({ { 0, 0 }, { renderWidth, renderHeight } });

		commandList->CmdBuffer.beginRendering(renderingInfo);
		commandList->BoundFramebuffer = framebuffer;
	}

	void SubmitCommandList(CommandList commandList)
	{
		if (commandList == nullptr)
		{
			// #TODO: Error. Cannot submit null CommandList.
			return;
		}

		auto* graphicsDevice = commandList->GraphicsDevice;

		CheckSubmittedCmdBuffers(*commandList);

		vk::Fence submitFence;
		if (!graphicsDevice->AvailableFences.empty())
		{
			submitFence = graphicsDevice->AvailableFences.front();
			graphicsDevice->AvailableFences.pop();
			graphicsDevice->Device.resetFences(submitFence);
		}
		else
		{
			if (!Vulkan::CreateFence(submitFence, graphicsDevice->Device))
			{
				// #TODO: Error. Failed to create Vulkan fence.
				return;
			}
		}

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBuffers(commandList->CmdBuffer);
		auto queue = graphicsDevice->GraphicsQueue;
		queue.submit(submitInfo, submitFence);

		auto& submittedCmdInfo = commandList->SubmittedCmdBuffers.emplace();
		submittedCmdInfo.Fence = submitFence;
		submittedCmdInfo.CmdBuffer = commandList->CmdBuffer;
	}

} // namespace VkMana