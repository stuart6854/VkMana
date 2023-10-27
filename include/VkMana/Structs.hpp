#pragma once

#include <cstdint>

namespace VkMana
{
	struct Viewport
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Width = 0.0f;
		float Height = 0.0f;
		float MinDepth = 0.0f;
		float MaxDepth = 1.0f;
	};
	struct Scissor
	{
		std::int32_t X = 0;
		std::int32_t Y = 0;
		std::uint32_t Width = 0;
		std::uint32_t Height = 0;
	};

	struct RgbaFloat
	{
		float R = 0.0f;
		float G = 0.0f;
		float B = 0.0f;
		float A = 1.0f;
	};
	constexpr auto Rgba_Transparent = RgbaFloat{ 0.0f, 0.0f, 0.0f, 0.0f };
	constexpr auto Rgba_Black = RgbaFloat{ 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr auto Rgba_White = RgbaFloat{ 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr auto Rgba_CornflowerBlue = RgbaFloat{ 0.392f, 0.584f, 0.929f, 1.0f };

} // namespace VkMana