#include "QueryPool.hpp"

#include "Context.hpp"

namespace VkMana
{
	QueryPool::~QueryPool()
	{
		m_ctx->GetDevice().destroy(m_pool);
	}

	void QueryPool::ResetQueries(uint32_t firstQuery, uint32_t queryCount) const
	{
		m_ctx->GetDevice().resetQueryPool(m_pool, firstQuery, queryCount);
	}

	void QueryPool::GetQueryResults(
		uint32_t firstQuery, uint32_t queryCount, uint64_t dataSize, void* data, uint64_t stride, vk::QueryResultFlags flags)
	{
		auto result = m_ctx->GetDevice().getQueryPoolResults(m_pool, firstQuery, queryCount, dataSize, data, stride, flags);
		UNUSED(result);
	}

	QueryPool::QueryPool(Context* context, vk::QueryPool pool)
		: m_ctx(context)
		, m_pool(pool)
	{
	}

} // namespace VkMana