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
		~PipelineLayout();

		auto GetLayout() const -> auto { return m_layout; }
		auto GetHash() const -> auto { return m_hash; }

	private:
		friend class Context;

		PipelineLayout(Context* context, vk::PipelineLayout layout, size_t hash, const PipelineLayoutCreateInfo& info);

	private:
		Context* m_ctx;
		vk::PipelineLayout m_layout;
		size_t m_hash;
		PipelineLayoutCreateInfo m_info;
	};
	using PipelineLayoutHandle = IntrusivePtr<PipelineLayout>;

	class Pipeline : public IntrusivePtrEnabled<Pipeline>
	{
	public:
		~Pipeline();

		auto GetLayout() const -> auto { return m_layout; }
		auto GetPipeline() const -> auto { return m_pipeline; }
		auto GetBindPoint() const -> auto { return m_bindPoint; }

	private:
		friend class Context;

		Pipeline(Context* context, const IntrusivePtr<PipelineLayout>& layout, vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint);

	private:
		Context* m_ctx;
		IntrusivePtr<PipelineLayout> m_layout;
		vk::Pipeline m_pipeline;
		vk::PipelineBindPoint m_bindPoint;
	};
	using PipelineHandle = IntrusivePtr<Pipeline>;

} // namespace VkMana
