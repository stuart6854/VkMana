#pragma once

#include "SwapchainSource.hpp"

#ifdef VKMANA_USE_SDL2
	#include <SDL.h>
	#include <SDL_syswm.h>
#endif
#ifdef VK_MANA_USE_GLFW
	#include <GLFW/glfw3.h>
#endif

#include <memory>

namespace VkMana
{
#ifdef VKMANA_USE_SDL2
	auto GetSwapchainSource(SDL_Window* window) -> std::shared_ptr<SwapchainSource>
	{
		SDL_version version;
		SDL_GetVersion(&version);
		SDL_SysWMinfo info;
		SDL_GetWindowWMInfo(window, &info);
		switch (info.subsystem)
		{
			case SDL_SYSWM_WINDOWS:
				auto* hwnd = info.info.win.window;
				auto* hInstance = info.info.win.hinstance;
				return SwapchainSource::CreateWin32(hwnd, hInstance);
//			default:
//				break;
		}

		return nullptr;
	}
#endif

#ifdef VKMANA_USE_GLFW
	auto GetSwapchainSource() -> std::shared_ptr<SwapchainSource>
	{
	}
#endif

} // namespace VkMana