#pragma once

#include "Vulkan_Common.hpp"

#include <array>

namespace VkMana
{
	class Context;

	class Image;
	class ImageView;
	using ImageViewHandle = IntrusivePtr<ImageView>;

	struct ImageCreateInfo
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Depth = 1;
		uint32_t MipLevels = 1;
		uint32_t ArrayLayers = 1;
		vk::Format Format = vk::Format::eUndefined;
		vk::ImageUsageFlags Usage;

		static auto ColorTarget(uint32_t width, uint32_t height, vk::Format format) -> ImageCreateInfo
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.Format = format,
				.Usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
			};
		}
		static auto DepthStencilTarget(uint32_t width, uint32_t height, bool use32Bit) -> ImageCreateInfo
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.Format = use32Bit ? vk::Format::eD32SfloatS8Uint : vk::Format::eD24UnormS8Uint,
				.Usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
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
		const Image* Image = nullptr;
		uint32_t BaseMipLevel = 0;
		uint32_t MipLevelCount = 1;
		uint32_t BaseArrayLayer = 0;
		uint32_t ArrayLayerCount = 1;
	};

	enum class ImageViewType : uint8_t
	{
		RenderTarget,
		Count,
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

} // namespace VkMana
