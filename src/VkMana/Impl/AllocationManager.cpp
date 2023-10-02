#include "AllocationManager.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	AllocationManager::AllocationManager(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device logicalDevice)
	{
		vma::VulkanFunctions functions{};
		functions.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

		vma::AllocatorCreateInfo allocInfo{};
		allocInfo.instance = instance;
		allocInfo.physicalDevice = physicalDevice;
		allocInfo.device = logicalDevice;
		allocInfo.vulkanApiVersion = VK_API_VERSION_1_1;
		allocInfo.pVulkanFunctions = &functions;
		m_allocator = vma::createAllocator(allocInfo);
	}

	AllocationManager::~AllocationManager()
	{
		m_allocator.destroy();
	}

	auto AllocationManager::AllocateBuffer() -> std::pair<vk::Buffer, vma::Allocation>
	{
		return std::pair<vk::Buffer, vma::Allocation>();
	}

	auto AllocationManager::AllocatorImage() -> std::pair<vk::Image, vma::Allocation>
	{
		return std::pair<vk::Image, vma::Allocation>();
	}

} // namespace VkMana