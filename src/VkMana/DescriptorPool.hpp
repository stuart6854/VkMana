#pragma once

#include "Vulkan_Common.hpp"

namespace VkMana
{
	class Context;

	class DescriptorPool : public IntrusivePtrEnabled<DescriptorPool>
	{
	public:
		~DescriptorPool() = default;

		auto AcquireDescriptorSet() -> vk::DescriptorSet;
		void ResetPool();

	private:
		friend class Context;
		DescriptorPool(Context* context, vk::DescriptorPool pool, vk::DescriptorSetLayout setLayout);

	private:
		Context* m_ctx;
		vk::DescriptorPool m_pool;
		vk::DescriptorSetLayout m_setLayout;
		std::vector<vk::DescriptorSet> m_sets;
		uint32_t m_index;
	};
	using DescriptorPoolHandle = IntrusivePtr<DescriptorPool>;

} // namespace VkMana
