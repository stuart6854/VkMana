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

	DescriptorSet::DescriptorSet(Context* context, vk::DescriptorSet set)
		: m_ctx(context)
		, m_set(set)
	{
	}

} // namespace VkMana