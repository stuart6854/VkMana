#pragma once

#include "Vulkan_Common.hpp"
#include "RenderPass.hpp"
#include "Pipeline.hpp"
#include "Image.hpp"
#include "Buffer.hpp"

// #TODO: Batch pipeline barriers (image transitions)

namespace VkMana
{
	class Context;

	struct ImageTransitionInfo
	{
		const Image* TargetImage = nullptr;
		vk::ImageLayout OldLayout = vk::ImageLayout::eUndefined;
		vk::ImageLayout NewLayout = vk::ImageLayout::eUndefined;
		uint32_t BaseMipLevel = 0;
		uint32_t MipLevelCount = 1;
		uint32_t BaseArrayLayer = 0;
		uint32_t ArrayLayerCount = 1;
	};
	struct ImageBlitInfo
	{
		const Image* SrcImage = nullptr;
		vk::ImageLayout SrcLayout = vk::ImageLayout::eTransferSrcOptimal;
		vk::Offset3D SrcRectStart = { 0, 0, 0 };
		vk::Offset3D SrcRectEnd = { 0, 0, 0 };
		uint32_t SrcMipLevel = 0;
		uint32_t SrcBaseArrayLayer = 0;
		uint32_t SrcArrayLayerCount = 1;

		const Image* DstImage = nullptr;
		vk::ImageLayout DstLayout = vk::ImageLayout::eTransferDstOptimal;
		vk::Offset3D DstRectStart = { 0, 0, 0 };
		vk::Offset3D DstRectEnd = { 0, 0, 0 };
		uint32_t DstMipLevel = 0;
		uint32_t DstBaseArrayLayer = 0;
		uint32_t DstArrayLayerCount = 1;

		vk::Filter Filter = vk::Filter::eLinear;
	};

	struct BufferCopyInfo
	{
		const Buffer* SrcBuffer = nullptr;
		const Buffer* DstBuffer = nullptr;
		uint64_t Size = 0;
		uint64_t SrcOffset = 0;
		uint64_t DstOffset = 0;
	};
	struct BufferToImageCopyInfo
	{
		const Buffer* SrcBuffer = nullptr;
		const Image* DstImage = nullptr;
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
		void SetPushConstants(vk::ShaderStageFlags shaderStages, uint32_t offset, uint32_t size, const void* data);

		void BindDescriptorSets(uint32_t firstSet, const std::vector<DescriptorSet*>& sets, const std::vector<uint32_t>& dynamicOffsets);

		void BindIndexBuffer(const Buffer* buffer, uint64_t offsetBytes = 0, vk::IndexType indexType = vk::IndexType::eUint16);
		void BindVertexBuffers(uint32_t firstBinding, const std::vector<const Buffer*>& buffers, const std::vector<uint64_t>& offsets);

		void Draw(uint32_t vertexCount, uint32_t firstVertex);
		void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);

		void TransitionImage(const ImageTransitionInfo& info);
		void BlitImage(const ImageBlitInfo& info);

		void CopyBuffer(const BufferCopyInfo& info);
		void CopyBufferToImage(const BufferToImageCopyInfo& info);

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
