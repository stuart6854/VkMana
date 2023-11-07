#include "CommandBuffer.hpp"

namespace VkMana
{
	CommandBuffer::CommandBuffer(Context* context, vk::CommandBuffer cmd)
		: m_ctx(context)
		, m_cmd(cmd)
	{
	}

} // namespace VkMana