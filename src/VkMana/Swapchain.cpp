#include "VkMana/Swapchain.hpp"

#include "Impl/SwapchainImpl.hpp"

namespace VkMana
{
	Swapchain::Swapchain(GraphicsDevice& graphicsDevice, SwapchainDescription& desc) : m_impl(new Impl(graphicsDevice, desc)){}

	Swapchain::~Swapchain() = default;

	void Swapchain::SetSyncToVerticalBlank(bool syncToVerticalBlank) {}

	void Swapchain::Resize(std::uint32_t width, std::uint32_t height) {}

} // namespace VkMana