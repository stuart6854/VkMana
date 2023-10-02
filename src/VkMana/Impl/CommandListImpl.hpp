#pragma once

#include "VkMana/GraphicsDevice.hpp"

#include <vulkan/vulkan.hpp>

#include <queue>
#include <vector>

// #TODO: Make thread-safe
namespace VkMana
{
	class GraphicsDevice;

	class CommandList::Impl
	{
	public:
		explicit Impl(GraphicsDevice& graphicsDevice);
		~Impl();

		void Reset();

		void Begin();
		void End();

		void CommandBufferSubmitted(vk::CommandBuffer commandBuffer);
		void CommandBufferCompleted(vk::CommandBuffer commandBuffer);

		auto GetCmdBuffer() const -> auto { return m_cmdBuffer; }

	private:
		auto GetNextCommandBuffer() -> vk::CommandBuffer;

		void ClearCachedState();

	private:
		GraphicsDevice& m_graphicsDevice;
		vk::CommandPool m_cmdPool = nullptr;
		vk::CommandBuffer m_cmdBuffer = nullptr;

		bool m_hasBegun = false;
		bool m_hasEnded = false;

		std::queue<vk::CommandBuffer> m_availableCmdBuffers;
		std::vector<vk::CommandBuffer> m_submittedCmdBuffers;
	};

} // namespace VkMana
