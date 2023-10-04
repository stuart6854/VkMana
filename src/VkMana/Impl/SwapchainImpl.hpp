#pragma once

#include "VkMana/Swapchain.hpp"
#include "../SwapchainFramebuffer.hpp"

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	class GraphicsDevice;

	class Swapchain::Impl
	{
	public:
		explicit Impl(GraphicsDevice& graphicsDevice, const SwapchainDescription& desc);
		~Impl();

		void SetSurface(vk::SurfaceKHR newSurface);
		void SetSyncToVerticalBlank(bool syncToVerticalBlank);

		void Resize(std::uint32_t width, std::uint32_t height);
		void Present();

		auto GetSwapchain() const -> auto { return m_swapchain; }
		auto GetPresentQueue() const -> auto { return m_presentQueue; }
		//		auto GetFramebuffer() const -> const auto&;
		auto GetImageIndex() const -> auto { return m_currentImageIndex; }

	private:
		void CreateSwapchain(std::uint32_t width, std::uint32_t height);

		void AcquireNextImage(vk::Device device, vk::Semaphore semaphore, vk::Fence fence);

		void RecreateAndReacquire(std::uint32_t width, std::uint32_t height);

	private:
		GraphicsDevice& m_graphicsDevice;

		vk::SurfaceKHR m_surface;
		vk::SwapchainKHR m_swapchain;
		std::unique_ptr<SwapchainFramebuffer> m_framebuffer;
		vk::Fence m_imageReadyFence;
		std::uint32_t m_presentQueueIndex;
		vk::Queue m_presentQueue;
		bool m_syncToVerticalBlank;
		std::shared_ptr<SwapchainSource> m_source;
		bool m_colorSrgb;

		std::uint32_t m_currentImageIndex;

		bool m_isDirty = false;
	};

} // namespace VkMana
