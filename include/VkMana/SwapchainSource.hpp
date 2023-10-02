#pragma once

#ifndef _WINDEF_
class HINSTANCE__;
using HINSTANCE = HINSTANCE__*;
class HWND__;
using HWND = HWND__*;
#endif

#include <memory>

namespace VkMana
{
	class SwapchainSource
	{
	public:
		virtual ~SwapchainSource() = default;

		static auto CreateWin32(HWND hwnd, HINSTANCE hInstance) -> std::unique_ptr<SwapchainSource>;

		// static auto CreateXlib(void* display, void* window) -> std::unique_ptr<SwapchainSource>;
		// static auto CreateWayland(void* display, void* surface) -> std::unique_ptr<SwapchainSource>;
	};
} // namespace VkMana