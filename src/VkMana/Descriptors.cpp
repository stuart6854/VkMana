#include "Descriptors.hpp"

#include "Context.hpp"

namespace VkMana
{
    SetLayout::~SetLayout()
    {
        if(m_layout)
            m_ctx->DestroySetLayout(m_layout);
    }

    SetLayout::SetLayout(Context* context, vk::DescriptorSetLayout layout, size_t hash)
        : m_ctx(context)
        , m_layout(layout)
        , m_hash(hash)
    {
    }

    void DescriptorSet::Write(const ImageView* pImage, const Sampler* pSampler, uint32_t binding)
    {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.setImageView(pImage->GetView());
        imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        imageInfo.setSampler(pSampler->GetSampler());

        vk::WriteDescriptorSet write{};
        write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        write.setDstSet(m_set);
        write.setDstBinding(binding);
        write.setDescriptorCount(1);
        write.setImageInfo(imageInfo);
        m_ctx->GetDevice().updateDescriptorSets(write, {});
    }

    void DescriptorSet::Write(uint32_t binding, const Buffer* pBuffer, uint64_t offset, uint64_t range, vk::DescriptorType descriptorType)
    {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.setBuffer(pBuffer->GetBuffer());
        bufferInfo.setOffset(offset);
        bufferInfo.setRange(range);

        vk::WriteDescriptorSet write{};
        write.setDescriptorType(descriptorType);
        write.setDstSet(m_set);
        write.setDstBinding(binding);
        write.setDescriptorCount(1);
        write.setBufferInfo(bufferInfo);
        m_ctx->GetDevice().updateDescriptorSets(write, {});
    }

    void DescriptorSet::WriteArray(uint32_t binding, uint32_t arrayOffset, const std::vector<const ImageView*>& images, const Sampler* sampler)
    {
        std::vector<vk::DescriptorImageInfo> imageInfos(images.size());
        std::vector<vk::WriteDescriptorSet> writes(images.size());

        for(auto i = 0u; i < images.size(); ++i)
        {
            auto& image = images[i];

            auto& imageInfo = imageInfos[i];
            imageInfo.setImageView(image->GetView());
            imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            imageInfo.setSampler(sampler->GetSampler());

            auto& write = writes[i];
            write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
            write.setDstSet(m_set);
            write.setDstBinding(binding);
            write.setDstArrayElement(arrayOffset + i);
            write.setDescriptorCount(1);
            write.setImageInfo(imageInfo);
        }

        m_ctx->GetDevice().updateDescriptorSets(writes, {});
    }

    DescriptorSet::DescriptorSet(Context* context, vk::DescriptorSet set)
        : m_ctx(context)
        , m_set(set)
    {
    }

} // namespace VkMana