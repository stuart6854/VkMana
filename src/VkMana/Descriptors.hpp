#pragma once

#include "Vulkan_Common.hpp"
#include "Image.hpp"
#include "Buffer.hpp"

namespace VkMana
{
	class Context;

	struct SetLayoutBinding
	{
		uint32_t Binding = 0;
		vk::DescriptorType Type;
		uint32_t Count = 1;
		vk::ShaderStageFlags StageFlags;
		vk::DescriptorBindingFlags BindingFlags;
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

		void Write(const ImageView* image, const Sampler* sampler, uint32_t binding);
		void Write(const Buffer* buffer, uint32_t binding, vk::DescriptorType descriptorType, uint64_t offset, uint64_t range);

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
