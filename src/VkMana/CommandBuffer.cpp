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
        for(const auto& target : info.targets)
        {
            if(target.isDepthStencil)
            {
                // Depth/Stencil
                auto& attachment = depthStencilAttachment;
                attachment.setImageView(target.pImage->GetView());
                attachment.setLoadOp(target.clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare);
                attachment.setStoreOp(target.store ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare);
                attachment.setClearValue(vk::ClearDepthStencilValue(target.clearValue[0], uint32_t(target.clearValue[1])));
                attachment.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
                hasDepthStencil = true;

                ImageTransitionInfo transitionInfo{
                    .pImage = target.pImage->GetImage(),
                    .oldLayout = target.preLayout,
                    .newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                };
                TransitionImage(transitionInfo);
            }
            else
            {
                // Color
                auto& attachment = colorAttachments.emplace_back();
                attachment.setImageView(target.pImage->GetView());
                attachment.setLoadOp(target.clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare);
                attachment.setStoreOp(target.store ? vk::AttachmentStoreOp::eStore : vk::AttachmentStoreOp::eDontCare);
                attachment.setClearValue(vk::ClearColorValue(target.clearValue));
                attachment.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);

                ImageTransitionInfo transitionInfo{
                    .pImage = target.pImage->GetImage(),
                    .oldLayout = target.preLayout,
                    .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
                };
                TransitionImage(transitionInfo);
            }

            width = std::min(width, target.pImage->GetImage()->GetWidth());
            height = std::min(height, target.pImage->GetImage()->GetHeight());
        }

        vk::RenderingInfo renderingInfo{};
        renderingInfo.setRenderArea({
            {    0,      0},
            {width, height}
        });
        renderingInfo.setLayerCount(1);
        renderingInfo.setColorAttachments(colorAttachments);
        if(hasDepthStencil)
            renderingInfo.setPDepthAttachment(&depthStencilAttachment); // #TODO: Stencil Attachment.

        m_cmd.beginRendering(renderingInfo);

        m_renderPass = info;
    }

    void CommandBuffer::EndRenderPass()
    {
        m_cmd.endRendering();

        for(const auto& target : m_renderPass.targets)
        {
            if(target.isDepthStencil)
            {
                ImageTransitionInfo transitionInfo{
                    .pImage = target.pImage->GetImage(),
                    .oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                    .newLayout = target.postLayout,
                };
                TransitionImage(transitionInfo);
            }
            else
            {
                ImageTransitionInfo transitionInfo{
                    .pImage = target.pImage->GetImage(),
                    .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
                    .newLayout = target.postLayout,
                };
                TransitionImage(transitionInfo);
            }
        }
    }

    void CommandBuffer::BindPipeline(Pipeline* pPipeline)
    {
        m_cmd.bindPipeline(pPipeline->GetBindPoint(), pPipeline->GetPipeline());
        m_pipeline = pPipeline;
    }

    void CommandBuffer::SetViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
    {
        vk::Viewport viewport{ x, y, width, height, minDepth, maxDepth };
        m_cmd.setViewport(0, viewport);
    }

    void CommandBuffer::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        vk::Rect2D scissor{
            {    x,      y},
            {width, height}
        };
        m_cmd.setScissor(0, scissor);
    }

    void CommandBuffer::SetPushConstants(vk::ShaderStageFlags shaderStages, uint32_t offset, uint32_t size, const void* data)
    {
        m_cmd.pushConstants(m_pipeline->GetLayout()->GetLayout(), shaderStages, offset, size, data);
    }

    void CommandBuffer::BindDescriptorSets(uint32_t firstSet, const std::vector<DescriptorSet*>& sets, const std::vector<uint32_t>& dynamicOffsets)
    {
        const auto setCount = uint32_t(sets.size());
        assert(setCount <= 32);
        std::array<vk::DescriptorSet, 32> descSets;
        for(auto i = 0u; i < setCount; ++i)
            descSets[i] = sets[i]->GetSet();

        m_cmd.bindDescriptorSets(
            m_pipeline->GetBindPoint(),
            m_pipeline->GetLayout()->GetLayout(),
            firstSet,
            setCount,
            descSets.data(),
            uint32_t(dynamicOffsets.size()),
            dynamicOffsets.data()
        );
    }

    void CommandBuffer::BindIndexBuffer(const Buffer* pBuffer, uint64_t offsetBytes, vk::IndexType indexType)
    {
        m_cmd.bindIndexBuffer(pBuffer->GetBuffer(), offsetBytes, indexType);
    }

    void CommandBuffer::BindVertexBuffers(uint32_t firstBinding, const std::vector<const Buffer*>& buffers, const std::vector<uint64_t>& offsets)
    {
        const auto bufferCount = uint32_t(buffers.size());
        assert(bufferCount <= 8);
        std::array<vk::Buffer, 8> vtxBuffers;
        for(auto i = 0u; i < bufferCount; ++i)
            vtxBuffers[i] = buffers[i]->GetBuffer();

        assert(offsets.size() == bufferCount);
        m_cmd.bindVertexBuffers(firstBinding, bufferCount, vtxBuffers.data(), offsets.data());
    }

    void CommandBuffer::Draw(uint32_t vertexCount, uint32_t firstVertex) { m_cmd.draw(vertexCount, 1, firstVertex, 0); }

    void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset)
    {
        m_cmd.drawIndexed(indexCount, 1, firstIndex, vertexOffset, 0);
    }

    void CommandBuffer::DrawIndirect(const Buffer* pBuffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
    {
        m_cmd.drawIndirect(pBuffer->GetBuffer(), offset, drawCount, stride);
    }

    void CommandBuffer::DrawIndexedIndirect(const Buffer* pBuffer, uint64_t offset, uint32_t drawCount, uint32_t stride)
    {
        m_cmd.drawIndexedIndirect(pBuffer->GetBuffer(), offset, drawCount, stride);
    }

    void CommandBuffer::Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) { m_cmd.dispatch(groupCountX, groupCountY, groupCountZ); }

    void CommandBuffer::TransitionImage(const ImageTransitionInfo& info)
    {
        vk::PipelineStageFlags2 srcStage = {};
        vk::AccessFlags2 srcAccess = {};
        switch(info.oldLayout)
        {
        case vk::ImageLayout::eUndefined:
            srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;
            srcAccess = vk::AccessFlagBits2::eNone;
            break;
        case vk::ImageLayout::eTransferSrcOptimal:
            srcStage = vk::PipelineStageFlagBits2::eTransfer;
            srcAccess = vk::AccessFlagBits2::eTransferRead;
            break;
        case vk::ImageLayout::eTransferDstOptimal:
            srcStage = vk::PipelineStageFlagBits2::eTransfer;
            srcAccess = vk::AccessFlagBits2::eTransferWrite;
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
            assert(false);
            break;
        }

        vk::PipelineStageFlags2 dstStage = {};
        vk::AccessFlags2 dstAccess = {};
        switch(info.newLayout)
        {
        case vk::ImageLayout::eTransferSrcOptimal:
            dstStage = vk::PipelineStageFlagBits2::eTransfer;
            dstAccess = vk::AccessFlagBits2::eTransferRead;
            break;
        case vk::ImageLayout::eTransferDstOptimal:
            dstStage = vk::PipelineStageFlagBits2::eTransfer;
            dstAccess = vk::AccessFlagBits2::eTransferWrite;
            break;
        case vk::ImageLayout::eColorAttachmentOptimal:
            dstStage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            dstAccess = vk::AccessFlagBits2::eColorAttachmentWrite;
            break;
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            dstStage = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
            dstAccess = vk::AccessFlagBits2::eDepthStencilAttachmentRead;
            break;
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            dstStage = vk::PipelineStageFlagBits2::eFragmentShader;
            dstAccess = vk::AccessFlagBits2::eShaderRead;
            break;
        case vk::ImageLayout::ePresentSrcKHR:
            dstStage = vk::PipelineStageFlagBits2::eBottomOfPipe;
            dstAccess = vk::AccessFlagBits2::eNone;
            break;
        default:
            assert(false);
            break;
        }

        vk::ImageMemoryBarrier2 barrier{};
        barrier.setImage(info.pImage->GetImage());
        barrier.setOldLayout(info.oldLayout);
        barrier.setNewLayout(info.newLayout);
        barrier.setDstStageMask(srcStage);
        barrier.setDstAccessMask(srcAccess);
        barrier.setSrcStageMask(dstStage);
        barrier.setSrcAccessMask(dstAccess);
        barrier.subresourceRange.setAspectMask(info.pImage->GetAspect());
        barrier.subresourceRange.setBaseMipLevel(info.baseMipLevel);
        barrier.subresourceRange.setLevelCount(info.mipLevelCount);
        barrier.subresourceRange.setBaseArrayLayer(info.baseArrayLayer);
        barrier.subresourceRange.setLayerCount(info.arrayLayerCount);

        vk::DependencyInfo depInfo{};
        depInfo.setImageMemoryBarriers(barrier);
        m_cmd.pipelineBarrier2(depInfo);
    }

    void CommandBuffer::BlitImage(const ImageBlitInfo& info)
    {
        vk::ImageBlit region{};
        region.setSrcOffsets({ info.srcRectStart, info.srcRectEnd });
        region.srcSubresource.setAspectMask(info.pSrcImage->GetAspect());
        region.srcSubresource.setMipLevel(info.srcMipLevel);
        region.srcSubresource.setBaseArrayLayer(info.srcBaseArrayLayer);
        region.srcSubresource.setLayerCount(info.srcArrayLayerCount);
        region.setDstOffsets({ info.dstRectStart, info.dstRectEnd });
        region.dstSubresource.setAspectMask(info.pDstImage->GetAspect());
        region.dstSubresource.setMipLevel(info.dstMipLevel);
        region.dstSubresource.setBaseArrayLayer(info.dstBaseArrayLayer);
        region.dstSubresource.setLayerCount(info.dstArrayLayerCount);

        m_cmd.blitImage(info.pSrcImage->GetImage(), info.srcLayout, info.pDstImage->GetImage(), info.dstLayout, region, info.filter);
    }

    void CommandBuffer::CopyBuffer(const BufferCopyInfo& info)
    {
        vk::BufferCopy2 region{};
        region.setSize(info.size);
        region.setSrcOffset(info.srcOffset);
        region.setDstOffset(info.dstOffset);

        vk::CopyBufferInfo2 copyInfo{};
        copyInfo.setSrcBuffer(info.pSrcBuffer->GetBuffer());
        copyInfo.setDstBuffer(info.pDstBuffer->GetBuffer());
        copyInfo.setRegions(region);
        m_cmd.copyBuffer2(copyInfo);
    }

    void CommandBuffer::CopyBufferToImage(const BufferToImageCopyInfo& info)
    {
        vk::BufferImageCopy2 region{};
        region.setBufferOffset(0);
        region.setBufferRowLength(0);
        region.setBufferImageHeight(0);
        region.setImageExtent({ info.pDstImage->GetWidth(), info.pDstImage->GetHeight(), info.pDstImage->GetDepthOrArrayLayers() });
        region.imageSubresource.setAspectMask(info.pDstImage->GetAspect());
        region.imageSubresource.setMipLevel(0);
        region.imageSubresource.setBaseArrayLayer(0);
        region.imageSubresource.setLayerCount(1);

        vk::CopyBufferToImageInfo2 copyInfo{};
        copyInfo.setSrcBuffer(info.pSrcBuffer->GetBuffer());
        copyInfo.setDstImage(info.pDstImage->GetImage());
        copyInfo.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);
        copyInfo.setRegions(region);

        m_cmd.copyBufferToImage2(copyInfo);
    }

    void CommandBuffer::CopyImageToBuffer(const ImageToBufferCopyInfo& info)
    {
        vk::BufferImageCopy2 region{};
        region.setBufferOffset(0);
        region.setBufferRowLength(0);
        region.setBufferImageHeight(0);
        region.setImageExtent({ info.pSrcImage->GetWidth(), info.pSrcImage->GetHeight(), info.pSrcImage->GetDepthOrArrayLayers() });
        region.imageSubresource.setAspectMask(info.pSrcImage->GetAspect());
        region.imageSubresource.setMipLevel(0);
        region.imageSubresource.setBaseArrayLayer(0);
        region.imageSubresource.setLayerCount(1);

        vk::CopyImageToBufferInfo2 copyInfo{};
        copyInfo.setSrcImage(info.pSrcImage->GetImage());
        copyInfo.setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal);
        copyInfo.setDstBuffer(info.pDstBuffer->GetBuffer());
        copyInfo.setRegions(region);

        m_cmd.copyImageToBuffer2(copyInfo);
    }

    void CommandBuffer::ResetQueryPool(const QueryPool* pQueryPool, uint32_t firstQuery, uint32_t queryCount)
    {
        m_cmd.resetQueryPool(pQueryPool->GetPool(), firstQuery, queryCount);
    }

    void CommandBuffer::BeginQuery(const QueryPool* pQueryPool, uint32_t queryIndex, vk::QueryControlFlags flags)
    {
        m_cmd.beginQuery(pQueryPool->GetPool(), queryIndex, flags);
    }

    void CommandBuffer::EndQuery(const QueryPool* pQueryPool, uint32_t queryIndex) { m_cmd.endQuery(pQueryPool->GetPool(), queryIndex); }

    void CommandBuffer::CopyQueryResultsToBuffer(const QueryCopyInfo& info)
    {
        m_cmd.copyQueryPoolResults(
            info.pQueryPool->GetPool(),
            info.firstQuery,
            info.queryCount,
            info.pDstBuffer->GetBuffer(),
            info.dstOffset,
            info.stride,
            info.flags
        );
    }

    CommandBuffer::CommandBuffer(Context* context, vk::CommandBuffer cmd)
        : m_ctx(context)
        , m_cmd(cmd)
    {
    }

} // namespace VkMana