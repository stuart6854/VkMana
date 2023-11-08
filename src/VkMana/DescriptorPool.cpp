#include "DescriptorPool.hpp"

#include "Context.hpp"

namespace VkMana
{
	auto DescriptorPool::AcquireDescriptorSet() -> vk::DescriptorSet
	{
		if (m_index < m_sets.size())
		{
			auto set = m_sets[m_index++];
			return set;
		}

		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(m_pool);
		allocInfo.setSetLayouts(m_setLayout);
		allocInfo.setDescriptorSetCount(1);
		auto set = m_ctx->GetDevice().allocateDescriptorSets(allocInfo)[0];
		m_sets.push_back(set);
		m_index++;
		return set;
	}

	void DescriptorPool::ResetPool()
	{
		m_index = 0;
	}

	DescriptorPool::DescriptorPool(Context* context, vk::DescriptorPool pool, vk::DescriptorSetLayout setLayout)
		: m_ctx(context)
		, m_pool(pool)
		, m_setLayout(setLayout)
		, m_index(0)
	{
	}

} // namespace VkMana