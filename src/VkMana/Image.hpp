#pragma once

#include "VulkanCommon.hpp"

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
        uint32_t width = 1;
        uint32_t height = 1;
        uint32_t depthOrArrayLayers = 1;
        int32_t mipLevels = -1; // -1 = Automatically determine max mip levels.
        vk::Format format = vk::Format::eUndefined;
        vk::ImageUsageFlags usage;
        uint32_t flags = 0;

        static auto ColorTarget(uint32_t width, uint32_t height, vk::Format format) -> ImageCreateInfo
        {
            return ImageCreateInfo{
                .width = width,
                .height = height,
                .mipLevels = 1,
                .format = format,
                .usage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
            };
        }
        static auto DepthStencilTarget(uint32_t width, uint32_t height, bool use32Bit) -> ImageCreateInfo
        {
            return ImageCreateInfo{
                .width = width,
                .height = height,
                .mipLevels = 1,
                .format = use32Bit ? vk::Format::eD32SfloatS8Uint : vk::Format::eD24UnormS8Uint,
                .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
            };
        }
        static auto Texture(uint32_t width, uint32_t height, int32_t mipLevels = -1) -> ImageCreateInfo
        {
            return ImageCreateInfo{
                .width = width,
                .height = height,
                .mipLevels = mipLevels,
                .format = vk::Format::eR8G8B8A8Unorm,
                .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc,
                .flags = ImageCreateFlags_GenMipMaps,
            };
        }
    };
    struct ImageDataSource
    {
        uint64_t size = 0;
        const void* data = nullptr;
    };

    struct ImageViewCreateInfo
    {
        const Image* targetImage = nullptr;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = 1;
        uint32_t baseArrayLayer = 0;
        uint32_t arrayLayerCount = 1;
    };

    enum class ImageViewType : uint8_t
    {
        Texture,
        RenderTarget,
        Count,
    };

    struct SamplerCreateInfo
    {
        vk::Filter minFilter = vk::Filter::eLinear;
        vk::Filter magFilter = vk::Filter::eLinear;
        vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat;
        vk::SamplerMipmapMode mipMapMode = vk::SamplerMipmapMode::eLinear;
    };

    class Image : public IntrusivePtrEnabled<Image>
    {
    public:
        static auto New(Context* pContext, const ImageCreateInfo& info) -> IntrusivePtr<Image>;

        ~Image();

        auto GetImageView(ImageViewType type) -> ImageView*;

        auto GetImage() const -> auto { return m_image; }
        auto GetWidth() const -> auto { return m_width; }
        auto GetHeight() const -> auto { return m_height; }
        auto GetDepthOrArrayLayers() const -> auto { return m_depthOrArrayLayers; }
        auto GetMipLevels() const -> auto { return m_mipLevels; }
        auto GetFormat() const -> auto { return m_format; }
        auto GetAspect() const -> vk::ImageAspectFlags;

    private:
        friend class SwapChain;

        Image(
            Context* context,
            vk::Image image,
            vma::Allocation allocation,
            uint32_t width,
            uint32_t height,
            uint32_t depthOrArrayLayers,
            uint32_t mipLevels,
            vk::Format format
        );
        Image(Context* context, vk::Image image, uint32_t width, uint32_t height, vk::Format format);

    private:
        Context* m_ctx;
        vk::Image m_image;
        vma::Allocation m_allocation;
        bool m_ownsImage;

        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_depthOrArrayLayers;
        uint32_t m_mipLevels;
        vk::Format m_format;

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
