#include "Garbage.hpp"

#include "Context.hpp"

namespace VkMana
{
	Garbage::~Garbage()
	{
		EmptyBins();
	}

	void Garbage::Bin(vk::Semaphore semaphore)
	{
		m_semaphores.push_back(semaphore);
	}

	void Garbage::Bin(vk::Fence fence)
	{
		m_fences.push_back(fence);
	}

	void Garbage::Bin(vk::DescriptorSetLayout layout)
	{
		m_setLayouts.push_back(layout);
	}

	void Garbage::Bin(vk::PipelineLayout layout)
	{
		m_pipelineLayouts.push_back(layout);
	}

	void Garbage::Bin(vk::Pipeline pipeline)
	{
		m_pipelines.push_back(pipeline);
	}

	void Garbage::Bin(vk::Image image)
	{
		m_images.push_back(image);
	}

	void Garbage::Bin(vk::ImageView view)
	{
		m_imageViews.push_back(view);
	}

	void Garbage::Bin(vk::Sampler sampler)
	{
		m_samplers.push_back(sampler);
	}

	void Garbage::Bin(vk::Buffer buffer)
	{
		m_buffers.push_back(buffer);
	}

	void Garbage::Bin(vma::Allocation alloc)
	{
		m_allocs.push_back(alloc);
	}

	void Garbage::EmptyBins()
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
	}

	Garbage::Garbage(Context* context)
		: m_ctx(context)
	{
	}

} // namespace VkMana