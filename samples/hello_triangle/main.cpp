#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#define VKMANA_USE_SDL2
#include <VkMana/VkMana.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <memory>

// #TODO: Sdl2Window wrapper

auto GetSwapchainProvider(SDL_Window* window) -> std::shared_ptr<VkMana::SurfaceProvider>
{
	SDL_SysWMinfo systemInfo;
	SDL_VERSION(&systemInfo.version);
	SDL_GetWindowWMInfo(window, &systemInfo);
	switch (systemInfo.subsystem)
	{
		case SDL_SYSWM_WINDOWS:
			auto surface = std::make_shared<VkMana::Win32Surface>();
			surface->HInstance = systemInfo.info.win.hinstance;
			surface->HWnd = systemInfo.info.win.window;
			return surface;
	}
	return nullptr;
}

int main()
{
	std::cout << "Sample - Hello Triangle" << '\n';

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "[Sample] Failed to init SDL:" << SDL_GetError() << '\n';
		return -1;
	}

	auto windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
	auto* sdlWindow = SDL_CreateWindow("Sample - Hello Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, windowFlags);

	auto surfaceProvider = GetSwapchainProvider(sdlWindow);
	VkMana::GraphicsDeviceCreateInfo gdInfo{};
	gdInfo.Debug = true;
	gdInfo.MainSwapchainCreateInfo.SurfaceProvider = surfaceProvider.get();
	gdInfo.MainSwapchainCreateInfo.Width = 1280;
	gdInfo.MainSwapchainCreateInfo.Height = 720;
	gdInfo.MainSwapchainCreateInfo.ClearColor = VkMana::Rgba_CornflowerBlue;
	gdInfo.MainSwapchainCreateInfo.vsync = true;
	gdInfo.MainSwapchainCreateInfo.Srgb = true;
	auto* graphicsDevice = VkMana::CreateGraphicsDevice(gdInfo);

	VkMana::BufferCreateInfo bufferInfo{};
	bufferInfo.Size = 512;
	bufferInfo.Usage = VkMana::BufferUsage::Storage;
	auto* buffer = CreateBuffer(graphicsDevice, bufferInfo);

	VkMana::TextureCreateInfo textureInfo{};
	textureInfo.Width = 2048;
	textureInfo.Height = 2048;
	textureInfo.Format = VkMana::PixelFormat::R8_G8_B8_A8_UNorm;
	textureInfo.Usage = VkMana::TextureUsage::RenderTarget;
	auto texture = VkMana::CreateTexture(graphicsDevice, textureInfo);

	VkMana::FramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.ColorAttachments = {
		VkMana::FramebufferAttachmentCreateInfo{ texture, 0, 0 },
	};
	auto* offscreenFramebuffer = VkMana::CreateFramebuffer(graphicsDevice, framebufferCreateInfo);

	auto cmdList = VkMana::CreateCommandList(graphicsDevice);

	std::uint64_t nowTime = SDL_GetPerformanceCounter();
	std::uint64_t lastTime = 0;
	SDL_Event event;
	bool running = true;
	while (running)
	{
		lastTime = nowTime;
		nowTime = SDL_GetPerformanceCounter();
		auto deltaTime = float((nowTime - lastTime) * 1000 / double(SDL_GetPerformanceFrequency())) * 0.001f;
		//		std::cout << "New Frame. DeltaTime=" << deltaTime << "s" << std::endl;

		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				running = false;
			}
		}

		auto title = std::string("Sample - Hello Triangle - ") + std::to_string(deltaTime) + "s";
		SDL_SetWindowTitle(sdlWindow, title.c_str());

		//		const auto& vulkanStats = gd->GetVulkanStats();
		// std::cout << "Vulkan Stats:" << std::endl;
		// std::cout << " Command Buffers: " << vulkanStats.NumCommandBuffers << std::endl;
		// std::cout << " Fences: " << vulkanStats.NumFences << std::endl;

		VkMana::CommandListBegin(cmdList);
		VkMana::CommandListBindFramebuffer(cmdList, offscreenFramebuffer);
		VkMana::CommandListEnd(cmdList);

		VkMana::SubmitCommandList(cmdList);
		//		graphicsDevice->SwapBuffers();

		VkMana::SwapBuffers(graphicsDevice);

		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	VkMana::DestroyFramebuffer(offscreenFramebuffer);
	VkMana::DestroyCommandList(cmdList);
	VkMana::DestroyTexture(texture);
	VkMana::DestroyBuffer(buffer);
	VkMana::DestroyGraphicDevice(graphicsDevice);

	SDL_Quit();

	return 0;
}