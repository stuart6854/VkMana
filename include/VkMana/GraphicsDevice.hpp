#pragma once

#include "ResourceFactory.hpp"

#include <memory>
#include <string>
#include <vector>

namespace VkMana
{
	struct SwapchainDescription;

	class Swapchain;
	class Fence;

	struct GraphicsDeviceOptions
	{
		bool Debug;
		bool HasMainViewport;

		std::vector<const char*> InstanceExtensions;
		std::vector<const char*> DeviceExtensions;
	};

	class GraphicsDevice
	{
	public:
		class Impl;

	public:
		static auto Create(const GraphicsDeviceOptions& options, SwapchainDescription* swapchainDescription, bool colorSrgb = false)
			-> std::unique_ptr<GraphicsDevice>;

		GraphicsDevice(const GraphicsDeviceOptions& options, SwapchainDescription* swapchainDescription, bool colorSrgb = false);
		~GraphicsDevice();

		auto GetFactory() const -> auto* { return m_factory.get(); }

		void SubmitCommands(CommandList& commandList, Fence* fence = nullptr);

		void WaitForFence(Fence& fence, std::uint64_t timeout = std::uint64_t(-1));
		void WaitForFences(const std::vector<Fence&>& fences, bool waitAll, std::uint64_t timeout = std::uint64_t(-1));

		void ResetFence(Fence& fence);

		void SwapBuffers(); // SwapBuffers on the main swapchain
		void SwapBuffers(Swapchain& swapchain);

		void ResizeMainWindow(std::uint32_t width, std::uint32_t height);

		void WaitForIdle();

		//		auto Map(MappableResource resource, MapMode mode, std::uint32_t subresource = 0) -> void*;
		//		void Unmap(MappableResource resource, std::uint32_t subresource = 0);

		void UpdateTexture(Texture& texture,
			std::uint64_t sizeBytes,
			const void* data,
			std::uint32_t x,
			std::uint32_t y,
			std::uint32_t z,
			std::uint32_t width,
			std::uint32_t height,
			std::uint32_t depth,
			std::uint32_t mipLevel,
			std::uint32_t arrayLayer);

		void UpdateBuffer(DeviceBuffer& buffer, std::uint32_t offsetBytes, std::uint32_t sizeBytes, const void* data);

		//		void DisposeWhenIdle(std::unique_ptr<IDisposable> disposable);
		//		void DisposeWhenIdle(std::shared_ptr<IDisposable> disposable);

		struct VulkanStats
		{
			std::uint32_t NumCommandBuffers = 0;
			std::uint32_t NumFences = 0;
		};
		auto GetVulkanStats() const -> const VulkanStats&;

		auto GetImpl() const -> auto* { return m_impl.get(); }

	private:
		std::unique_ptr<Impl> m_impl;

		std::unique_ptr<ResourceFactory> m_factory;
		std::shared_ptr<Swapchain> m_mainSwapchain;
	};
} // namespace VkMana
