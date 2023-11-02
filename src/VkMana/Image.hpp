#pragma once

#include "Vulkan_Headers.hpp"

namespace VkMana
{
	class Device;

	enum class ImageDomain
	{
		Physical,
		Transient,
		LinearHost,
	};

	struct ImageCreateInfo
	{
		ImageDomain Domain = ImageDomain::Physical;
		std::uint32_t Width = 0;
		std::uint32_t Height = 0;
		std::uint32_t Depth = 1;
		std::uint32_t Levels = 1;
		vk::Format Format = vk::Format::eUndefined;
		vk::ImageType Type = vk::ImageType::e2D;
		std::uint32_t Layers = 1;
		vk::ImageUsageFlags Usage = {};
		vk::SampleCountFlags Samples = vk::SampleCountFlagBits::e1;
		vk::ImageCreateFlags Flags = {};
		vk::ImageLayout InitialLayout = vk::ImageLayout::eGeneral; // if != UNDEFINED, we need to submit an image barrier

		static auto Immutable2DImage(std::uint32_t width, std::uint32_t height, vk::Format format, bool mippmaped = false)
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.Depth = 1,
				.Levels = mippmaped ? 0u : 1u,
				.Format = format,
				.Type = vk::ImageType::e2D,
				.Layers = 1,
				.Usage = vk::ImageUsageFlagBits::eSampled,
				.Samples = vk::SampleCountFlagBits::e1,
				.Flags = {},
			};
		}

		static auto RenderTarget(std::uint32_t width, std::uint32_t height, vk::Format format)
		{
			return ImageCreateInfo{
				.Width = width,
				.Height = height,
				.Depth = 1,
				.Levels = 1,
				.Format = format,
				.Type = vk::ImageType::e2D,
				.Layers = 1,
				.Usage = vkuFormatIsDepthOrStencil(VkFormat(format)) ? vk::ImageUsageFlagBits::eDepthStencilAttachment
																	 : vk::ImageUsageFlagBits::eColorAttachment,
				.Samples = vk::SampleCountFlagBits::e1,
				.Flags = {},
			};
		}
	};

	class Image
	{
	public:
		~Image();
		Image(Image&&) = delete;
		void operator=(Image&&) = delete;

	protected:
		Image(Device* device,
			vk::Image image,
			vk::ImageView defaultView,
			vma::Allocation alloc,
			const ImageCreateInfo& info,
			vk::ImageViewType viewType);

	private:
		Device* m_device;
		vk::Image m_image;
		vma::Allocation m_alloc;
		ImageCreateInfo m_info;

		bool m_ownsImage = true;
	};
	using ImageHandle = std::shared_ptr<Image>;

} // namespace VkMana
