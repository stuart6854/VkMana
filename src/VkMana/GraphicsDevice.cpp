#include "VkMana/GraphicsDevice.hpp"

#include "Impl/GraphicsDeviceImpl.hpp"

#include <vulkan/vulkan.hpp>

namespace VkMana
{
	auto GraphicsDevice::Create(const GraphicsDeviceOptions& options, SwapchainDescription* swapchainDescription, bool colorSrgb)
		-> std::unique_ptr<GraphicsDevice>
	{
		return std::make_unique<GraphicsDevice>(options, swapchainDescription, colorSrgb);
	}

	GraphicsDevice::GraphicsDevice(const GraphicsDeviceOptions& options, SwapchainDescription* swapchainDescription, bool colorSrgb)
		: m_impl(new Impl(options, swapchainDescription, colorSrgb))
		, m_factory(new ResourceFactory(*this))
	{
	}

	GraphicsDevice::~GraphicsDevice() = default;

	void GraphicsDevice::SubmitCommands(CommandList& commandList, Fence* fence)
	{
		m_impl->SubmitCommands(commandList, fence);
	}

	auto GraphicsDevice::GetVulkanStats() const -> const VulkanStats&
	{
		return m_impl->GetVulkanStats();
	}

} // namespace VkMana