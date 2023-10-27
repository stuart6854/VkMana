#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_syswm.h>
#define VKMANA_USE_SDL2
#include <VkMana/VkMana.hpp>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

const std::string TriangleVertexShaderSrc = R"(
#version 450

layout(push_constant) uniform Constant
{
	mat4 modelMatrix;
} uConstants;

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(0.5, 0.5, 0.0),
		vec3(-0.5, 0.5, 0.0),
		vec3(0.0, -0.5, 0.0)
	);

	//output the position of each vertex
	gl_Position = uConstants.modelMatrix * vec4(positions[gl_VertexIndex], 1.0f);
}
)";
const std::string TriangleFragmentShaderSrc = R"(
#version 450

layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

constexpr auto WindowWidth = 1280;
constexpr auto WindowHeight = 720;

int main()
{
	std::cout << "Sample - Hello Triangle" << '\n';

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "[Sample] Failed to init SDL:" << SDL_GetError() << '\n';
		return -1;
	}

	auto windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
	auto* sdlWindow = SDL_CreateWindow(
		"Sample - Hello Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight, windowFlags);

	auto surfaceProvider = GetSwapchainProvider(sdlWindow);
	VkMana::GraphicsDeviceCreateInfo gdInfo{};
	gdInfo.Debug = true;
	gdInfo.MainSwapchainCreateInfo.SurfaceProvider = surfaceProvider.get();
	gdInfo.MainSwapchainCreateInfo.Width = WindowWidth;
	gdInfo.MainSwapchainCreateInfo.Height = WindowHeight;
	gdInfo.MainSwapchainCreateInfo.ClearColor = VkMana::Rgba_CornflowerBlue;
	gdInfo.MainSwapchainCreateInfo.VSync = true;
	gdInfo.MainSwapchainCreateInfo.Srgb = true;
	auto* graphicsDevice = VkMana::CreateGraphicsDevice(gdInfo);
	auto* mainSwapchain = VkMana::GraphicsDeviceGetMainSwapchain(graphicsDevice);
	auto* swapchainFramebuffer = VkMana::SwapchainGetFramebuffer(mainSwapchain);

	VkMana::BufferCreateInfo bufferInfo{};
	bufferInfo.Size = sizeof(float) * 3 * 3;
	bufferInfo.Usage = VkMana::BufferUsage::Vertex | VkMana::BufferUsage::HostAccessible;
	auto* buffer = CreateBuffer(graphicsDevice, bufferInfo);

	std::vector<float> vertices = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	VkMana::BufferUpdateData(buffer, 0, bufferInfo.Size, vertices.data());

	VkMana::TextureCreateInfo textureInfo{};
	textureInfo.Width = 2048;
	textureInfo.Height = 2048;
	textureInfo.Format = VkMana::PixelFormat::R8_G8_B8_A8_UNorm;
	textureInfo.Usage = VkMana::TextureUsage::RenderTarget | VkMana::TextureUsage::Sampled;
	auto texture = VkMana::CreateTexture(graphicsDevice, textureInfo);

	VkMana::FramebufferCreateInfo framebufferCreateInfo{};
	framebufferCreateInfo.ColorAttachments = {
		VkMana::FramebufferAttachmentCreateInfo{ texture, 0, 0 },
	};
	auto* offscreenFramebuffer = VkMana::CreateFramebuffer(graphicsDevice, framebufferCreateInfo);

	VkMana::GraphicsPipelineCreateInfo gfxPipelineInfo{};
	gfxPipelineInfo.Shaders = {
		VkMana::ShaderCreateInfo{
			VkMana::ShaderStage::Vertex,
			{},
			TriangleVertexShaderSrc,
			VkMana::ShaderCompileSettings{
				true,
			},
		},
		VkMana::ShaderCreateInfo{
			VkMana::ShaderStage::Fragment,
			{},
			TriangleFragmentShaderSrc,
			VkMana::ShaderCompileSettings{
				true,
			},
		},
	};
	gfxPipelineInfo.Constant = { VkMana::ShaderStage::Vertex, sizeof(glm::mat4) };
	gfxPipelineInfo.Topology = VkMana::PrimitiveTopology::TriangleList;
	auto* gfxPipeline = VkMana::CreateGraphicsPipeline(graphicsDevice, gfxPipelineInfo);

	auto cmdList = VkMana::CreateCommandList(graphicsDevice);

	float rot = 0.0f;

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

		rot += 45.0f * deltaTime;
		auto transform = glm::rotate(glm::mat4(1.0f), glm::radians(rot), glm::vec3(0, 1, 0));

		auto title = std::string("Sample - Hello Triangle - ") + std::to_string(deltaTime) + "s";
		SDL_SetWindowTitle(sdlWindow, title.c_str());

		//		const auto& vulkanStats = gd->GetVulkanStats();
		// std::cout << "Vulkan Stats:" << std::endl;
		// std::cout << " Command Buffers: " << vulkanStats.NumCommandBuffers << std::endl;
		// std::cout << " Fences: " << vulkanStats.NumFences << std::endl;

		VkMana::CommandListBegin(cmdList);
		VkMana::CommandListBindFramebuffer(cmdList, offscreenFramebuffer);
		VkMana::CommandListBindFramebuffer(cmdList, swapchainFramebuffer);
		VkMana::CommandListBindPipeline(cmdList, gfxPipeline);
		VkMana::CommandListSetViewport(cmdList, { 0, 0, WindowWidth, WindowHeight });
		VkMana::CommandListSetScissor(cmdList, { 0, 0, WindowWidth, WindowHeight });
		VkMana::CommandListSetPipelineStateCullMode(cmdList, VkMana::CullMode::None);
		VkMana::CommandListSetPipelineStateFrontFace(cmdList, VkMana::FrontFace::AntiClockwise);
		VkMana::CommandListSetPipelineConstants(cmdList, VkMana::ShaderStage::Vertex, 0, sizeof(glm::mat4), glm::value_ptr(transform));
		VkMana::CommandListDraw(cmdList, 3, 0);
		VkMana::CommandListEnd(cmdList);

		VkMana::SubmitCommandList(cmdList);
		//		graphicsDevice->SwapBuffers();

		VkMana::SwapBuffers(graphicsDevice);

		std::this_thread::sleep_for(std::chrono::milliseconds(33));
	}

	VkMana::WaitForIdle(graphicsDevice);

	VkMana::DestroyPipeline(gfxPipeline);
	VkMana::DestroyFramebuffer(offscreenFramebuffer);
	VkMana::DestroyCommandList(cmdList);
	VkMana::DestroyTexture(texture);
	VkMana::DestroyBuffer(buffer);
	VkMana::DestroyGraphicDevice(graphicsDevice);

	SDL_Quit();

	return 0;
}