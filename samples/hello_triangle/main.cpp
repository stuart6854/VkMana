#define SDL_MAIN_HANDLED
// #include <SDL.h>
// #include <SDL_syswm.h>
// #define VKMANA_USE_SDL2
#define VULKAN_DEBUG
#include <VkMana/Logging.hpp>
#include <VkMana/Context.hpp>
#include <VkMana/Device.hpp>
#include <VkMana/Buffer.hpp>
#include <VkMana/Image.hpp>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <string>
#include <memory>

// #TODO: Sdl2Window wrapper

/*auto GetSwapchainProvider(SDL_Window* window) -> std::shared_ptr<VkMana::SurfaceProvider>
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
}*/

const std::string TriangleVertexShaderSrc = R"(
#version 450

layout(push_constant) uniform Constant
{
	mat4 projMatrix;
	mat4 viewMatrix;
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
	gl_Position = uConstants.projMatrix * uConstants.viewMatrix * uConstants.modelMatrix * vec4(positions[gl_VertexIndex], 1.0f);
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

auto CreateBuffer(VkMana::Device& device) -> VkMana::BufferHandle
{
	VkMana::BufferCreateInfo info{};
	info.Size = 64;
	info.Domain = VkMana::BufferDomain::Device;
	info.Usage = vk::BufferUsageFlagBits::eStorageBuffer;
	const void* initialData = nullptr;
	auto buffer = device.CreateBuffer(info, initialData);
	return buffer;
}

auto CreateImage(VkMana::Device& device) -> VkMana::ImageHandle
{
	auto info = VkMana::ImageCreateInfo::Immutable2DImage(4, 4, vk::Format::eR8G8B8A8Unorm);
	info.InitialLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	info.Levels = 0;
	//	info.Misc = VkMana::ImageMisc::GenerateMips;
	const void* initialData = nullptr;
	auto image = device.CreateImage(info, initialData);
	return image;
}

int main()
{
	//	std::cout << "Sample - Hello Triangle" << '\n';

	VkMana::Context context;
	if (!context.InitInstanceAndDevice({}, {}))
	{
		LOG_ERR("Failed to create VkInstance and VkDevice.");
		return 1;
	}

	VkMana::Device device;
	device.SetContext(context);

	auto buffer = CreateBuffer(device);
	auto image = CreateImage(device);

	return 0;
}