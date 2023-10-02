#pragma once

#include "Enums.hpp"
#include "SwapchainSource.hpp"

#include <cstdint>
#include <utility>

namespace VkMana
{
	struct SwapchainDescription
	{
		std::shared_ptr<SwapchainSource> Source;

		std::uint32_t Width;
		std::uint32_t Height;

		PixelFormat DepthFormat;

		bool SyncToVerticalBlank;

		bool ColorSrgb;

		SwapchainDescription(std::shared_ptr<SwapchainSource> source, std::uint32_t width, std::uint32_t height, PixelFormat depthFormat, bool syncToVerticalBlank, bool colorSrgb = false)
			: Source(std::move(source)), Width(width), Height(height), DepthFormat(depthFormat), SyncToVerticalBlank(syncToVerticalBlank), ColorSrgb(colorSrgb)
		{
		}
	};
} // namespace VkMana