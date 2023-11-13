#pragma once

#include "Vulkan_Common.hpp"

#include <unordered_map>

namespace VkMana
{
	class Context;

	class DescriptorAllocator : public IntrusivePtrEnabled<DescriptorAllocator>
	{
	public:
		~DescriptorAllocator();

		void SetPoolSizeMultiplier(vk::DescriptorType type, float multiplier);

		auto Allocate(vk::DescriptorSetLayout setLayout) -> vk::DescriptorSet;
		void ResetAllocator();

	private:
		friend class Context;
		explicit DescriptorAllocator(Context* context, uint32_t maxSets);

		struct Pool
		{
			vk::DescriptorSetLayout Layout;
			vk::DescriptorPool DescriptorPool;
			uint32_t MaxSets;
			// std::vector<vk::DescriptorSet> PreAllocatedSets; // #TODO: Consider pre-allocating maxSets each frame. Better performance?
			// uint32_t SetIndex = 0;
		};
		auto TryGetValidPool(vk::DescriptorSetLayout setLayout) -> Pool*;

	private:
		Context* m_ctx;
		uint32_t m_maxSets;
		std::unordered_map<vk::DescriptorType, float> m_poolSizeMultipliers;

		std::vector<Pool> m_pools;
	};
	using DescriptorAllocatorHandle = IntrusivePtr<DescriptorAllocator>;

} // namespace VkMana
