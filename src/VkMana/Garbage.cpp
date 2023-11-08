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

	void Garbage::EmptyBins()
	{
		for (auto& v : m_semaphores)
			m_ctx->GetDevice().destroy(v);
		for (auto& v : m_fences)
			m_ctx->GetDevice().destroy(v);

		m_semaphores.clear();
		m_fences.clear();
	}

	Garbage::Garbage(Context* context)
		: m_ctx(context)
	{
	}

} // namespace VkMana