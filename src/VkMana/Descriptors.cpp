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

	void DescriptorSet::Write(const ImageView* image, uint32_t binding)
	{
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.setImageView(image->GetView());
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
		imageInfo.setSampler({});

		vk::WriteDescriptorSet write{};
		write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		write.setDstSet(m_set);
		write.setDstBinding(binding);
		write.setDescriptorCount(1);
		write.setImageInfo(imageInfo);
		m_ctx->GetDevice().updateDescriptorSets(write, {});
	}

	DescriptorSet::DescriptorSet(Context* context, vk::DescriptorSet set)
		: m_ctx(context)
		, m_set(set)
	{
	}

} // namespace VkMana