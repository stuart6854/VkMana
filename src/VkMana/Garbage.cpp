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

	void Garbage::Bin(vk::Image image)
	{
		m_images.push_back(image);
	}

	void Garbage::Bin(vk::ImageView view)
	{
		m_imageViews.push_back(view);
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
		for (auto& v : m_imageViews)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_images)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_allocs)
			m_ctx->GetAllocator().freeMemory(v);

		m_semaphores.clear();
		m_fences.clear();
		m_imageViews.clear();
		m_images.clear();
		m_allocs.clear();
	}

	Garbage::Garbage(Context* context)
		: m_ctx(context)
	{
	}

} // namespace VkMana