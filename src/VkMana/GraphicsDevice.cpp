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
		if (swapchainDescription != nullptr)
		{
			m_mainSwapchain = std::make_shared<Swapchain>(*this, *swapchainDescription);
		}
	}

	GraphicsDevice::~GraphicsDevice() = default;

	void GraphicsDevice::SubmitCommands(CommandList& commandList, Fence* fence)
	{
		m_impl->SubmitCommands(commandList, fence);
	}

	void GraphicsDevice::SwapBuffers()
	{
		if (m_mainSwapchain == nullptr)
		{
			// #TODO: Error.
		}

		SwapBuffers(*m_mainSwapchain);
	}

	void GraphicsDevice::SwapBuffers(Swapchain& swapchain)
	{
		m_impl->SwapBuffers(swapchain);
	}

	void GraphicsDevice::WaitForIdle()
	{
		m_impl->WaitForIdle();
	}

	auto GraphicsDevice::GetVulkanStats() const -> const VulkanStats&
	{
		return m_impl->GetVulkanStats();
	}

} // namespace VkMana