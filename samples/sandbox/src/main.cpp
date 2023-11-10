#include "Renderer.hpp"
#include "ModelLoader.hpp"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_syswm.h>
#define VULKAN_DEBUG
#include <VkMana/Logging.hpp>
#include <VkMana/Context.hpp>
#include <VkMana/ShaderCompiler.hpp>

#include <iostream>
#include <filesystem>
#include <string>

using namespace VkMana::Sample;

constexpr auto WindowTitle = "VkMana - Sandbox";
constexpr auto WindowWidth = 1280;
constexpr auto WindowHeight = 720;
constexpr auto WindowAspect = float(WindowWidth) / float(WindowHeight);

class SDL2WSI : public VkMana::WSI
{
public:
	explicit SDL2WSI(SDL_Window* window)
		: m_window(window)
	{
	}

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

private:
	SDL_Window* m_window;
	bool m_isAlive = true;
};

int main()
{
	LOG_INFO("VkMana - Sample - Sandbox");
	LOG_INFO("Path: {}", std::filesystem::current_path().string());

	auto* window = SDL_CreateWindow(
		WindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	SDL2WSI wsi(window);

	Renderer renderer;
	if (!renderer.Init(&wsi))
	{
		LOG_ERR("Failed to initialise Renderer.");
		return 1;
	}

	auto staticMesh = renderer.CreateStaticMesh();
	if (!staticMesh->LoadFromFile("assets/models/viking_room.obj"))
	{
		LOG_ERR("Failed to load mesh!");
		return 1;
	}

	if (!LoadGLTFModel(*staticMesh, renderer, "assets/models/runestone/scene.gltf"))
	// if (!LoadGLTFModel(*staticMesh, renderer, "assets/models/submesh_test/scene.gltf"))
	{
		LOG_ERR("Failed to load GLTF model.");
		return 1;
	}

	double lastFrameTime = double(SDL_GetTicks()) / 1000.0f;
	bool isRunning = true;
	while (isRunning)
	{
		const auto currentFrameTime = double(SDL_GetTicks()) / 1000.0f;
		const auto deltaTime = float(currentFrameTime - lastFrameTime);
		lastFrameTime = currentFrameTime;

		if (!wsi.IsAlive())
			isRunning = false;

		auto windowTitle = std::format("{} - {}ms", WindowTitle, uint32_t(deltaTime * 1000));
		SDL_SetWindowTitle(window, windowTitle.c_str());

		auto projMat = glm::perspectiveLH_ZO(glm::radians(60.0f), WindowAspect, 0.1f, 500.0f);
		const glm::vec3 cameraPos = { -3, 3, -4 };
		const glm::vec3 cameraTarget = { 0, 0.5f, 0 };
		auto viewMat = glm::lookAtLH(cameraPos, cameraTarget, glm::vec3(0, 1, 0));

		renderer.SetSceneCamera(projMat, viewMat);

		glm::mat4 transformMat = glm::mat4(1.0f);
		transformMat = glm::scale(transformMat, glm::vec3(0.5f));
		renderer.Submit(staticMesh.Get(), transformMat);

		renderer.Flush();
	}

	// renderer.Shutdown();

	return 0;
}