#pragma once

#include "VulkanCommon.hpp"

namespace VkMana
{
    class Context;

    struct BufferCreateInfo
    {
        uint64_t size = 0;
        vk::BufferUsageFlags usage;
        vma::MemoryUsage memUsage;
        vma::AllocationCreateFlags allocFlags;

        static auto Staging(uint64_t size)
        {
            return BufferCreateInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .memUsage = vma::MemoryUsage::eAuto,
                .allocFlags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            };
        }
        static auto Vertex(uint64_t size)
        {
            return BufferCreateInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                .memUsage = vma::MemoryUsage::eAutoPreferDevice,
            };
        }
        static auto Index(uint64_t size)
        {
            return BufferCreateInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                .memUsage = vma::MemoryUsage::eAutoPreferDevice,
            };
        }
        static auto Uniform(uint64_t size)
        {
            return BufferCreateInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eUniformBuffer,
                .memUsage = vma::MemoryUsage::eAutoPreferDevice,
                .allocFlags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
            };
        }
        static auto Storage(uint64_t size)
        {
            return BufferCreateInfo{
                .size = size,
                .usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                .memUsage = vma::MemoryUsage::eAutoPreferDevice,
            };
        }
    };

    struct BufferDataSource
    {
        uint64_t size = 0;
        const void* pData = nullptr;
    };

    class Buffer;
    using BufferHandle = IntrusivePtr<Buffer>;

    class Buffer : public GPUResource<Buffer>
    {
    public:
        static auto New(Context* pContext, const BufferCreateInfo& info) -> BufferHandle;

        ~Buffer();

        void SetDebugName(const std::string& name) override;

        void WriteHostAccessible(uint64_t offset, uint64_t size, const void* pData) const;

        auto GetBuffer() const -> auto { return m_buffer; }
        auto GetSize() const -> auto { return m_info.size; }
        auto GetUsage() const -> auto { return m_info.usage; }
        auto IsHostAccessible() const -> auto { return m_info.allocFlags & vma::AllocationCreateFlagBits::eHostAccessSequentialWrite; }

    private:
        Buffer(Context* context, vk::Buffer buffer, vma::Allocation allocation, const BufferCreateInfo& info);

    private:
        vk::Buffer m_buffer;
        vma::Allocation m_allocation;
        BufferCreateInfo m_info;
    };

} // namespace VkMana
