#pragma once

#include "Vulkan_Common.hpp"

namespace VkMana
{
	class Context;

	struct QueryPoolCreateInfo
	{
		vk::QueryType queryType;
		uint32_t queryCount;
		vk::QueryPipelineStatisticFlags pipelineStatistics;
	};

	class QueryPool : public IntrusivePtrEnabled<QueryPool>
	{
	public:
		~QueryPool();

		void ResetQueries(uint32_t firstQuery, uint32_t queryCount) const;
		void GetQueryResults(
			uint32_t firstQuery, uint32_t queryCount, uint64_t dataSize, void* data, uint64_t stride, vk::QueryResultFlags flags = {});

		auto GetPool() const -> auto { return m_pool; }

	private:
		friend class Context;

		QueryPool(Context* context, vk::QueryPool pool);

	private:
		Context* m_ctx;
		vk::QueryPool m_pool;
	};
	using QueryPoolHandle = IntrusivePtr<QueryPool>;

} // namespace VkMana
