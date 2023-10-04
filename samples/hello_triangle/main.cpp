#define SDL_MAIN_HANDLED
#include <SDL.h>
#define VKMANA_USE_SDL2
#include <VkMana/VkMana.hpp>

#include <chrono>
#include <iostream>
#include <thread>

// #TODO: Sdl2Window wrapper

int main()
{
	using namespace VkMana;

	std::cout << "Sample - Hello Triangle" << '\n';

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "[Sample] Failed to init SDL:" << SDL_GetError() << '\n';
		return -1;
	}

	auto windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
	auto* sdlWindow = SDL_CreateWindow("Sample - Hello Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1270, 720, windowFlags);

	GraphicsDeviceCreateInfo gdInfo{};
	gdInfo.Debug = true;
	auto* graphicsDevice = CreateGraphicsDevice(gdInfo);
	//	auto* factory = graphicsDevice->GetFactory();

	//	auto cmdList = factory->CreateCommandList();

	SDL_Event event;
	bool running = true;
	while (running)
	{
		// std::cout << "New Frame" << std::endl;

		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		//		const auto& vulkanStats = gd->GetVulkanStats();
		// std::cout << "Vulkan Stats:" << std::endl;
		// std::cout << " Command Buffers: " << vulkanStats.NumCommandBuffers << std::endl;
		// std::cout << " Fences: " << vulkanStats.NumFences << std::endl;

		//		cmdList->Begin();
		//		cmdList->End();

		//		graphicsDevice->SubmitCommands(*cmdList);
		//		graphicsDevice->SwapBuffers();

		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}

	DestroyGraphicDevice(graphicsDevice);

	SDL_Quit();

	return 0;
}