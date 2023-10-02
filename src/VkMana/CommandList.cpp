#include "VkMana/CommandList.hpp"

#include "Impl/CommandListImpl.hpp"

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	CommandList::CommandList(GraphicsDevice& graphicsDevice)
		: m_impl(new Impl(graphicsDevice))
	{
	}

	CommandList::~CommandList() = default;

	void CommandList::Reset()
	{
		m_impl->Reset();
	}

	void CommandList::Begin()
	{
		m_impl->Begin();
	}

	void CommandList::End()
	{
		m_impl->End();
	}

} // namespace VkMana