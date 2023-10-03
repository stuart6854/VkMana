#pragma once

#include "GraphicsDeviceImpl.hpp"
#include "VkMana/SwapchainSource.hpp"
#include "Win32SwapchainSource.hpp"

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	class SwapchainSource;

	class SurfaceUtil
	{
	public:
		static auto CreateSurface(GraphicsDevice::Impl& graphicsDevice, vk::Instance instance, SwapchainSource& swapchainSource)
			-> vk::SurfaceKHR;

	private:
		static auto CreateWin32Surface(vk::Instance instance, Win32SwapchainSource& source) -> vk::SurfaceKHR;
	};

} // namespace VkMana
