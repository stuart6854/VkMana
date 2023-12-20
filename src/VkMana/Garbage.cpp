#include "Garbage.hpp"

#include "Context.hpp"

namespace VkMana
{
	GarbageBin::~GarbageBin()
	{
		EmptyBins();
	}

	void GarbageBin::Bin(vk::Semaphore semaphore)
	{
		m_semaphores.push_back(semaphore);
	}

	void GarbageBin::Bin(vk::Fence fence)
	{
		m_fences.push_back(fence);
	}

	void GarbageBin::Bin(vk::DescriptorSetLayout layout)
	{
		m_setLayouts.push_back(layout);
	}

	void GarbageBin::Bin(vk::PipelineLayout layout)
	{
		m_pipelineLayouts.push_back(layout);
	}

	void GarbageBin::Bin(vk::Pipeline pipeline)
	{
		m_pipelines.push_back(pipeline);
	}

	void GarbageBin::Bin(vk::Image image)
	{
		m_images.push_back(image);
	}

	void GarbageBin::Bin(vk::ImageView view)
	{
		m_imageViews.push_back(view);
	}

	void GarbageBin::Bin(vk::Sampler sampler)
	{
		m_samplers.push_back(sampler);
	}

	void GarbageBin::Bin(vk::Buffer buffer)
	{
		m_buffers.push_back(buffer);
	}

	void GarbageBin::Bin(vma::Allocation alloc)
	{
		m_allocs.push_back(alloc);
	}

	void GarbageBin::Bin(vk::QueryPool pool)
	{
		m_queryPools.push_back(pool);
	}

	void GarbageBin::EmptyBins()
	{
		for (auto& v : m_semaphores)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_fences)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_setLayouts)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_pipelineLayouts)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_pipelines)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_samplers)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_imageViews)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_images)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_buffers)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_allocs)
			m_ctx->GetAllocator().freeMemory(v);
		for (auto& v : m_queryPools)
			m_ctx->GetDevice().destroy(v);

		m_semaphores.clear();
		m_fences.clear();
		m_setLayouts.clear();
		m_pipelineLayouts.clear();
		m_pipelines.clear();
		m_samplers.clear();
		m_imageViews.clear();
		m_images.clear();
		m_buffers.clear();
		m_allocs.clear();
		m_queryPools.clear();
	}

	GarbageBin::GarbageBin(Context* context)
		: m_ctx(context)
	{
	}

} // namespace VkMana