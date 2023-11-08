#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_syswm.h>
#define VULKAN_DEBUG
#include <VkMana/Logging.hpp>
#include <VkMana/Context.hpp>
#include <VkMana/ShaderCompiler.hpp>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <string>

const std::string VertexShaderSrc = R"(
#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstants
{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} uConsts;

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

	gl_Position = uConsts.projMatrix * uConsts.viewMatrix * uConsts.modelMatrix * vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colors[gl_VertexIndex];
}
)";
const std::string FragmentShaderSrc = R"(
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
constexpr auto WindowAspect = float(WindowWidth) / float(WindowHeight);

/*auto CreateBuffer(VkMana::Device& device) -> VkMana::BufferHandle
{
	VkMana::BufferCreateInfo info{};
	info.Size = 64;
	info.Domain = VkMana::BufferDomain::Device;
	info.Usage = vk::BufferUsageFlagBits::eStorageBuffer;
	const void* initialData = nullptr;
	auto buffer = device.CreateBuffer(info, initialData);
	return buffer;
}*/

/*auto CreateImage(VkMana::Device& device) -> VkMana::ImageHandle
{
	auto info = VkMana::ImageCreateInfo::Immutable2DImage(4, 4, vk::Format::eR8G8B8A8Unorm);
	info.InitialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	info.Levels = 0;
	//	info.Misc = VkMana::ImageMisc::GenerateMips;
	const void* initialData = nullptr;
	auto image = device.CreateImage(info, initialData);
	return image;
}*/

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

	auto* window = SDL_CreateWindow(
		"VkMana - Sandbox", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	SDL2WSI wsi(window);

	VkMana::Context context;
	if (!context.Init(&wsi))
	{
		LOG_ERR("Failed to initialise device.");
		return 1;
	}

	std::vector<vk::DescriptorSetLayoutBinding> setBindings{
		{ 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll },
	};
	auto setLayout = context.CreateSetLayout(setBindings);

	VkMana::PipelineLayoutCreateInfo pipelineLayoutInfo{
		.PushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, 192 },
		.SetLayouts = {
			// setLayout.Get(),
		},
	};
	auto pipelineLayout = context.CreatePipelineLayout(pipelineLayoutInfo);

	std::vector<uint32_t> vertSpirv;
	if (!VkMana::CompileShader(vertSpirv, VertexShaderSrc, shaderc_vertex_shader, true))
	{
		LOG_ERR("Failed to compiler VERTEX shader.");
		return 1;
	}
	std::vector<uint32_t> fragSpirv;
	if (!VkMana::CompileShader(fragSpirv, FragmentShaderSrc, shaderc_fragment_shader, true))
	{
		LOG_ERR("Failed to compiler FRAGMENT shader.");
		return 1;
	}

	VkMana::GraphicsPipelineCreateInfo pipelineInfo{
		.Vertex = vertSpirv,
		.Fragment = fragSpirv,
		.Topology = vk::PrimitiveTopology::eTriangleList,
		.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
		.Layout = pipelineLayout.Get(),
	};
	auto pipeline = context.CreateGraphicsPipeline(pipelineInfo);

	struct PushConstants
	{
		glm::mat4 projMatrix = glm::perspective(glm::radians(60.0f), WindowAspect, 0.1f, 600.0f);
		glm::mat4 viewMatrix = glm::lookAtRH(glm::vec3(0, 1, -2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		glm::mat4 modelMatrix = glm::mat4(1.0f);
	} pushConsts;

	bool isRunning = true;
	while (isRunning)
	{
		if (!wsi.IsAlive())
			isRunning = false;

		pushConsts.modelMatrix = glm::rotate(pushConsts.modelMatrix, glm::radians(1.0f), glm::vec3(0, 1, 0));

		context.BeginFrame();
		auto cmd = context.RequestCmd();

		/**
		 * Update Uniforms & Descriptors
		 */

		auto globalSet = context.RequestDescriptorSet(setLayout.Get());
		// globalSet->Write(imageView, sampler, 0);
		// globalSet->Write(buffer, 1, 0, 512);

		/**
		 * Render
		 */

		auto rpInfo = context.GetSurfaceRenderPass(&wsi);
		cmd->BeginRenderPass(rpInfo);
		cmd->BindPipeline(pipeline.Get());
		cmd->SetViewport(0, 0, WindowWidth, WindowHeight);
		cmd->SetScissor(0, 0, WindowWidth, WindowHeight);
		cmd->SetPushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), &pushConsts);
		cmd->Draw(3, 0);
		cmd->EndRenderPass();

		context.Submit(cmd);

		context.EndFrame();
		context.Present();
	}

	return 0;
}