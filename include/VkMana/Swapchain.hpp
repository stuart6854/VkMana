#pragma once

#include "SwapchainDescription.hpp"

#include <memory>

namespace VkMana
{
	class GraphicsDevice;

	class Swapchain
	{
	public:
		Swapchain(GraphicsDevice& graphicsDevice, SwapchainDescription& desc);
		~Swapchain();

		// void SetName(const std::string& name);
		void SetSyncToVerticalBlank(bool syncToVerticalBlank);

		void Resize(std::uint32_t width, std::uint32_t height);

		// auto GetFramebuffer() const -> const auto&;

		auto GetImpl() -> auto* { return m_impl.get(); }

	private:
		class Impl;
		std::unique_ptr<Impl> m_impl;
	};

} // namespace VkMana