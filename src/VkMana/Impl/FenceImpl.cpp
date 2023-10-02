#include "FenceImpl.hpp"

#include "GraphicsDeviceImpl.hpp"

namespace VkMana
{
	Fence::Impl::Impl(GraphicsDevice& graphicsDevice)
		: m_graphicsDevice(graphicsDevice)
	{
		vk::FenceCreateInfo fenceInfo{};
		m_fence = m_graphicsDevice.GetImpl()->GetLogicalDevice().createFence(fenceInfo);
	}

	Fence::Impl::~Impl()
	{
		m_graphicsDevice.GetImpl()->GetLogicalDevice().destroy(m_fence);
	}

} // namespace VkMana