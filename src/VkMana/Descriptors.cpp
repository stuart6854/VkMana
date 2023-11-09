#include "Descriptors.hpp"

#include "Context.hpp"

namespace VkMana
{
	SetLayout::~SetLayout()
	{
		if (m_layout)
			m_ctx->DestroySetLayout(m_layout);
	}

	SetLayout::SetLayout(Context* context, vk::DescriptorSetLayout layout, size_t hash)
		: m_ctx(context)
		, m_layout(layout)
		, m_hash(hash)
	{
	}

	void DescriptorSet::Write(const ImageView* image, const Sampler* sampler, uint32_t binding)
	{
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.setImageView(image->GetView());
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		imageInfo.setSampler(sampler->GetSampler());

		vk::WriteDescriptorSet write{};
		write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		write.setDstSet(m_set);
		write.setDstBinding(binding);
		write.setDescriptorCount(1);
		write.setImageInfo(imageInfo);
		m_ctx->GetDevice().updateDescriptorSets(write, {});
	}

	void DescriptorSet::Write(const Buffer* buffer, uint32_t binding, vk::DescriptorType descriptorType, uint64_t offset, uint64_t range)
	{
		vk::DescriptorBufferInfo bufferInfo{};
		bufferInfo.setBuffer(buffer->GetBuffer());
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

	DescriptorSet::DescriptorSet(Context* context, vk::DescriptorSet set)
		: m_ctx(context)
		, m_set(set)
	{
	}

} // namespace VkMana