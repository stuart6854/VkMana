#pragma once

#include "VkMana/Enums.hpp"
#include "VkMana/Framebuffer.hpp"

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	class GraphicsDevice;
	class Swapchain;

	class SwapchainFramebuffer : public Framebuffer
	{
	public:
		SwapchainFramebuffer(GraphicsDevice& graphicsDevice,
			Swapchain& swapchain,
			vk::SurfaceKHR surface,
			std::uint32_t width,
			std::uint32_t height,
			PixelFormat depthFormat);
		~SwapchainFramebuffer();

		void SetImageIndex(std::uint32_t imageIndex);

		void SetNewSwapchain(vk::SwapchainKHR swapchain,
			std::uint32_t width,
			std::uint32_t height,
			vk::SurfaceFormatKHR surfaceFormat,
			vk::Extent2D swapchainExtent);

		void TransitionToIntermediateLayout(vk::CommandBuffer cmdBuffer);
		void TransitionToFinalLayout(vk::CommandBuffer cmdBuffer);

	private:
		GraphicsDevice& m_graphicsDevice;

		Swapchain& m_swapchain;
		vk::SurfaceKHR m_surface;
		PixelFormat m_depthFormat;
		std::uint32_t m_currentImageIndex;

		std::uint32_t  m_desiredWidth;
		std::uint32_t  m_desiredHeight;

		std::vector<vk::Image> m_swapchainImages;
		vk::SurfaceFormatKHR m_swapchainImageFormat;
		vk::Extent2D m_swapchainExtent;

		std::vector<vk::RenderingAttachmentInfo> m_colorAttachments;
		vk::RenderingAttachmentInfo m_depthAttachment;
		vk::RenderingInfo m_dynRenderingInfo{};

		// vk::RenderPass m_renderPass;
	};

} // namespace VkMana
