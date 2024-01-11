#include "QueryPool.hpp"

#include "Context.hpp"

namespace VkMana
{
    QueryPool::~QueryPool() { m_ctx->GetDevice().destroy(m_pool); }

    void QueryPool::ResetQueries(uint32_t firstQuery, uint32_t queryCount) const { m_ctx->GetDevice().resetQueryPool(m_pool, firstQuery, queryCount); }

    bool QueryPool::GetResults32(std::vector<uint32_t>& outResults, uint32_t firstQuery, uint32_t queryCount, vk::QueryResultFlags resultFlags) const
    {
        assert(!(resultFlags & vk::QueryResultFlagBits::e64));

        auto bufferSize = queryCount;
        auto stride = sizeof(uint32_t);
        if(resultFlags & vk::QueryResultFlagBits::eWithAvailability)
        {
            bufferSize += queryCount;
            stride += sizeof(uint32_t);
        }
        if(resultFlags & vk::QueryResultFlagBits::eWithStatusKHR)
        {
            bufferSize += 1;
        }
        outResults.resize(bufferSize);

        const auto result
            = m_ctx->GetDevice().getQueryPoolResults(m_pool, firstQuery, queryCount, bufferSize * sizeof(uint32_t), outResults.data(), stride, resultFlags);
        return result == vk::Result::eSuccess;
    }

    bool QueryPool::GetResults64(std::vector<uint64_t>& outResults, uint32_t firstQuery, uint32_t queryCount, vk::QueryResultFlags resultFlags) const
    {
        resultFlags |= vk::QueryResultFlagBits::e64;

        auto bufferSize = queryCount;
        auto stride = sizeof(uint64_t);
        if(resultFlags & vk::QueryResultFlagBits::eWithAvailability)
        {
            bufferSize += queryCount;
            stride += sizeof(uint64_t);
        }
        if(resultFlags & vk::QueryResultFlagBits::eWithStatusKHR)
        {
            bufferSize += 1;
        }
        outResults.resize(bufferSize);

        const auto result
            = m_ctx->GetDevice().getQueryPoolResults(m_pool, firstQuery, queryCount, bufferSize * sizeof(uint64_t), outResults.data(), stride, resultFlags);
        return result == vk::Result::eSuccess;
    }

    QueryPool::QueryPool(Context* context, vk::QueryPool pool, uint32_t queryCount)
        : m_ctx(context)
        , m_pool(pool)
        , m_queryCount(queryCount)
    {
    }

} // namespace VkMana