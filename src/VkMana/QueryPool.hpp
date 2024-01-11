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

    class QueryPool;
    using QueryPoolHandle = IntrusivePtr<QueryPool>;

    class QueryPool : public IntrusivePtrEnabled<QueryPool>
    {
    public:
        static auto New(Context* pContext, const QueryPoolCreateInfo& info) -> QueryPoolHandle;

        ~QueryPool();

        void ResetQueries(uint32_t firstQuery, uint32_t queryCount) const;

        bool GetResults32(std::vector<uint32_t>& outResults, uint32_t firstQuery, uint32_t queryCount, vk::QueryResultFlags resultFlags = {}) const;
        bool GetResults64(std::vector<uint64_t>& outResults, uint32_t firstQuery, uint32_t queryCount, vk::QueryResultFlags resultFlags = {}) const;

        auto GetPool() const -> auto { return m_pool; }
        auto GetQueryCount() const -> auto { return m_queryCount; }

    private:
        QueryPool(Context* pContext, vk::QueryPool pool, uint32_t queryCount);

    private:
        Context* m_ctx;
        vk::QueryPool m_pool;
        uint32_t m_queryCount;
    };

} // namespace VkMana
