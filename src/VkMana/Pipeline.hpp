#pragma once

#include "Descriptors.hpp"
#include "Vulkan_Common.hpp"

#include <vector>

namespace VkMana
{
    class Context;

    struct PipelineLayoutCreateInfo
    {
        vk::PushConstantRange PushConstantRange;
        std::vector<SetLayout*> SetLayouts;
    };

    class PipelineLayout;

    using ShaderByteCode = std::vector<uint8_t>;
    struct ShaderByteCodeInfo
    {
        const void* pByteCode = nullptr;
        uint32_t sizeBytes = 0;
    };
    struct ShaderInfo
    {
        ShaderByteCodeInfo byteCode = {};
        const char* entryPoint = "main"; // Should be "main" for GLSL.
    };

    /**
     * Dynamic State
     * 	- Polygon Mode (Wireframe)
     * 	- Line Width
     *
     */
    struct GraphicsPipelineCreateInfo
    {
        ShaderInfo vs;
        ShaderInfo fs;

        std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
        std::vector<vk::VertexInputBindingDescription> vertexBindings;

        vk::PrimitiveTopology primitiveTopology;

        uint32_t colorTargetCount = 0;
        std::array<vk::Format, 8> colorFormats;                 // Render Targets - Color
        vk::Format depthStencilFormat = vk::Format::eUndefined; // Render Target - Depth/Stencil

        IntrusivePtr<PipelineLayout> pPipelineLayout = nullptr;
    };

    struct ComputePipelineCreateInfo
    {
        ShaderInfo cs;
        IntrusivePtr<PipelineLayout> pPipelineLayout = nullptr;
    };

    class PipelineLayout : public IntrusivePtrEnabled<PipelineLayout>
    {
    public:
        static auto New(Context* pContext, const PipelineLayoutCreateInfo& info) -> IntrusivePtr<PipelineLayout>;

        ~PipelineLayout();

        auto GetLayout() const -> auto { return m_layout; }
        auto GetHash() const -> auto { return m_hash; }

    private:
        PipelineLayout(Context* context, vk::PipelineLayout layout, size_t hash);

    private:
        Context* m_ctx;
        vk::PipelineLayout m_layout;
        size_t m_hash;
    };
    using PipelineLayoutHandle = IntrusivePtr<PipelineLayout>;

    class Pipeline : public IntrusivePtrEnabled<Pipeline>
    {
    public:
        static auto NewGraphics(Context* pContext, const GraphicsPipelineCreateInfo& info) -> IntrusivePtr<Pipeline>;
        static auto NewCompute(Context* pContext, const ComputePipelineCreateInfo& info) -> IntrusivePtr<Pipeline>;

        ~Pipeline();

        auto GetLayout() const -> auto { return m_layout; }
        auto GetPipeline() const -> auto { return m_pipeline; }
        auto GetBindPoint() const -> auto { return m_bindPoint; }

    private:
        Pipeline(Context* pContext, const IntrusivePtr<PipelineLayout>& layout, vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint);

    private:
        Context* m_ctx;
        IntrusivePtr<PipelineLayout> m_layout;
        vk::Pipeline m_pipeline;
        vk::PipelineBindPoint m_bindPoint;
    };
    using PipelineHandle = IntrusivePtr<Pipeline>;

} // namespace VkMana
