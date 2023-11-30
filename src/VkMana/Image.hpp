#pragma once

#include "Vulkan_Common.hpp"

#include <array>

namespace VkMana
{
	class Context;

	class Image;
	class ImageView;
	using ImageViewHandle = IntrusivePtr<ImageView>;

	constexpr auto ImageCreateFlags_GenMipMaps = (1 << 0);

	struct ImageCreateInfo
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Depth = 1;
		int32_t MipLevels = -1; // -1 = Automatically determine max mip levels.
		uint32_t ArrayLayers = 1;
		vk::Format Format = vk::Format::eUndefined;
		vk::ImageUsageFlags Usage;
		uint32_t Flags = 0;

		static auto ColorTarget(uint32_t width, uint32_t height, vk::Format format) -> ImageCreateInfo
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.MipLevels = 1,
				.Format = format,
				.Usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			};
		}
		static auto DepthStencilTarget(uint32_t width, uint32_t height, bool use32Bit) -> ImageCreateInfo
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.MipLevels = 1,
				.Format = use32Bit ? vk::Format::eD32SfloatS8Uint : vk::Format::eD24UnormS8Uint,
				.Usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
			};
		}
		static auto Texture(uint32_t width, uint32_t height, int32_t mipLevels = -1) -> ImageCreateInfo
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.MipLevels = mipLevels,
				.Format = vk::Format::eR8G8B8A8Unorm,
				.Usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc,
				.Flags = ImageCreateFlags_GenMipMaps,
			};
		}
	};
	struct ImageDataSource
	{
		uint64_t Size = 0;
		const void* Data = nullptr;
	};

	struct ImageViewCreateInfo
	{
		const Image* TargetImage = nullptr;
		uint32_t BaseMipLevel = 0;
		uint32_t MipLevelCount = 1;
		uint32_t BaseArrayLayer = 0;
		uint32_t ArrayLayerCount = 1;
	};

	enum class ImageViewType : uint8_t
	{
		Texture,
		RenderTarget,
		Count,
	};

	struct SamplerCreateInfo
	{
		vk::Filter MinFilter = vk::Filter::eLinear;
		vk::Filter MagFilter = vk::Filter::eLinear;
		vk::SamplerAddressMode AddressMode = vk::SamplerAddressMode::eRepeat;
		vk::SamplerMipmapMode MipMapMode = vk::SamplerMipmapMode::eLinear;
	};

	class Image : public IntrusivePtrEnabled<Image>
	{
	public:
		~Image();

		auto GetImageView(ImageViewType type) -> ImageView*;

		auto GetImage() const -> auto { return m_image; }
		auto GetWidth() const -> auto { return m_info.Width; }
		auto GetHeight() const -> auto { return m_info.Height; }
		auto GetDepth() const -> auto { return m_info.Depth; }
		auto GetMipLevels() const -> auto { return uint32_t(m_info.MipLevels); }
		auto GetArrayLayers() const -> auto { return m_info.ArrayLayers; }
		auto GetFormat() const -> auto { return m_info.Format; }
		auto GetAspect() const -> vk::ImageAspectFlags;

	private:
		friend class Context;

		Image(Context* context, vk::Image image, const ImageCreateInfo& info);
		Image(Context* context, vk::Image image, vma::Allocation allocation, const ImageCreateInfo& info);

	private:
		Context* m_ctx;
		vk::Image m_image;
		vma::Allocation m_allocation;
		ImageCreateInfo m_info;
		bool m_ownsImage;

		std::array<ImageViewHandle, uint8_t(ImageViewType::Count)> m_views;
	};
	using ImageHandle = IntrusivePtr<Image>;

	class ImageView : public IntrusivePtrEnabled<ImageView>
	{
	public:
		~ImageView();

		auto GetImage() const -> auto { return m_image; }
		auto GetView() const -> auto { return m_view; }

	private:
		friend class Context;

		ImageView(Context* context, const Image* image, vk::ImageView view, const ImageViewCreateInfo& info);

	private:
		Context* m_ctx;
		const Image* m_image;
		vk::ImageView m_view;
		ImageViewCreateInfo m_info;
	};

	class Sampler : public IntrusivePtrEnabled<Sampler>
	{
	public:
		~Sampler();

		auto GetSampler() const -> auto { return m_sampler; }

	private:
		friend class Context;

		Sampler(Context* context, vk::Sampler sampler);

	private:
		Context* m_ctx;
		vk::Sampler m_sampler;
	};
	using SamplerHandle = IntrusivePtr<Sampler>;

} // namespace VkMana
