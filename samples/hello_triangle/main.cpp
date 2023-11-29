#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>
#define VULKAN_DEBUG
#include <VkMana/Logging.hpp>
#include <VkMana/Context.hpp>
#include <VkMana/ShaderCompiler.hpp>

#include <chrono>
#include <iostream>
#include <string>

const std::string TriangleVertexShaderSrc = R"(
#version 450

layout(location = 0) out vec4 outColor;

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(0.5, 0.5, 0.0),
		vec3(-0.5, 0.5, 0.0),
		vec3(0.0, -0.5, 0.0)
	);

	const vec4 colors[3] = vec4[3](
		vec4(1.0, 0.0, 0.0, 1.0),
		vec4(0.0, 1.0, 0.0, 1.0),
		vec4(0.0, 0.0, 1.0, 1.0)
	);

	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colors[gl_VertexIndex];
}
)";
const std::string TriangleFragmentShaderSrc = R"(
#version 450

layout (location = 0) in vec4 inColor;
layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = inColor;
}
)";

constexpr auto WindowWidth = 1280;
constexpr auto WindowHeight = 720;

class SDL2WSI : public VkMana::WSI
{
public:
	explicit SDL2WSI(SDL_Window* window)
		: m_window(window)
	{
	}
	~SDL2WSI() override = default;

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

int main()
{
	VM_INFO("Sample - Hello Triangle");

	auto* window = SDL_CreateWindow(
		"Hello Triangle", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	SDL2WSI wsi(window);

	VkMana::Context context;
	if (!context.Init(&wsi))
	{
		VM_ERR("Failed to initialise device.");
		return 1;
	}

	VkMana::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	auto pipelineLayout = context.CreatePipelineLayout(pipelineLayoutInfo);

	VkMana::ShaderCompileInfo compileInfo{
		.SrcLanguage = VkMana::SourceLanguage::GLSL,
		.SrcFilename = "",
		.SrcString = TriangleVertexShaderSrc,
		.Stage = vk::ShaderStageFlagBits::eVertex,
		.Debug = true,
	};
	auto vertSpirvOpt = VkMana::CompileShader(compileInfo);
	if (!vertSpirvOpt)
	{
		VM_ERR("Failed to compiler VERTEX shader.");
		return 1;
	}

	compileInfo.SrcString = TriangleFragmentShaderSrc;
	compileInfo.Stage = vk::ShaderStageFlagBits::eFragment;
	auto fragSpirvOpt = VkMana::CompileShader(compileInfo);
	if (!fragSpirvOpt)
	{
		VM_ERR("Failed to compiler FRAGMENT shader.");
		return 1;
	}

	VkMana::GraphicsPipelineCreateInfo pipelineInfo{
		.Vertex = vertSpirvOpt.value(),
		.Fragment = fragSpirvOpt.value(),
		.Topology = vk::PrimitiveTopology::eTriangleList,
		.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
		.Layout = pipelineLayout.Get(),
	};
	auto pipeline = context.CreateGraphicsPipeline(pipelineInfo);

	bool isRunning = true;
	while (isRunning)
	{
		if (!wsi.IsAlive())
			isRunning = false;

		context.BeginFrame();
		auto cmd = context.RequestCmd();

		/**
		 * Render
		 */

		auto rpInfo = context.GetSurfaceRenderPass(&wsi);
		cmd->BeginRenderPass(rpInfo);
		cmd->BindPipeline(pipeline.Get());
		cmd->SetViewport(0, 0, WindowWidth, WindowHeight);
		cmd->SetScissor(0, 0, WindowWidth, WindowHeight);
		cmd->Draw(3, 0);
		cmd->EndRenderPass();

		context.Submit(cmd);

		context.EndFrame();
		context.Present();
	}

	return 0;
}