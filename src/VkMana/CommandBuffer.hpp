#pragma once

#include "Vulkan_Common.hpp"
#include "RenderPass.hpp"

// #TODO: Batch pipeline barriers (image transitions)

namespace VkMana
{
	class Context;

	struct ImageTransitionInfo
	{
		const Image* Image = nullptr;
		vk::ImageLayout OldLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout NewLayout = vk::ImageLayout::eUndefined;
		uint32_t BaseMipLevel = 0;
		uint32_t MipLevelCount = 1;
		uint32_t BaseArrayLayer = 0;
		uint32_t ArrayLayerCount = 1;
	};

	class CommandBuffer : public IntrusivePtrEnabled<CommandBuffer>
	{
	public:
		~CommandBuffer() = default;

		/* State */

		void BeginRenderPass(const RenderPassInfo& info);
		void EndRenderPass();

		void TransitionImage(const ImageTransitionInfo& info);

		/* Getters */

		auto GetCmd() const -> auto { return m_cmd; }

	private:
		friend class Context;

		CommandBuffer(Context* context, vk::CommandBuffer cmd);

	private:
		Context* m_ctx;
		vk::CommandBuffer m_cmd;

		/* State */

		RenderPassInfo m_renderPass;
	};
	using CmdBuffer = IntrusivePtr<CommandBuffer>;

} // namespace VkMana
