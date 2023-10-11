#pragma once

#include <cstdint>
#include <type_traits>

namespace VkMana
{
// Define bitwise operators for an enum class, allowing usage as bitmasks.
#define DEFINE_ENUM_CLASS_BITWISE_OPERATORS(Enum)                                                                                        \
	inline constexpr Enum operator|(Enum Lhs, Enum Rhs)                                                                                  \
	{                                                                                                                                    \
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) | static_cast<std::underlying_type_t<Enum>>(Rhs));       \
	}                                                                                                                                    \
	inline constexpr Enum operator&(Enum Lhs, Enum Rhs)                                                                                  \
	{                                                                                                                                    \
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) & static_cast<std::underlying_type_t<Enum>>(Rhs));       \
	}                                                                                                                                    \
	inline constexpr Enum operator^(Enum Lhs, Enum Rhs)                                                                                  \
	{                                                                                                                                    \
		return static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) ^ static_cast<std::underlying_type_t<Enum>>(Rhs));       \
	}                                                                                                                                    \
	inline constexpr Enum operator~(Enum E)                                                                                              \
	{                                                                                                                                    \
		return static_cast<Enum>(~static_cast<std::underlying_type_t<Enum>>(E));                                                         \
	}                                                                                                                                    \
	inline Enum& operator|=(Enum& Lhs, Enum Rhs)                                                                                         \
	{                                                                                                                                    \
		return Lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) | static_cast<std::underlying_type_t<Enum>>(Lhs)); \
	}                                                                                                                                    \
	inline Enum& operator&=(Enum& Lhs, Enum Rhs)                                                                                         \
	{                                                                                                                                    \
		return Lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) & static_cast<std::underlying_type_t<Enum>>(Lhs)); \
	}                                                                                                                                    \
	inline Enum& operator^=(Enum& Lhs, Enum Rhs)                                                                                         \
	{                                                                                                                                    \
		return Lhs = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(Lhs) ^ static_cast<std::underlying_type_t<Enum>>(Lhs)); \
	}

	enum class BufferUsage : std::uint8_t
	{
		None,
		/** Vertex buffer. */
		Vertex,
		/** Index buffer. */
		Index,
		/** Uniform buffer. Implies host-accessible. */
		Uniform,
		/** Similar to Uniform buffer, but usually for much larger data. */
		Storage,
		/** Similar to Uniform buffer, but usually for much larger data. */
		HostAccessible,
	};
	DEFINE_ENUM_CLASS_BITWISE_OPERATORS(BufferUsage);

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
		/**
		 * RGBA component order. Each component is an 8-bit unsigned normalized integer.
		 * This is an sRGB format.
		 */
		R8_G8_B8_A8_UNorm_SRgb,
		/** BGRA component order. Each component is an 8-bit unsigned normalized integer. */
		B8_G8_R8_A8_UNorm_SRgb,
	};

	enum class TextureUsage : std::uint8_t
	{
		None = 0,
		/** Shader accessible. Read-Only. */
		Sampled = 1 << 0,
		/** Shader accessible. Read/Write. */
		Storage = 1 << 1,
		/** Color Target. */
		RenderTarget = 1 << 2,
		/** Depth/Stencil Target. */
		DepthStencil = 1 << 3,
		/** 2D CubeMap. */
		// CubeMap = 1 << 4,
		/** Read/Write staging resource for uploading texture data. */
		// Staging = 1 << 5,
		/** Texture supports automatic generation of mipmaps through CommandList. */
		// GenerateMipMaps = 1 << 6,
	};
	DEFINE_ENUM_CLASS_BITWISE_OPERATORS(TextureUsage);

	/*enum class TextureType : std::uint8_t
	{
		None,
		Texture1D,
		Texture2D,
		Texture3D,
	};*/

	enum class IndexFormat : std::uint8_t
	{
		UInt16,
		UInt32,
	};

} // namespace VkMana