#include "Image.hpp"

#include "Context.hpp"

namespace VkMana
{
    auto Image::New(Context* pContext, const ImageCreateInfo& info) -> IntrusivePtr<Image>
    {
        auto actualMipLevels = info.MipLevels;
        if(actualMipLevels == -1)
            actualMipLevels = int32_t(std::floor(std::log2(std::max(info.Width, info.Height)))) + 1;

        vk::ImageCreateInfo imageInfo{};
        imageInfo.setExtent({ info.Width, info.Height, info.depthOrArrayLayers });
        imageInfo.setMipLevels(uint32_t(actualMipLevels));
        imageInfo.setArrayLayers(info.ArrayLayers);
        imageInfo.setFormat(info.Format);
        imageInfo.setUsage(info.Usage);
        imageInfo.setImageType(vk::ImageType::e2D);        // #TODO: Make auto.
        imageInfo.setSamples(vk::SampleCountFlagBits::e1); // #TODO: Make optional.

        vma::AllocationCreateInfo allocInfo{};

        auto [image, allocation] = pContext->GetAllocator().createImage(imageInfo, allocInfo);
        if(image == nullptr || !allocation)
        {
            VM_ERR("Failed to create Image");
            return nullptr;
        }

        auto pNewImage = IntrusivePtr(new Image(pContext, image, allocation, info.Width, info.Height, info.depthOrArrayLayers, actualMipLevels, info.Format));
        // #TODO: Auto create image views from info.Usage
        return pNewImage;
    }

    Image::~Image()
    {
        if(m_image && m_ownsImage)
            m_ctx->DestroyImage(m_image);

        if(m_allocation)
            m_ctx->DestroyAllocation(m_allocation);
    }

    auto Image::GetImageView(ImageViewType type) -> ImageView*
    {
        auto& view = m_views.at(uint8_t(type));
        if(!view)
        {
            ImageViewCreateInfo viewInfo;
            if(type == ImageViewType::Texture)
            {
                viewInfo = {
                    .TargetImage = this,
                    .BaseMipLevel = 0,
                    .MipLevelCount = uint32_t(m_mipLevels),
                    .BaseArrayLayer = 0,
                    .ArrayLayerCount = 1,
                };
            }
            else if(type == ImageViewType::RenderTarget)
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
        if(FormatIsColor(m_format))
            aspect = vk::ImageAspectFlagBits::eColor;
        else
        {
            if(FormatHasDepth(m_format))
                aspect |= vk::ImageAspectFlagBits::eDepth;
            if(FormatHasStencil(m_format))
                aspect |= vk::ImageAspectFlagBits::eStencil;
        }
        return aspect;
    }

    Image::Image(
        Context* context,
        vk::Image image,
        vma::Allocation allocation,
        uint32_t width,
        uint32_t height,
        uint32_t depthOrArrayLayers,
        uint32_t mipLevels,
        vk::Format format
    )
        : m_ctx(context)
        , m_image(image)
        , m_allocation(allocation)
        , m_ownsImage(true)
        , m_width(width)
        , m_height(height)
        , m_depthOrArrayLayers(depthOrArrayLayers)
        , m_mipLevels(mipLevels)
        , m_format(format)

    {
    }

    Image::Image(Context* context, vk::Image image, uint32_t width, uint32_t height, vk::Format format)
        : m_ctx(context)
        , m_image(image)
        , m_allocation(nullptr)
        , m_ownsImage(false)
        , m_width(width)
        , m_height(height)
        , m_depthOrArrayLayers(1)
        , m_mipLevels(1)
        , m_format(format)
    {
    }

    ImageView::~ImageView()
    {
        if(m_view)
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
        if(m_sampler)
            m_ctx->DestroySampler(m_sampler);
    }

    Sampler::Sampler(Context* context, vk::Sampler sampler)
        : m_ctx(context)
        , m_sampler(sampler)
    {
    }

} // namespace VkMana