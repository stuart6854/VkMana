#include "CommandPool.hpp"

#include "Context.hpp"

namespace VkMana
{
	CommandPool::~CommandPool()
	{
		m_ctx->GetDevice().destroy(m_pool);
	}

	auto CommandPool::RequestCmd() -> vk::CommandBuffer
	{
		if (m_index >= m_cmds.size())
		{
			vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.setCommandPool(m_pool);
			allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
			allocInfo.setCommandBufferCount(1);
			return m_ctx->GetDevice().allocateCommandBuffers(allocInfo)[0];
		}

		auto cmd = m_cmds[m_index++];
		return cmd;
	}

	void CommandPool::ResetPool()
	{
		m_ctx->GetDevice().resetCommandPool(m_pool);
		m_index = 0;
	}

	CommandPool::CommandPool(Context* context, uint32_t queueFamilyIndex)
		: m_ctx(context)
	{
		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.setQueueFamilyIndex(queueFamilyIndex);
		m_pool = m_ctx->GetDevice().createCommandPool(poolInfo);
	}

} // namespace VkMana