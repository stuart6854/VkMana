#include "CommandListImpl.hpp"

#include "GraphicsDeviceImpl.hpp"

namespace VkMana
{
	CommandList::Impl::Impl(GraphicsDevice& graphicsDevice)
		: m_graphicsDevice(graphicsDevice)
	{
		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();
		auto graphicsFamilyIndex = m_graphicsDevice.GetImpl()->GetGraphicsFamilyIndex();

		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.setQueueFamilyIndex(graphicsFamilyIndex);
		poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		m_cmdPool = device.createCommandPool(poolInfo);

		m_cmdBuffer = GetNextCommandBuffer();
	}

	CommandList::Impl::~Impl()
	{
		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();

		const auto cmdBufferCount = m_availableCmdBuffers.size() + m_submittedCmdBuffers.size();
		m_graphicsDevice.GetImpl()->GetVulkanStats().NumCommandBuffers -= cmdBufferCount;

		device.destroy(m_cmdPool);
	}

	void CommandList::Impl::Reset()
	{
		ClearCachedState();
		m_cmdBuffer.reset();
	}

	void CommandList::Impl::Begin()
	{
		if (m_hasBegun)
		{
			return; // #TODO: Error/Warn.
		}
		if (m_hasEnded)
		{
			m_hasEnded = false;
			m_cmdBuffer = GetNextCommandBuffer();
		}

		m_cmdBuffer = GetNextCommandBuffer();
		vk::CommandBufferBeginInfo beginInfo{};
		m_cmdBuffer.begin(beginInfo);
		m_hasBegun = true;

		ClearCachedState();
	}

	void CommandList::Impl::End()
	{
		if (!m_hasBegun)
		{
			return; // #TODO: Error/Warn.
		}
		if (m_hasEnded)
		{
			return; // #TODO: Error/Warn.
		}

		m_hasBegun = false;
		m_hasEnded = true;

		m_cmdBuffer.end();
	}

	void CommandList::Impl::CommandBufferSubmitted(vk::CommandBuffer commandBuffer) {}

	void CommandList::Impl::CommandBufferCompleted(vk::CommandBuffer commandBuffer)
	{
		for (auto i = 0; i < m_submittedCmdBuffers.size(); ++i)
		{
			auto& submittedCmd = m_submittedCmdBuffers[i];
			if (submittedCmd == commandBuffer)
			{
				m_submittedCmdBuffers.erase(m_submittedCmdBuffers.begin() + i);
				i--;
			}
		}

		m_availableCmdBuffers.push(commandBuffer);
	}

	auto CommandList::Impl::GetNextCommandBuffer() -> vk::CommandBuffer
	{
		if (!m_availableCmdBuffers.empty())
		{
			auto cmdBuffer = m_availableCmdBuffers.front();
			m_availableCmdBuffers.pop();
			cmdBuffer.reset();
			return cmdBuffer;
		}

		auto device = m_graphicsDevice.GetImpl()->GetLogicalDevice();
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.setCommandPool(m_cmdPool);
		allocInfo.setCommandBufferCount(1);
		allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
		auto cmd = device.allocateCommandBuffers(allocInfo)[0];

		m_graphicsDevice.GetImpl()->GetVulkanStats().NumCommandBuffers++;
		return cmd;
	}

	void CommandList::Impl::ClearCachedState() {}

} // namespace VkMana