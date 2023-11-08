#pragma once

#include "Vulkan_Common.hpp"

namespace VkMana
{
	class Context;

	/**
	 * Dynamic State
	 * 	- Polygon Mode (Wireframe)
	 * 	- Line Width
	 *
	 */
	struct GraphicsPipelineCreateInfo
	{
		vk::PrimitiveTopology Topology;

		std::vector<vk::Format> ColorTargetFormats;
		vk::Format DepthTargetFormat = vk::Format::eUndefined;
		vk::Format StencilTargetFormat = vk::Format::eUndefined;


	};

	class Pipeline : public IntrusivePtrEnabled<Pipeline>
	{
	public:
		~Pipeline();

		auto GetLayout() const -> auto { return m_layout; }
		auto GetPipeline() const -> auto { return m_pipeline; }
		auto GetBindPoint() const -> auto { return m_bindPoint; }

	private:
		friend class Context;

		Pipeline(Context* context, vk::PipelineLayout layout, vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint);

	private:
		Context* m_ctx;
		vk::PipelineLayout m_layout;
		vk::Pipeline m_pipeline;
		vk::PipelineBindPoint m_bindPoint;
	};
	using PipelineHandle = IntrusivePtr<Pipeline>;

} // namespace VkMana
