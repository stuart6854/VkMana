#pragma once

#include "Vulkan_Common.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"

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

		void BindPipeline(Pipeline* pipeline);
		void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
		void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

		void Draw(uint32_t vertexCount, uint32_t firstVertex);

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
		Pipeline* m_pipeline;
	};
	using CmdBuffer = IntrusivePtr<CommandBuffer>;

} // namespace VkMana
