#include "Buffer.hpp"

#include "Context.hpp"

namespace VkMana
{
	Buffer::~Buffer()
	{
		if (m_buffer)
			m_ctx->DestroyBuffer(m_buffer);
		if (m_allocation)
			m_ctx->DestroyAllocation(m_allocation);
	}

	Buffer::Buffer(Context* context, vk::Buffer buffer, vma::Allocation allocation, const BufferCreateInfo& info)
		: m_ctx(context)
		, m_buffer(buffer)
		, m_allocation(allocation)
		, m_info(info)
	{
	}

} // namespace VkMana