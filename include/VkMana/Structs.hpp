#pragma once

#include <cstdint>

namespace VkMana
{
	struct Viewport
	{
		std::int32_t X;
		std::int32_t Y;
		std::int32_t Width;
		std::int32_t Height;
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