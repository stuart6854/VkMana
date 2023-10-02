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
		: m_impl(new Impl)
		, m_factory(new ResourceFactory(*this))
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		m_impl->CreateInstance(options.Debug, options.InstanceExtensions);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_impl->GetInstance());

		vk::SurfaceKHR surface;
		if (swapchainDescription != nullptr)
		{
			//			surface = SurfaceUtils.CreateSurface(this, m_impl->GetInstance(), swapchainDescription->Source);
		}

		m_impl->CreatePhysicalDevice();
		m_impl->CreateLogicalDevice(surface, options.DeviceExtensions);

		VULKAN_HPP_DEFAULT_DISPATCHER.init(m_impl->GetInstance(), m_impl->GetLogicalDevice());

		m_allocator =
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