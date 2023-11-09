#include "CommandBuffer.hpp"

#include "Image.hpp"

namespace VkMana
{
	void CommandBuffer::BeginRenderPass(const RenderPassInfo& info)
	{
		uint32_t width = UINT32_MAX;
		uint32_t height = UINT32_MAX;

		std::vector<vk::RenderingAttachmentInfo> colorAttachments;
		vk::RenderingAttachmentInfo depthStencilAttachment;
		bool hasDepthStencil = false;
		for (const auto& target : info.Targets)
		{
			if (target.IsDepthStencil)
			{
				// Depth/Stencil
				auto& attachment = depthStencilAttachment;
				attachment.setImageView(target.Image->GetView());
				attachment.setLoadOp(target.Clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare);
				attachment.setStoreOp(target.Store ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare);
				attachment.setClearValue(vk::ClearDepthStencilValue(target.ClearValue[0], uint32_t(target.ClearValue[1])));
				attachment.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				hasDepthStencil = true;

				ImageTransitionInfo transitionInfo{
					.Image = target.Image->GetImage(),
					.OldLayout = target.PreLayout,
					.NewLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
				};
				TransitionImage(transitionInfo);
			}
			else
			{
				// Color
				auto& attachment = colorAttachments.emplace_back();
				attachment.setImageView(target.Image->GetView());
				attachment.setLoadOp(target.Clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare);
				attachment.setStoreOp(target.Store ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare);
				attachment.setClearValue(vk::ClearColorValue(target.ClearValue));
				attachment.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);

				ImageTransitionInfo transitionInfo{
					.Image = target.Image->GetImage(),
					.OldLayout = target.PreLayout,
					.NewLayout = vk::ImageLayout::eColorAttachmentOptimal,
				};
				TransitionImage(transitionInfo);
			}

			width = std::min(width, target.Image->GetImage()->GetWidth());
			height = std::min(height, target.Image->GetImage()->GetHeight());
		}

		vk::RenderingInfo renderingInfo{};
		renderingInfo.setRenderArea({ { 0, 0 }, { width, height } });
		renderingInfo.setLayerCount(1);
		renderingInfo.setColorAttachments(colorAttachments);
		if (hasDepthStencil)
			renderingInfo.setPDepthAttachment(&depthStencilAttachment); // #TODO: Stencil Attachment.

		m_cmd.beginRendering(renderingInfo);

		m_renderPass = info;
	}

	void CommandBuffer::EndRenderPass()
	{
		m_cmd.endRendering();

		for (const auto& target : m_renderPass.Targets)
		{
			if (target.IsDepthStencil)
			{
				ImageTransitionInfo transitionInfo{
					.Image = target.Image->GetImage(),
					.OldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
					.NewLayout = target.PostLayout,
				};
				TransitionImage(transitionInfo);
			}
			else
			{
				ImageTransitionInfo transitionInfo{
					.Image = target.Image->GetImage(),
					.OldLayout = vk::ImageLayout::eColorAttachmentOptimal,
					.NewLayout = target.PostLayout,
				};
				TransitionImage(transitionInfo);
			}
		}
	}

	void CommandBuffer::BindPipeline(Pipeline* pipeline)
	{
		m_cmd.bindPipeline(pipeline->GetBindPoint(), pipeline->GetPipeline());
		m_pipeline = pipeline;
	}

	void CommandBuffer::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		vk::Viewport viewport{ x, y, width, height, minDepth, maxDepth };
		m_cmd.setViewport(0, viewport);
	}

	void CommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
	{
		vk::Rect2D scissor{ { x, y }, { width, height } };
		m_cmd.setScissor(0, scissor);
	}

	void CommandBuffer::SetPushConstants(vk::ShaderStageFlags shaderStages, uint32_t offset, uint32_t size, const void* data)
	{
		m_cmd.pushConstants(m_pipeline->GetLayout(), shaderStages, offset, size, data);
	}

	void CommandBuffer::BindIndexBuffer(const Buffer* buffer, uint64_t offsetBytes, vk::IndexType indexType)
	{
		m_cmd.bindIndexBuffer(buffer->GetBuffer(), offsetBytes, indexType);
	}

	void CommandBuffer::BindVertexBuffers(
		uint32_t firstBinding, const std::vector<const Buffer*>& buffers, const std::vector<uint64_t>& offsets)
	{
		std::vector<vk::Buffer> vtxBuffers(buffers.size());
		for (auto i = 0; i < buffers.size(); ++i)
			vtxBuffers[i] = buffers[i]->GetBuffer();

		m_cmd.bindVertexBuffers(firstBinding, vtxBuffers, offsets);
	}

	void CommandBuffer::Draw(uint32_t vertexCount, uint32_t firstVertex)
	{
		m_cmd.draw(vertexCount, 1, firstVertex, 0);
	}

	void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
	{
		m_cmd.drawIndexed(indexCount, 1, firstIndex, vertexOffset, 0);
	}

	void CommandBuffer::TransitionImage(const ImageTransitionInfo& info)
	{
		vk::PipelineStageFlags2 srcStage = {};
		vk::AccessFlags2 srcAccess = {};
		switch (info.OldLayout)
		{
			case vk::ImageLayout::eUndefined:
				srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
				srcAccess = vk::AccessFlagBits2::eNone;
				break;
			case vk::ImageLayout::eColorAttachmentOptimal:
				srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
				srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
				break;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				srcStage = vk::PipelineStageFlagBits2::eLateFragmentTests;
				srcAccess = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
				break;
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				srcStage = vk::PipelineStageFlagBits2::eFragmentShader;
				srcAccess = vk::AccessFlagBits2::eShaderRead;
				break;
			default:
				break;
		}

		vk::PipelineStageFlags2 dstStage = {};
		vk::AccessFlags2 dstAccess = {};
		switch (info.NewLayout)
		{
			case vk::ImageLayout::eColorAttachmentOptimal:
				srcStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
				srcAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
				break;
			case vk::ImageLayout::eDepthStencilAttachmentOptimal:
				srcStage = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
				srcAccess = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
				break;
			case vk::ImageLayout::eShaderReadOnlyOptimal:
				srcStage = vk::PipelineStageFlagBits2::eFragmentShader;
				srcAccess = vk::AccessFlagBits2::eShaderRead;
				break;
			case vk::ImageLayout::ePresentSrcKHR:
				srcStage = vk::PipelineStageFlagBits2::eBottomOfPipe;
				srcAccess = vk::AccessFlagBits2::eNone;
				break;
			default:
				break;
		}

		vk::ImageMemoryBarrier2 barrier{};
		barrier.setImage(info.Image->GetImage());
		barrier.setOldLayout(info.OldLayout);
		barrier.setNewLayout(info.NewLayout);
		barrier.setDstStageMask(srcStage);
		barrier.setDstAccessMask(srcAccess);
		barrier.setSrcStageMask(dstStage);
		barrier.setSrcAccessMask(dstAccess);
		barrier.subresourceRange.setAspectMask(info.Image->GetAspect());
		barrier.subresourceRange.setBaseMipLevel(info.BaseMipLevel);
		barrier.subresourceRange.setLevelCount(info.MipLevelCount);
		barrier.subresourceRange.setBaseArrayLayer(info.BaseArrayLayer);
		barrier.subresourceRange.setLayerCount(info.ArrayLayerCount);

		vk::DependencyInfo depInfo{};
		depInfo.setImageMemoryBarriers(barrier);
		m_cmd.pipelineBarrier2(depInfo);
	}

	void CommandBuffer::CopyBuffer(const BufferCopyInfo& info)
	{
		vk::BufferCopy2 region{};
		region.setSize(info.Size);
		region.setSrcOffset(info.SrcOffset);
		region.setDstOffset(info.DstOffset);

		vk::CopyBufferInfo2 copyInfo{};
		copyInfo.setSrcBuffer(info.SrcBuffer->GetBuffer());
		copyInfo.setDstBuffer(info.DstBuffer->GetBuffer());
		copyInfo.setRegions(region);
		m_cmd.copyBuffer2(copyInfo);
	}

	CommandBuffer::CommandBuffer(Context* context, vk::CommandBuffer cmd)
		: m_ctx(context)
		, m_cmd(cmd)
	{
	}

} // namespace VkMana