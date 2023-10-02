#include "VkMana/Fence.hpp"

#include "Impl/FenceImpl.hpp"

namespace VkMana
{
	Fence::Fence(GraphicsDevice& graphicsDevice) : m_impl(new Impl(graphicsDevice)){}

	Fence::~Fence() = default;
}