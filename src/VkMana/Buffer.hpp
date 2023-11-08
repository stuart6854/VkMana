#pragma once

#include "Vulkan_Common.hpp"

namespace VkMana
{
	class Context;

	struct BufferCreateInfo
	{
		uint64_t Size = 0;
		vk::BufferUsageFlags Usage;
		vma::MemoryUsage MemUsage;
		vma::AllocationCreateFlags AllocFlags;

		static auto Staging(uint64_t size)
		{
			return BufferCreateInfo{
				.Size = size,
				.Usage = vk::BufferUsageFlagBits::eTransferSrc,
				.MemUsage = vma::MemoryUsage::eAuto,
				.AllocFlags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
			};
		}
		static auto Vertex(uint64_t size)
		{
			return BufferCreateInfo{
				.Size = size,
				.Usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				.MemUsage = vma::MemoryUsage::eAutoPreferDevice,
			};
		}
		static auto Index(uint64_t size)
		{
			return BufferCreateInfo{
				.Size = size,
				.Usage = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				.MemUsage = vma::MemoryUsage::eAutoPreferDevice,
			};
		}
		static auto Uniform(uint64_t size)
		{
			return BufferCreateInfo{
				.Size = size,
				.Usage = vk::BufferUsageFlagBits::eUniformBuffer,
				.MemUsage = vma::MemoryUsage::eAutoPreferDevice,
				.AllocFlags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
			};
		}
		static auto Storage(uint64_t size)
		{
			return BufferCreateInfo{
				.Size = size,
				.Usage = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
				.MemUsage = vma::MemoryUsage::eAutoPreferDevice,
			};
		}
	};

	class Buffer : public IntrusivePtrEnabled<Buffer>
	{
	public:
		~Buffer();

		auto GetSize() const -> auto { return m_info.Size; }
		auto GetUsage() const -> auto { return m_info.Usage; }
		auto IsHostAccessible() const -> auto { return m_info.AllocFlags & vma::AllocationCreateFlagBits::eHostAccessSequentialWrite; }

	private:
		friend class Context;

		Buffer(Context* context, vk::Buffer buffer, vma::Allocation allocation, const BufferCreateInfo& info);

	private:
		Context* m_ctx;
		vk::Buffer m_buffer;
		vma::Allocation m_allocation;
		BufferCreateInfo m_info;
	};
	using BufferHandle = IntrusivePtr<Buffer>;

} // namespace VkMana
