#pragma once

#include "Enums.hpp"
#include "Structs.hpp"

#include <cstdint>

namespace VkMana
{
#define VKMANA_DEFINE_HANDLE(object) typedef struct object##_T* object

	VKMANA_DEFINE_HANDLE(GraphicsDevice);
	VKMANA_DEFINE_HANDLE(DeviceBuffer);
	VKMANA_DEFINE_HANDLE(Texture);
	VKMANA_DEFINE_HANDLE(CommandList);

	struct GraphicsDeviceCreateInfo
	{
		bool Debug = false;
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

	auto CreateGraphicsDevice(const GraphicsDeviceCreateInfo& createInfo) -> GraphicsDevice;
	auto CreateBuffer(GraphicsDevice graphicsDevice, const BufferCreateInfo& createInfo) -> DeviceBuffer;
	auto CreateTexture(GraphicsDevice graphicsDevice, const TextureCreateInfo& createInfo) -> Texture;
	auto CreateCommandList(GraphicsDevice graphicsDevice) -> CommandList;

	bool DestroyCommandList(CommandList commandList);
	bool DestroyTexture(Texture texture);
	bool DestroyBuffer(DeviceBuffer buffer);
	bool DestroyGraphicDevice(GraphicsDevice graphicsDevice);

} // namespace VkMana