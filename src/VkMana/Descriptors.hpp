#pragma once

#include "Vulkan_Common.hpp"
#include "Image.hpp"

namespace VkMana
{
	class Context;

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

	private:
		friend class Context;

		DescriptorSet(Context* context, vk::DescriptorSet set);

		void Write(const ImageView* image, uint32_t binding);
		// 	void Write(const Buffer* image, uint32_t binding, uint64_t offset, uint64_t range);

	private:
		Context* m_ctx;
		vk::DescriptorSet m_set;
	};
	using DescriptorSetHandle = IntrusivePtr<DescriptorSet>;

} // namespace VkMana
