#include "Image.hpp"

#include "Context.hpp"

namespace VkMana
{
	Image::~Image()
	{
		if (m_image && m_ownsImage)
			m_ctx->DestroyImage(m_image);

		if (m_allocation)
			m_ctx->DestroyAllocation(m_allocation);
	}

	auto Image::GetImageView(ImageViewType type) -> ImageView*
	{
		auto& view = m_views.at(uint8_t(type));
		if (!view)
		{
			ImageViewCreateInfo viewInfo;
			if (type == ImageViewType::Texture)
			{
				viewInfo = {
					.TargetImage = this,
					.BaseMipLevel = 0,
					.MipLevelCount = uint32_t(m_info.MipLevels),
					.BaseArrayLayer = 0,
					.ArrayLayerCount = 1,
				};
			}
			else if (type == ImageViewType::RenderTarget)
			{
				viewInfo = {
					.TargetImage = this,
					.BaseMipLevel = 0,
					.MipLevelCount = 1,
					.BaseArrayLayer = 0,
					.ArrayLayerCount = 1,
				};
			}
			view = m_ctx->CreateImageView(this, viewInfo);
		}
		return view.Get();
	}

	auto Image::GetAspect() const -> vk::ImageAspectFlags
	{
		vk::ImageAspectFlags aspect{};
		if (FormatIsColor(m_info.Format))
			aspect = vk::ImageAspectFlagBits::eColor;
		else
		{
			if (FormatHasDepth(m_info.Format))
				aspect |= vk::ImageAspectFlagBits::eDepth;
			if (FormatHasStencil(m_info.Format))
				aspect |= vk::ImageAspectFlagBits::eStencil;
		}
		return aspect;
	}

	Image::Image(Context* context, vk::Image image, const ImageCreateInfo& info)
		: m_ctx(context)
		, m_image(image)
		, m_info(info)
		, m_ownsImage(false)
	{
	}

	Image::Image(Context* context, vk::Image image, vma::Allocation allocation, const ImageCreateInfo& info)
		: m_ctx(context)
		, m_image(image)
		, m_allocation(allocation)
		, m_info(info)
		, m_ownsImage(true)
	{
	}

	ImageView::~ImageView()
	{
		if (m_view)
			m_ctx->DestroyImageView(m_view);
	}

	ImageView::ImageView(Context* context, const Image* image, vk::ImageView view, const ImageViewCreateInfo& info)
		: m_ctx(context)
		, m_image(image)
		, m_view(view)
		, m_info(info)
	{
	}

	Sampler::~Sampler()
	{
		if (m_sampler)
			m_ctx->DestroySampler(m_sampler);
	}

	Sampler::Sampler(Context* context, vk::Sampler sampler)
		: m_ctx(context)
		, m_sampler(sampler)
	{
	}

} // namespace VkMana