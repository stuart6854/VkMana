#pragma once

#include "SwapchainSource.hpp"

#ifdef VKMANA_USE_SDL2
	#include <SDL.h>
	#include <SDL_syswm.h>
#endif
#ifdef VK_MANA_USE_GLFW
	#include <GLFW/glfw3.h>
#endif

#include <iostream>
#include <memory>

namespace VkMana
{
#ifdef VKMANA_USE_SDL2
	auto GetSwapchainSource(SDL_Window* window) -> std::shared_ptr<SwapchainSource>
	{
		SDL_SysWMinfo info;
		SDL_GetVersion(&info.version);
		if (SDL_GetWindowWMInfo(window, &info) == 0)
		{
			std::cerr << SDL_GetError() << "\n";
			// #TODO: Error.
		}

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
	auto GetSwapchainSource() -> std::shared_ptr<SwapchainSource> {}
#endif

} // namespace VkMana