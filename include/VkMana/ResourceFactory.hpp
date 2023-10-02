#pragma once

#include "CommandList.hpp"

#include <memory>

namespace VkMana
{
	class GraphicsDevice;

	class ResourceFactory
	{
	public:
		explicit ResourceFactory(GraphicsDevice& graphicsDevice);
		~ResourceFactory() = default;

		auto CreateCommandList() -> std::unique_ptr<CommandList>;

	private:
		GraphicsDevice& m_graphicsDevice;
	};
} // namespace VkMana