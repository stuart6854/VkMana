#pragma once

#include "VkMana/VkMana.hpp"
#include "VulkanUtils.hpp"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include <cstdint>
#include <queue>
#include <mutex>
#include <vector>

namespace VkMana
{
	struct Context_T
	{
		std::mutex Mutex;
		std::vector<std::unique_ptr<GraphicsDevice_T>> GraphicsDevices;
	};

	struct GraphicsDevice_T
	{
		std::mutex Mutex;
		vk::Instance Instance;
		vk::PhysicalDevice PhysicalDevice;
		vk::Device Device;
		Vulkan::QueueFamilyIndices QueueFamilyIndices;
		vk::Queue GraphicsQueue;
		vma::Allocator Allocator;
		Swapchain MainSwapchain;
		std::queue<vk::Fence> AvailableFences;
		std::vector<std::unique_ptr<Swapchain_T>> Swapchains;
		std::vector<std::unique_ptr<Framebuffer_T>> Framebuffers;
		std::vector<std::unique_ptr<DeviceBuffer_T>> Buffers;
		std::vector<std::unique_ptr<Texture_T>> Textures;
		std::vector<std::unique_ptr<CommandList_T>> CmdLists;
	};
	struct Swapchain_T
	{
		GraphicsDevice GraphicsDevice = nullptr;
		vk::SurfaceKHR Surface;
		vk::SwapchainKHR Swapchain;
		std::uint32_t Width = 0;
		std::uint32_t Height = 0;
		bool VSync = false;
		bool ColorSrgb = false;
		vk::Format Format = vk::Format::eUndefined;
		Framebuffer Framebuffer = nullptr;
		vk::Fence AcquireFence;
	};
	struct DeviceBuffer_T
	{
		std::mutex Mutex;
		GraphicsDevice graphicsDevice = nullptr;
		vk::Buffer Buffer;
		vma::Allocation Allocation;
		std::uint64_t Size = 0;
		vk::BufferUsageFlags Usage = {};
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
		vk::Format Format = {};
		vk::ImageUsageFlags Usage = {};
		// TextureType Type = {};
	};
	struct FramebufferAttachment_T
	{
		Texture TargetTexture = nullptr;
		std::uint32_t MipLevel = 0;
		std::uint32_t ArrayLayer = 0;
		RgbaFloat ClearColor = Rgba_Black;
		vk::ImageView ImageView;
		vk::ImageLayout IntermediateLayout; // Pre-Pass
		vk::ImageLayout FinalLayout;		// Post-Pass
	};
	struct Framebuffer_T
	{
		GraphicsDevice GraphicsDevice = nullptr;
		Swapchain SwapchainTarget = nullptr; // nullptr = Offscreen.
		std::vector<FramebufferAttachment_T> ColorTargets;
		FramebufferAttachment_T DepthTarget;
		std::uint32_t CurrentImageIndex = 0; // For swapchain target.

		// vk::RenderPass RenderPass;
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

		Framebuffer BoundFramebuffer = nullptr;
	};

	auto GetContext() -> Context_T&;

	bool CreateGraphicsDevice(GraphicsDevice_T& graphicsDevice, const GraphicsDeviceCreateInfo& createInfo);
	bool CreateSwapchain(Swapchain_T& swapchain, const SwapchainCreateInfo& createInfo);
	bool CreateFramebuffer(Framebuffer_T& framebuffer, const FramebufferCreateInfo& createInfo);

	void TransitionFramebufferToIntermediate(CommandList_T& cmdList, Framebuffer_T& framebuffer);
	void TransitionFramebufferToFinal(CommandList_T& cmdList, Framebuffer_T& framebuffer);

	/*************************************************************
	 * Conversions
	 ************************************************************/

	auto ToVkFormat(PixelFormat format) -> vk::Format;

	/*************************************************************
	 * Utilities
	 ************************************************************/

	bool CreateSurface(vk::SurfaceKHR& outSurface, SurfaceProvider* surfaceProvider, vk::Instance instance);

	auto CreateTexturesFromSwapchainImages(Swapchain_T& swapchain) -> std::vector<Texture>;

	void CheckSubmittedCmdBuffers(CommandList_T& commandList);

	void ClearCachedCmdListState(CommandList_T& commandList);

} // namespace VkMana