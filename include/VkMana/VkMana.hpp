/**
 * TODO:
 * - Buffer Upload
 * - Texture Upload
 * - Push Constants
 * - Binding Sets (Shader resources)
 * - Fences (Public API)
 * - MipMap Generation
 * - Compute
 * - Framebuffers - vk::RenderPass (if Dynamic rendering extension not supported.)
 */

#pragma once

#include "Enums.hpp"
#include "Structs.hpp"

#include <cstdint>
#include <vector>

// Win32
class HINSTANCE__;
using HINSTANCE = HINSTANCE__*;
class HWND__;
using HWND = HWND__*;

// X11
class Display;
class Window;

// Wayland
struct wl_display;
struct wl_surface;

namespace VkMana
{
#define VKMANA_DEFINE_HANDLE(object) typedef struct object##_T* object

	VKMANA_DEFINE_HANDLE(GraphicsDevice);
	VKMANA_DEFINE_HANDLE(Swapchain);
	VKMANA_DEFINE_HANDLE(DeviceBuffer);
	VKMANA_DEFINE_HANDLE(Texture);
	VKMANA_DEFINE_HANDLE(Framebuffer);
	VKMANA_DEFINE_HANDLE(CommandList);

	struct SurfaceProvider;

	struct SwapchainCreateInfo
	{
		SurfaceProvider* SurfaceProvider = nullptr;
		std::uint32_t Width = 0;
		std::uint32_t Height = 0;
		bool VSync = false;
		bool Srgb = false;
		RgbaFloat ClearColor = Rgba_Black;
	};
	struct GraphicsDeviceCreateInfo
	{
		bool Debug = false;
		SwapchainCreateInfo MainSwapchainCreateInfo = {};
	};
	struct BufferCreateInfo
	{
		std::uint64_t Size = 0;
		BufferUsage Usage = BufferUsage::None;
	};
	struct TextureCreateInfo
	{
		std::uint32_t Width = 0;
		std::uint32_t Height = 0;
		std::uint32_t Depth = 1;
		std::uint32_t MipLevels = 1;
		std::uint32_t ArrayLayers = 1;
		PixelFormat Format = PixelFormat::None;
		TextureUsage Usage = TextureUsage::None;
		//		TextureType Type = TextureType::None;
	};
	struct FramebufferAttachmentCreateInfo
	{
		Texture TargetTexture = nullptr;
		std::uint32_t MipLevel = 0;
		std::uint32_t ArrayLayer = 0;
		RgbaFloat ClearColor = Rgba_Black;
	};
	struct FramebufferCreateInfo
	{
		std::vector<FramebufferAttachmentCreateInfo> ColorAttachments;
		FramebufferAttachmentCreateInfo DepthAttachment;
	};

	auto CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) -> GraphicsDevice;
	auto CreateSwapchain(GraphicsDevice graphicsDevice, const SwapchainCreateInfo& createInfo) -> Swapchain;
	auto CreateBuffer(GraphicsDevice graphicsDevice, const BufferCreateInfo& createInfo) -> DeviceBuffer;
	auto CreateTexture(GraphicsDevice graphicsDevice, const TextureCreateInfo& createInfo) -> Texture;
	auto CreateFramebuffer(GraphicsDevice graphicsDevice, const FramebufferCreateInfo& createInfo) -> Framebuffer;
	auto CreateCommandList(GraphicsDevice graphicsDevice) -> CommandList;

	bool DestroyCommandList(CommandList commandList);
	bool DestroyFramebuffer(Framebuffer framebuffer);
	bool DestroyTexture(Texture texture);
	bool DestroyBuffer(DeviceBuffer buffer);
	bool DestroySwapchain(Swapchain swapchain);
	bool DestroyGraphicDevice(GraphicsDevice graphicsDevice);

	void WaitForIdle(GraphicsDevice graphicsDevice);

	void SwapBuffers(GraphicsDevice graphicsDevice, Swapchain swapchain = nullptr);

	auto GraphicsDeviceGetMainSwapchain(GraphicsDevice graphicsDevice) -> Swapchain;
	auto SwapchainGetFramebuffer(Swapchain swapchain) -> Framebuffer;

	void BufferUpdateData(DeviceBuffer buffer, std::uint64_t dstOffset, std::uint64_t dataSize, const void* data);

	/*************************************************************
	 * Command List Recording
	 ************************************************************/

	void CommandListBegin(CommandList commandList);
	void CommandListEnd(CommandList commandList);

	void CommandListBindFramebuffer(CommandList commandList, Framebuffer framebuffer);

	void SubmitCommandList(CommandList commandList);

	/*************************************************************
	 * Platform
	 ************************************************************/

	struct SurfaceProvider
	{
		virtual ~SurfaceProvider() = default;
	};
	struct Win32Surface : public SurfaceProvider
	{
		HINSTANCE HInstance;
		HWND HWnd;
	};
	struct X11Surface : public SurfaceProvider
	{
		Display* Display;
		Window* Window;
	};
	struct WaylandSurface : public SurfaceProvider
	{
		wl_display* Display;
		wl_surface* Surface;
	};

} // namespace VkMana