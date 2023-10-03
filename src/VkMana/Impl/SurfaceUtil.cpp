#include "SurfaceUtil.hpp"

namespace VkMana
{
	auto SurfaceUtil::CreateSurface(GraphicsDevice::Impl& graphicsDevice, vk::Instance instance, SwapchainSource& swapchainSource)
		-> vk::SurfaceKHR
	{
		// #TODO: Check GraphicsDevice for Surface extension

		if (auto* win32Source = dynamic_cast<Win32SwapchainSource*>(&swapchainSource))
		{
			// #TODO: Check GraphicsDevice for Win32 surface extension
			return CreateWin32Surface(instance, *win32Source);
		}

		// #TODO: Handle Error.
		return nullptr;
	}

	auto SurfaceUtil::CreateWin32Surface(vk::Instance instance, Win32SwapchainSource& source) -> vk::SurfaceKHR
	{
		vk::Win32SurfaceCreateInfoKHR surfaceInfo{};
		surfaceInfo.hinstance = source.GetHInstance();
		surfaceInfo.hwnd = source.GetHWnd();
		auto surface = instance.createWin32SurfaceKHR(surfaceInfo);
		return surface;
	}

} // namespace VkMana