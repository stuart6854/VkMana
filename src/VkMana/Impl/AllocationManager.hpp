#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.hpp>

#include <utility>

namespace VkMana
{
	class AllocationManager
	{
	public:
		AllocationManager() = default;
		explicit AllocationManager(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice);
		~AllocationManager();

		auto AllocateBuffer() -> std::pair<vk::Buffer, vma::Allocation>;
		auto AllocatorImage() -> std::pair<vk::Image, vma::Allocation>;

		struct AllocationInfo
		{
			std::uint64_t AllocatedBytes = 0; // Total allocated memory
			std::uint64_t UsedBytes = 0;	  // Total memory in use
			std::uint32_t NumBuffers = 0;
			std::uint32_t NumImages = 0;
		};
		auto GetAllocationInfo() const -> const auto& { return m_allocationInfo; }

	private:
		vma::Allocator m_allocator;

		AllocationInfo m_allocationInfo;
	};

} // namespace VkMana
