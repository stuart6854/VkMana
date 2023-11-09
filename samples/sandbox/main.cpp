#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>
#include <SDL_syswm.h>
#define VULKAN_DEBUG
#include <VkMana/Logging.hpp>
#include <VkMana/Context.hpp>
#include <VkMana/ShaderCompiler.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>

const std::string VertexShaderSrc = R"(
#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec2 outTexCoord;

layout(push_constant) uniform PushConstants
{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} uConsts;

void main()
{
	gl_Position = uConsts.projMatrix * uConsts.viewMatrix * uConsts.modelMatrix * vec4(aPosition, 1.0f);
	outTexCoord = aTexCoord;
}
)";
const std::string FragmentShaderSrc = R"(
#version 450

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(inTexCoord, 0, 1);
}
)";

constexpr auto WindowTitle = "VkMana - Sandbox";
constexpr auto WindowWidth = 1280;
constexpr auto WindowHeight = 720;
constexpr auto WindowAspect = float(WindowWidth) / float(WindowHeight);

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;

	bool operator==(const Vertex& other) const
	{
		return Position == other.Position && Normal == other.Normal && TexCoord == other.TexCoord;
	}
};

namespace std
{
	template <>
	struct hash<Vertex>
	{
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.Position) ^ (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1)
				^ (hash<glm::vec2>()(vertex.TexCoord) << 1);
		}
	};
} // namespace std

struct Mesh
{
	VkMana::BufferHandle VertexBuffer;
	VkMana::BufferHandle IndexBuffer;
	uint32_t IndexCount = 0;
};

bool LoadObjMesh(Mesh& outMesh, VkMana::Context& context, const std::string& filename)
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
	{
		LOG_ERR("{}", err);
		if (!warn.empty())
			LOG_WARN("{}", warn);
		return false;
	}

	std::unordered_map<Vertex, uint16_t> uniqueVertices{};

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};
			vertex.Position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2],
			};
			vertex.Normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2],
			};
			vertex.TexCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1],
			};
			if (!uniqueVertices.contains(vertex))
			{
				uniqueVertices[vertex] = uint16_t(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	VkMana::BufferDataSource vtxDataSrc{
		.Size = sizeof(Vertex) * vertices.size(),
		.Data = vertices.data(),
	};
	auto vtxBufferInfo = VkMana::BufferCreateInfo::Vertex(vtxDataSrc.Size);
	outMesh.VertexBuffer = context.CreateBuffer(vtxBufferInfo, &vtxDataSrc);

	VkMana::BufferDataSource idxDataSrc{
		.Size = sizeof(uint16_t) * indices.size(),
		.Data = indices.data(),
	};
	auto idxBufferInfo = VkMana::BufferCreateInfo::Index(idxDataSrc.Size);
	outMesh.IndexBuffer = context.CreateBuffer(idxBufferInfo, &idxDataSrc);

	outMesh.IndexCount = indices.size();

	return true;
}

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
	LOG_INFO("Path: {}", std::filesystem::current_path().string());

	auto* window = SDL_CreateWindow(
		WindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

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
		.VertexAttributes = {
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Position)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal)),
			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord)),
		},
		.VertexBindings = {
			vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex),
		},
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

	Mesh mesh;
	if (!LoadObjMesh(mesh, context, "assets/models/viking_room.obj"))
	{
		LOG_ERR("Failed to load mesh.");
		return 1;
	}

	float lastFrameTime = SDL_GetTicks() / 1000.0f;
	bool isRunning = true;
	while (isRunning)
	{
		auto currentFrameTime = SDL_GetTicks() / 1000.0f;
		auto deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;

		if (!wsi.IsAlive())
			isRunning = false;

		auto windowTitle = std::format("{} - {}ms", WindowTitle, uint32_t(deltaTime * 1000));
		SDL_SetWindowTitle(window, windowTitle.c_str());

		// const auto rotDegrees = 45.0f * deltaTime;
		// pushConsts.modelMatrix = glm::rotate(pushConsts.modelMatrix, glm::radians(rotDegrees), glm::vec3(0, 1, 0));

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
		cmd->BindIndexBuffer(mesh.IndexBuffer.Get());
		cmd->BindVertexBuffers(0, { mesh.VertexBuffer.Get() }, { 0 });
		cmd->DrawIndexed(mesh.IndexCount, 0, 0);
		cmd->EndRenderPass();

		context.Submit(cmd);

		context.EndFrame();
		context.Present();
	}

	return 0;
}