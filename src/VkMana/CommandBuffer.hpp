#pragma once

#include "Vulkan_Common.hpp"

namespace VkMana
{
	class Context;

	class CommandBuffer : public IntrusivePtrEnabled<CommandBuffer>
	{
	public:
		~CommandBuffer() = default;

		auto GetCmd() const -> auto { return m_cmd; }

	private:
		friend class Context;

		CommandBuffer(Context* context, vk::CommandBuffer cmd);

	private:
		Context* m_ctx;
		vk::CommandBuffer m_cmd;
	};
	using CmdBuffer = IntrusivePtr<CommandBuffer>;

} // namespace VkMana
