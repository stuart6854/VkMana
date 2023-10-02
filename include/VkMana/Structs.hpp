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
		float R;
		float G;
		float B;
		float A;
	};

} // namespace VkMana