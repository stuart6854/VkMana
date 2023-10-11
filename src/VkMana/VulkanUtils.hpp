#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include <memory>
#include <optional>
#include <unordered_set>
#include <vector>

namespace VkMana::Vulkan
{
	bool CreateInstance(vk::Instance& outInstance, bool debug, std::vector<const char*> extensions);

	bool CreatePhysicalDevice(vk::PhysicalDevice& outPhysicalDevice, vk::Instance instance);

	struct QueueFamilyIndices
	{
		std::int32_t Graphics = -1;
		std::int32_t Present = -1;
	};
	auto GetQueueFamilyIndices(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) -> QueueFamilyIndices;

	bool CreateLogicalDevice(vk::Device& outDevice,
		vk::PhysicalDevice physicalDevice,
		QueueFamilyIndices queueFamilyIndices,
		std::vector<const char*> extensions);

	bool CreateAllocator(vma::Allocator& outAllocator, vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);

	auto SelectSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool colorSrgb);

	auto SelectPresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool vsync) -> vk::PresentModeKHR;

	bool CreateSwapchain(vk::SwapchainKHR& outSwapchain,
		vk::Format& outFormat,
		vk::Device device,
		vk::SurfaceKHR surface,
		std::uint32_t width,
		std::uint32_t height,
		bool vsync,
		vk::SwapchainKHR oldSwapchain,
		bool colorSrgb,
		vk::PhysicalDevice physicalDevice);

	bool CreateDeviceBuffer(vk::Buffer& outBuffer,
		vma::Allocation& outAllocation,
		vma::Allocator allocator,
		std::uint64_t size,
		vk::BufferUsageFlags usage,
		vma::AllocationCreateFlags allocFlags);

	bool CreateTexture(vk::Image& outImage,
		vma::Allocation& outAllocation,
		vma::Allocator allocator,
		std::uint32_t width,
		std::uint32_t height,
		std::uint32_t depth,
		std::uint32_t mipLevels,
		std::uint32_t arrayLayers,
		vk::Format format,
		vk::ImageUsageFlags usage);

	bool CreateCommandPool(vk::CommandPool& cmdPool, vk::Device device);

	bool AllocateCommandBuffer(vk::CommandBuffer& outCmdBuffer, vk::Device device, vk::CommandPool cmdPool);

	bool CreateFence(vk::Fence& outFence, vk::Device device);

	bool CreateImageView(vk::ImageView& outView,
		vk::Device device,
		vk::Image image,
		vk::ImageViewType type,
		vk::Format format,
		vk::ImageSubresourceRange subresource);

	bool TransitionImage(vk::CommandBuffer cmd,
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::ImageSubresourceRange subresource);

} // namespace VkMana::Internal::Vulkan