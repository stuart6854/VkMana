#pragma once

#include <cstdint>

namespace VkMana
{
	enum class PixelFormat : std::uint8_t
	{
		None,
		/** RGBA component order. Each component is an 8-bit unsigned normalized integer. */
		R8_G8_B8_A8_UNorm,
		/** BGRA component order. Each component is an 8-bit unsigned normalized integer. */
		B8_G8_R8_A8_UNorm,
		/** Single-channel, 8-bit unsigned normalized integer. */
		R8_UNorm,
		/** Single-channel, 16-bit unsigned normalized integer. Can be used as a depth format. */
		R16_UNorm,
		/** RGBA component order. Each component is a 32-bit signed floating-point value. */
		R32_G32_B32_A32_Float,
		/** Single-channel, 32-bit signed floating-point value. Can be used as a depth format. */
		R32_Float,
		/** BC3 block compressed format. */
		BC3_UNorm,
		/**
		 * A depth-stencil format where the depth is stored in a 24-bit unsigned normalized integer, and the stencil is stored
		 * in an 8-bit unsigned integer.
		 */
		D24_UNorm_S8_UInt,
		/**
		 * A depth-stencil format where the depth is stored in a 32-bit signed floating-point value, and the stencil is stored
		 * in an 8-bit unsigned integer.
		 */
		D32_Float_S8_UInt,
		/** RGBA component order. Each component is a 32-bit unsigned integer. */
		R32_G32_B32_A32_UInt,
		/** RG component order. Each component is an 8-bit signed normalized integer. */
		R8_G8_SNorm,
		/** BC1 block compressed format with no alpha. */
		BC1_Rgb_UNorm,
		/** BC1 block compressed format with a single-bit alpha channel. */
		BC1_Rgba_UNorm,
		/** BC2 block compressed format. */
		BC2_UNorm,
		/**
		 * A 32-bit packed format. The 10-bit R component occupies bits 0..9, the 10-bit G component occupies bits 10..19,
		 * the 10-bit A component occupies 20..29, and the 2-bit A component occupies bits 30..31. Each value is an unsigned,
		 * normalized integer.
		 */
		R10_G10_B10_A2_UNorm,
		/**
		 * A 32-bit packed format. The 10-bit R component occupies bits 0..9, the 10-bit G component occupies bits 10..19,
		 * the 10-bit A component occupies 20..29, and the 2-bit A component occupies bits 30..31. Each value is an unsigned
		 * integer.
		 */
		R10_G10_B10_A2_UInt,
		/**
		 * A 32-bit packed format. The 11-bit R componnent occupies bits 0..10, the 11-bit G component occupies bits 11..21,
		 * and the 10-bit B component occupies bits 22..31. Each value is an unsigned floating point value.
		 */
		R11_G11_B10_Float,
		/** Single-channel, 8-bit signed normalized integer. */
		R8_SNorm,
		/** Single-channel, 8-bit unsigned integer. */
		R8_UInt,
		/** Single-channel, 8-bit signed integer. */
		R8_SInt,
		/** Single-channel, 16-bit signed normalized integer. */
		R16_SNorm,
		/** Single-channel, 16-bit unsigned integer. */
		R16_UInt,
		/** Single-channel, 16-bit signed integer. */
		R16_SInt,
		/** Single-channel, 16-bit signed floating-point value. */
		R16_Float,
		/** Single-channel, 32-bit unsigned integer */
		R32_UInt,
		/** Single-channel, 32-bit signed integer */
		R32_SInt,
		/** RG component order. Each component is an 8-bit unsigned normalized integer. */
		R8_G8_UNorm,
		/** RG component order. Each component is an 8-bit unsigned integer. */
		R8_G8_UInt,
		/** RG component order. Each component is an 8-bit signed integer. */
		R8_G8_SInt,
		/** RG component order. Each component is a 16-bit unsigned normalized integer. */
		R16_G16_UNorm,
		/** RG component order. Each component is a 16-bit signed normalized integer. */
		R16_G16_SNorm,
		/** RG component order. Each component is a 16-bit unsigned integer. */
		R16_G16_UInt,
		/** RG component order. Each component is a 16-bit signed integer. */
		R16_G16_SInt,
		/** RG component order. Each component is a 16-bit signed floating-point value. */
		R16_G16_Float,
		/** RG component order. Each component is a 32-bit unsigned integer. */
		R32_G32_UInt,
		/** RG component order. Each component is a 32-bit signed integer. */
		R32_G32_SInt,
		/** RG component order. Each component is a 32-bit signed floating-point value. */
		R32_G32_Float,
		/** RGBA component order. Each component is an 8-bit signed normalized integer. */
		R8_G8_B8_A8_SNorm,
		/** RGBA component order. Each component is an 8-bit unsigned integer. */
		R8_G8_B8_A8_UInt,
		/** RGBA component order. Each component is an 8-bit signed integer. */
		R8_G8_B8_A8_SInt,
		/** RGBA component order. Each component is a 16-bit unsigned normalized integer. */
		R16_G16_B16_A16_UNorm,
		/** RGBA component order. Each component is a 16-bit signed normalized integer. */
		R16_G16_B16_A16_SNorm,
		/** RGBA component order. Each component is a 16-bit unsigned integer. */
		R16_G16_B16_A16_UInt,
		/** RGBA component order. Each component is a 16-bit signed integer. */
		R16_G16_B16_A16_SInt,
		/** RGBA component order. Each component is a 16-bit floating-point value. */
		R16_G16_B16_A16_Float,
		/** RGBA component order. Each component is a 32-bit signed integer. */
		R32_G32_B32_A32_SInt,
		/** A 64-bit, 4x4 block-compressed format storing unsigned normalized RGB data. */
		ETC2_R8_G8_B8_UNorm,
		/** A 64-bit, 4x4 block-compressed format storing unsigned normalized RGB data, as well as 1 bit of alpha data. */
		ETC2_R8_G8_B8_A1_UNorm,
		/**
		 * A 128-bit, 4x4 block-compressed format storing 64 bits of unsigned normalized RGB data, as well as 64 bits of alpha
		 * data.
		 */
		ETC2_R8_G8_B8_A8_UNorm,
		/** BC4 block compressed format, unsigned normalized values. */
		BC4_UNorm,
		/** BC4 block compressed format, signed normalized values. */
		BC4_SNorm,
		/** BC5 block compressed format, unsigned normalized values. */
		BC5_UNorm,
		/** BC5 block compressed format, signed normalized values. */
		BC5_SNorm,
		/** BC7 block compressed format. */
		BC7_UNorm,
		/**
		 * RGBA component order. Each component is an 8-bit unsigned normalized integer.
		 * This is an sRGB format.
		 */
		R8_G8_B8_A8_UNorm_SRgb,
		/** BGRA component order. Each component is an 8-bit unsigned normalized integer. */
		B8_G8_R8_A8_UNorm_SRgb,
		/**
		 * BC1 block compressed format with no alpha.
		 * This is an sRGB format.
		 */
		BC1_Rgb_UNorm_SRgb,
		/**
		 * BC1 block compressed format with a single-bit alpha channel.
		 * This is an sRGB format.
		 */
		BC1_Rgba_UNorm_SRgb,
		/**
		 * BC2 block compressed format.
		 * This is an sRGB format.
		 */
		BC2_UNorm_SRgb,
		/**
		 * BC3 block compressed format.
		 * This is an sRGB format.
		 */
		BC3_UNorm_SRgb,
		/**
		 * BC7 block compressed format.
		 * This is an sRGB format.
		 */
		BC7_UNorm_SRgb,
	};

	enum class IndexFormat
	{
		UInt16,
		UInt32,
	};

} // namespace VkMana