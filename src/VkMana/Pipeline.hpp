#pragma once

#include "Vulkan_Common.hpp"
#include "Descriptors.hpp"

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

	using ShaderBinary = std::vector<uint32_t>;
	struct ShaderInfo
	{
		ShaderBinary SPIRVBinary;
		std::string EntryPoint = "main"; // Should be "main" for GLSL.
	};

	/**
	 * Dynamic State
	 * 	- Polygon Mode (Wireframe)
	 * 	- Line Width
	 *
	 */
	struct GraphicsPipelineCreateInfo
	{
		ShaderInfo Vertex;
		ShaderInfo Fragment;

		std::vector<vk::VertexInputAttributeDescription> VertexAttributes;
		std::vector<vk::VertexInputBindingDescription> VertexBindings;

		vk::PrimitiveTopology Topology;

		std::vector<vk::Format> ColorTargetFormats;
		vk::Format DepthTargetFormat = vk::Format::eUndefined;
		vk::Format StencilTargetFormat = vk::Format::eUndefined;

		IntrusivePtr<PipelineLayout> Layout = nullptr;
	};

	struct ComputePipelineCreateInfo
	{
		ShaderInfo compute;
		IntrusivePtr<PipelineLayout> layout = nullptr;
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
