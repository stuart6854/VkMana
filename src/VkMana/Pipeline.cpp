#include "Pipeline.hpp"

#include "Context.hpp"

namespace VkMana
{
	PipelineLayout::~PipelineLayout()
	{
		if (m_layout)
			m_ctx->DestroyPipelineLayout(m_layout);
	}

	PipelineLayout::PipelineLayout(Context* context, vk::PipelineLayout layout, size_t hash, const PipelineLayoutCreateInfo& info)
		: m_ctx(context)
		, m_layout(layout)
		, m_hash(hash)
		, m_info(info)
	{
	}

	Pipeline::~Pipeline()
	{
		if (m_pipeline)
			m_ctx->DestroyPipeline(m_pipeline);
	}

	Pipeline::Pipeline(Context* context, const IntrusivePtr<PipelineLayout>& layout, vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint)
		: m_ctx(context)
		, m_layout(layout)
		, m_pipeline(pipeline)
		, m_bindPoint(bindPoint)
	{
	}

} // namespace VkMana