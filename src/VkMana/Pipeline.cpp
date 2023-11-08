#include "Pipeline.hpp"

#include "Context.hpp"

namespace VkMana
{
	Pipeline::~Pipeline()
	{
		if (m_pipeline)
			m_ctx->DestroyPipeline(m_pipeline);
	}

	Pipeline::Pipeline(Context* context, vk::PipelineLayout layout, vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint)
		: m_ctx(context)
		, m_layout(layout)
		, m_pipeline(pipeline)
		, m_bindPoint(bindPoint)
	{
	}

} // namespace VkMana