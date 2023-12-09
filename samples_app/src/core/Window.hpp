#pragma once

#include <VkMana/WSI.hpp>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

namespace VkMana::SamplesApp
{
	class Window : public WSI
	{
	public:
		explicit Window(SDL_Window* window)
			: m_window(window)
		{
		}
		~Window() override = default;

		void PollEvents() override
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_QUIT:
						m_isAlive = false;
						break;
				}
			}
		}

		auto CreateSurface(vk::Instance instance) -> vk::SurfaceKHR override
		{
			VkSurfaceKHR surface = nullptr;
			if (SDL_Vulkan_CreateSurface(m_window, instance, &surface))
				return surface;

			return nullptr;
		}

		/*auto GetInstanceExtension() -> std::vector<const char*> override
		{
			uint32_t extCount = 0;
			SDL_Vulkan_GetInstanceExtensions(Window, &extCount, nullptr);
			std::vector<const char*> exts(extCount);
			SDL_Vulkan_GetInstanceExtensions(Window, &extCount, exts.data());
			return exts;
		}*/

		auto GetSurfaceWidth() -> uint32_t override
		{
			int32_t w = 0;
			int32_t h = 0;
			SDL_Vulkan_GetDrawableSize(m_window, &w, &h);
			return w;
		}
		auto GetSurfaceHeight() -> uint32_t override
		{
			int32_t w = 0;
			int32_t h = 0;
			SDL_Vulkan_GetDrawableSize(m_window, &w, &h);
			return h;
		}

		bool IsVSync() override { return true; }
		bool IsAlive() override { return m_isAlive; }

		void HideCursor() override {}
		void ShowCursor() override {}

		auto CreateCursor(uint32_t cursorType) -> void* override { return nullptr; }
		void SetCursor(void* cursor) override {}

	private:
		SDL_Window* m_window;
		bool m_isAlive = true;
	};

} // namespace VkMana::SamplesApp