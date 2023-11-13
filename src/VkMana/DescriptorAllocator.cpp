#include "DescriptorAllocator.hpp"

#include "Context.hpp"

namespace VkMana
{
	DescriptorAllocator::~DescriptorAllocator()
	{
		for (auto& pool : m_pools)
		{
			m_ctx->GetDevice().destroy(pool.DescriptorPool);
		}
	}

	void DescriptorAllocator::SetPoolSizeMultiplier(vk::DescriptorType type, float multiplier)
	{
		m_poolSizeMultipliers[type] = multiplier;
	}

	auto DescriptorAllocator::Allocate(vk::DescriptorSetLayout setLayout) -> vk::DescriptorSet
	{
		auto* pool = TryGetValidPool(setLayout);
		if (!pool)
		{
			uint32_t maxSets = 20;

			// Create new pool
			std::vector<vk::DescriptorPoolSize> poolSizes{};
			for (auto& [type, multiplier] : m_poolSizeMultipliers)
				poolSizes.emplace_back(type, uint32_t(multiplier * maxSets));

			vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.setMaxSets(maxSets);
			poolInfo.setPoolSizes(poolSizes);
			poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);

			auto& newPool = m_pools.emplace_back();
			newPool.Layout = setLayout;
			newPool.DescriptorPool = m_ctx->GetDevice().createDescriptorPool(poolInfo);
			newPool.MaxSets = maxSets;

			/*std::vector setLayouts(maxSets, newPool.Layout);
			vk::DescriptorSetAllocateInfo setAllocInfo{};
			setAllocInfo.setDescriptorPool(newPool.DescriptorPool);
			setAllocInfo.setSetLayouts(setLayouts);
			newPool.PreAllocatedSets = m_ctx->GetDevice().allocateDescriptorSets(setAllocInfo);*/

			pool = &newPool;
		}

		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.setDescriptorPool(pool->DescriptorPool);
		allocInfo.setSetLayouts(setLayout);
		auto set = m_ctx->GetDevice().allocateDescriptorSets(allocInfo)[0];
		return set;
	}

	void DescriptorAllocator::ResetAllocator()
	{
		for (auto& pool : m_pools)
		{
			// pool.PreAllocatedSets.clear();
			m_ctx->GetDevice().resetDescriptorPool(pool.DescriptorPool);
			// pool.SetIndex = 0;

			/*std::vector setLayouts(pool.MaxSets, pool.Layout);
			vk::DescriptorSetAllocateInfo setAllocInfo{};
			setAllocInfo.setDescriptorPool(pool.DescriptorPool);
			setAllocInfo.setSetLayouts(setLayouts);
			pool.PreAllocatedSets = m_ctx->GetDevice().allocateDescriptorSets(setAllocInfo);*/
		}
	}

	DescriptorAllocator::DescriptorAllocator(Context* context, uint32_t maxSets)
		: m_ctx(context)
		, m_maxSets(maxSets)
	{
		m_poolSizeMultipliers = {
			{ vk::DescriptorType::eSampler, 1.0f },
			{ vk::DescriptorType::eCombinedImageSampler, 4.0f },
			{ vk::DescriptorType::eSampledImage, 4.0f },
			{ vk::DescriptorType::eStorageImage, 1.0f },
			{ vk::DescriptorType::eUniformTexelBuffer, 1.0f },
			{ vk::DescriptorType::eStorageTexelBuffer, 1.0f },
			{ vk::DescriptorType::eUniformBuffer, 2.0f },
			{ vk::DescriptorType::eStorageBuffer, 2.0f },
			{ vk::DescriptorType::eUniformBufferDynamic, 1.0f },
			{ vk::DescriptorType::eStorageBufferDynamic, 1.0f },
		};
	}

	auto DescriptorAllocator::TryGetValidPool(vk::DescriptorSetLayout setLayout) -> Pool*
	{
		for (auto& pool : m_pools)
		{
			if (pool.Layout != setLayout)
				continue;

			// if (pool.SetIndex >= pool.MaxSets)
			// continue;

			return &pool;
		}
		return nullptr;
	}

} // namespace VkMana