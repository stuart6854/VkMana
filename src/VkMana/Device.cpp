#include "Device.hpp"

#include "Context.hpp"

namespace VkMana
{
	Device::Device() = default;

	Device::~Device()
	{
		WaitIdle();
	}

	void Device::SetContext(const Context& context)
	{
		m_ctx = &context;

		InitFrameContexts(2);
	}

	void Device::InitFrameContexts(std::uint32_t count)
	{
		WaitIdle();

		m_perFrame.clear();

		for (auto i = 0; i < count; ++i)
		{
			auto frame = std::make_unique<PerFrame>(this, i);
			m_perFrame.emplace_back(std::move(frame));
		}
	}

	void Device::WaitIdle() const {}

	auto Device::CreateBuffer(const BufferCreateInfo& info, const void* initialData) -> BufferHandle
	{
		vk::BufferCreateInfo createInfo{};
		createInfo.setSize(info.Size);
		createInfo.setUsage(info.Usage | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
		createInfo.setSharingMode(vk::SharingMode::eExclusive);

		// #TODO: Sharing indices -
		// https://github.com/Themaister/Granite/blob/5647c2b0b73e6529135d8ea2955656fe8ebed521/vulkan/device.cpp#L4587C53-L4587C53

		vma::MemoryUsage memUsage = vma::MemoryUsage::eAutoPreferDevice;
		if (info.Domain == BufferDomain::HostLocal)
			memUsage = vma::MemoryUsage::eAutoPreferHost;

		vma::AllocationCreateFlags memFlags;
		if (info.Domain == BufferDomain::HostAccess)
			memFlags |= vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;

		vma::AllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.setUsage(memUsage);
		allocCreateInfo.setFlags(memFlags);
	}

	Device::PerFrame::PerFrame(class Device* device, std::uint32_t index)
		: Device(*device)
		, FrameIndex(index)
	{
	}

	Device::PerFrame::~PerFrame() {}

	void Device::PerFrame::Begin()
	{
		auto device = Device.GetDevice();

		if (!WaitFences.empty())
		{
			device.waitForFences(WaitFences, VK_TRUE, u64(-1));
			WaitFences.clear();
		}

		if (!RecycleFences.empty())
		{
			device.resetFences(RecycleFences);
			// #TODO: Push fences onto queue somewhere
			RecycleFences.clear();
		}

		for(auto& v : DestroyedBuffers)
			device.destroy(v);

		DestroyedBuffers.clear();
	}

} // namespace VkMana