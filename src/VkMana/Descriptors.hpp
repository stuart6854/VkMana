#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
#include "VulkanCommon.hpp"

namespace VkMana
{
    class Context;

    struct SetLayoutBinding
    {
        uint32_t binding = 0;
        vk::DescriptorType type;
        uint32_t count = 1;
        vk::ShaderStageFlags stageFlags;
        vk::DescriptorBindingFlags bindingFlags;
    };

    class SetLayout : public IntrusivePtrEnabled<SetLayout>
    {
    public:
        ~SetLayout();

        auto GetLayout() const -> auto { return m_layout; }
        auto GetHash() const -> auto { return m_hash; }

    private:
        friend class Context;

        SetLayout(Context* context, vk::DescriptorSetLayout layout, size_t hash);

    private:
        Context* m_ctx;
        vk::DescriptorSetLayout m_layout;
        size_t m_hash;
    };
    using SetLayoutHandle = IntrusivePtr<SetLayout>;

    class DescriptorSet : public IntrusivePtrEnabled<DescriptorSet>
    {
    public:
        ~DescriptorSet() = default;

        void Write(const ImageView* pImage, const Sampler* pSampler, uint32_t binding);
        void Write(uint32_t binding, const Buffer* pBuffer, uint64_t offset, uint64_t range, vk::DescriptorType descriptorType);

        void WriteArray(uint32_t binding, uint32_t arrayOffset, const std::vector<const ImageView*>& images, const Sampler* sampler);

        auto GetSet() const -> auto { return m_set; }

    private:
        friend class Context;

        DescriptorSet(Context* context, vk::DescriptorSet set);

    private:
        Context* m_ctx;
        vk::DescriptorSet m_set;
    };
    using DescriptorSetHandle = IntrusivePtr<DescriptorSet>;

} // namespace VkMana
