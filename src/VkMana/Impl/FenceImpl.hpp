#pragma once

#include "VkMana/Fence.hpp"

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	class GraphicsDevice;

	class Fence::Impl
	{
	public:
		explicit Impl(GraphicsDevice& graphicsDevice);
		~Impl();

		auto GetFence() const -> auto { return m_fence; }

	private:
		GraphicsDevice& m_graphicsDevice;

		vk::Fence m_fence;
	};
} // namespace VkMana
