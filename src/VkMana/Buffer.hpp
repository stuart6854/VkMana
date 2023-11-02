#pragma once

#include "Vulkan_Headers.hpp"

namespace VkMana
{
	class Device;

	enum class BufferDomain
	{
		Device,		// Device local, likely not visible from CPU.
		HostAccess, // Device local, host-accessible
		HostLocal,	// Host local
	};

	struct BufferCreateInfo
	{
		BufferDomain Domain = BufferDomain::Device;
		vk::DeviceSize Size = 0;
		vk::BufferUsageFlags Usage = {};
	};

	class Buffer
	{
	public:
		~Buffer();

	private:
		Buffer(Device* device, vk::Buffer buffer, vma::Allocation alloc, const BufferCreateInfo& info);

	private:
		Device* m_device;
		vk::Buffer m_buffer;
		vma::Allocation m_alloc;
		vk::BufferCreateInfo m_info;
	};
	using BufferHandle = std::shared_ptr<Buffer>;

} // namespace VkMana
