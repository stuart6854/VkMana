#include "Buffer.hpp"

#include "Context.hpp"

namespace VkMana
{
    auto Buffer::New(Context* pContext, const BufferCreateInfo& info) -> BufferHandle
    {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.setSize(info.size);
        bufferInfo.setUsage(info.usage);

        vma::AllocationCreateInfo allocInfo{};
        allocInfo.setUsage(info.memUsage);
        allocInfo.setFlags(info.allocFlags);

        auto [buffer, allocation] = pContext->GetAllocator().createBuffer(bufferInfo, allocInfo);
        if(!buffer || !allocation)
        {
            VM_ERR("Failed to create Buffer");
            return nullptr;
        }

        auto pBuffer = IntrusivePtr(new Buffer(pContext, buffer, allocation, info));
        return pBuffer;
    }

    Buffer::~Buffer()
    {
        if(m_buffer)
            GetContext()->DestroyBuffer(m_buffer);
        if(m_allocation)
            GetContext()->DestroyAllocation(m_allocation);
    }

    void Buffer::SetDebugName(const std::string& name)
    {
        std::string debugName = "[Buffer] " + name;
        SetObjectDebugName(GetContext()->GetDevice(), m_buffer, debugName.c_str());
    }

    void Buffer::WriteHostAccessible(uint64_t offset, uint64_t size, const void* pData) const
    {
        if(!IsHostAccessible())
            return;

        assert(offset + size <= GetSize() && "Buffer (host-accessible) write overflow.");

        auto* mapped = GetContext()->GetAllocator().mapMemory(m_allocation);
        auto* offsetMapped = static_cast<uint8_t*>(mapped) + offset;
        std::memcpy(offsetMapped, pData, size);
        GetContext()->GetAllocator().unmapMemory(m_allocation);
    }

    Buffer::Buffer(Context* context, vk::Buffer buffer, vma::Allocation allocation, const BufferCreateInfo& info)
        : GPUResource<Buffer>(context)
        , m_buffer(buffer)
        , m_allocation(allocation)
        , m_info(info)
    {
    }

} // namespace VkMana