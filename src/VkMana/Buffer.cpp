#include "Buffer.hpp"

#include "Context.hpp"

namespace VkMana
{
    Buffer::~Buffer()
    {
        if(m_buffer)
            m_ctx->DestroyBuffer(m_buffer);
        if(m_allocation)
            m_ctx->DestroyAllocation(m_allocation);
    }

    void Buffer::WriteHostAccessible(uint64_t offset, uint64_t size, const void* data) const
    {
        if(!IsHostAccessible())
            return;

        assert(offset + size <= GetSize() && "Buffer (host-accessible) write overflow.");

        auto* mapped = m_ctx->GetAllocator().mapMemory(m_allocation);
        auto* offsetMapped = static_cast<uint8_t*>(mapped) + offset;
        std::memcpy(offsetMapped, data, size);
        m_ctx->GetAllocator().unmapMemory(m_allocation);
    }

    Buffer::Buffer(Context* context, vk::Buffer buffer, vma::Allocation allocation, const BufferCreateInfo& info)
        : m_ctx(context)
        , m_buffer(buffer)
        , m_allocation(allocation)
        , m_info(info)
    {
    }

} // namespace VkMana