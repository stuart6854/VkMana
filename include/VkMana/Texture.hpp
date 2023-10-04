#pragma once

#include "Enums.hpp"

#include <cstdint>

namespace VkMana
{
	struct TextureDescription
	{
		std::uint32_t Width;
		std::uint32_t Height;
		std::uint32_t Depth;
		std::uint32_t MipLevels;
		std::uint32_t ArrayLayers;
		PixelFormat Format;
		TextureUsage Usage;
		TextureType Type;
		// TextureSampleCount SampleCount;
	};

	class GraphicsDevice;

	class Texture
	{
	public:
		explicit Texture(GraphicsDevice& graphicsDevice, const TextureDescription& description);
		/** Used for swapchain textures. */
		// explicit Texture(GraphicsDevice& graphicsDevice,
		// 	std::uint32_t width,
		// 	std::uint32_t height,
		// 	std::uint32_t mipLevels,
		// 	std::uint32_t arrayLayers,
		// 	vk::Format format,
		// 	TextureUsage usage,
		// 	vk::Image existingImage);
		~Texture();
	};
} // namespace VkMana
