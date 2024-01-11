#pragma once

#include "Buffer.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "QueryPool.hpp"
#include "RenderPass.hpp"
#include "VulkanCommon.hpp"

// #TODO: Batch pipeline barriers (image transitions)

namespace VkMana
{
    class Context;

    struct DrawIndirectCmd
    {
        uint32_t vertexCount;
        uint32_t instanceCount;
        uint32_t firstVertex;
        uint32_t firstInstance;
    };
    struct DrawIndexedIndirectCmd
    {
        uint32_t indexCount;
        uint32_t instanceCount;
        uint32_t firstIndex;
        uint32_t vertexOffset;
        uint32_t firstInstance;
    };

    struct ImageTransitionInfo
    {
        const Image* pImage = nullptr;
        vk::ImageLayout oldLayout = vk::ImageLayout::eUndefined;
        vk::ImageLayout newLayout = vk::ImageLayout::eUndefined;
        uint32_t baseMipLevel = 0;
        uint32_t mipLevelCount = 1;
        uint32_t baseArrayLayer = 0;
        uint32_t arrayLayerCount = 1;
    };
    struct ImageBlitInfo
    {
        const Image* pSrcImage = nullptr;
        vk::ImageLayout srcLayout = vk::ImageLayout::eTransferSrcOptimal;
        vk::Offset3D srcRectStart = { 0, 0, 0 };
        vk::Offset3D srcRectEnd = { 0, 0, 0 };
        uint32_t srcMipLevel = 0;
        uint32_t srcBaseArrayLayer = 0;
        uint32_t srcArrayLayerCount = 1;

        const Image* pDstImage = nullptr;
        vk::ImageLayout dstLayout = vk::ImageLayout::eTransferDstOptimal;
        vk::Offset3D dstRectStart = { 0, 0, 0 };
        vk::Offset3D dstRectEnd = { 0, 0, 0 };
        uint32_t dstMipLevel = 0;
        uint32_t dstBaseArrayLayer = 0;
        uint32_t dstArrayLayerCount = 1;

        vk::Filter filter = vk::Filter::eLinear;
    };

    struct BufferCopyInfo
    {
        const Buffer* pSrcBuffer = nullptr;
        const Buffer* pDstBuffer = nullptr;
        uint64_t size = 0;
        uint64_t srcOffset = 0;
        uint64_t dstOffset = 0;
    };
    struct BufferToImageCopyInfo
    {
        const Buffer* pSrcBuffer = nullptr;
        const Image* pDstImage = nullptr;
    };
    struct QueryCopyInfo
    {
        const QueryPool* pQueryPool;
        uint32_t firstQuery;
        uint32_t queryCount;
        const Buffer* pDstBuffer;
        uint64_t dstOffset;
        uint64_t stride;
        vk::QueryResultFlags flags;
    };

    class CommandBuffer : public IntrusivePtrEnabled<CommandBuffer>
    {
    public:
        ~CommandBuffer() = default;

        /* State */

        void BeginRenderPass(const RenderPassInfo& info);
        void EndRenderPass();

        void BindPipeline(Pipeline* pPipeline);
        void SetViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);
        void SetPushConstants(vk::ShaderStageFlags shaderStages, uint32_t offset, uint32_t size, const void* data);

        void BindDescriptorSets(uint32_t firstSet, const std::vector<DescriptorSet*>& sets, const std::vector<uint32_t>& dynamicOffsets);

        void BindIndexBuffer(const Buffer* pBuffer, uint64_t offsetBytes = 0, vk::IndexType indexType = vk::IndexType::eUint16);
        void BindVertexBuffers(uint32_t firstBinding, const std::vector<const Buffer*>& buffers, const std::vector<uint64_t>& offsets);

        void Draw(uint32_t vertexCount, uint32_t firstVertex);
        void DrawIndexed(uint32_t indexCount, uint32_t firstIndex, uint32_t vertexOffset);
        void DrawIndirect(const Buffer* pBuffer, uint64_t offset, uint32_t drawCount, uint32_t stride);
        void DrawIndexedIndirect(const Buffer* pBuffer, uint64_t offset, uint32_t drawCount, uint32_t stride);

        void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

        void TransitionImage(const ImageTransitionInfo& info);
        void BlitImage(const ImageBlitInfo& info);

        void CopyBuffer(const BufferCopyInfo& info);
        void CopyBufferToImage(const BufferToImageCopyInfo& info);

        void ResetQueryPool(const QueryPool* pQueryPool, uint32_t firstQuery, uint32_t queryCount);
        void BeginQuery(const QueryPool* pQueryPool, uint32_t queryIndex, vk::QueryControlFlags flags = {});
        void EndQuery(const QueryPool* pQueryPool, uint32_t queryIndex);
        void CopyQueryResultsToBuffer(const QueryCopyInfo& info);

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
