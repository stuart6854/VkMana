#include "VkMana/ResourceFactory.hpp"

namespace VkMana
{
	ResourceFactory::ResourceFactory(GraphicsDevice& graphicsDevice)
		: m_graphicsDevice(graphicsDevice)
	{
	}

	auto ResourceFactory::CreateCommandList() -> std::unique_ptr<CommandList>
	{
		return std::make_unique<CommandList>(m_graphicsDevice);
	}

} // namespace VkMana