#pragma once

#include "VkMana/GraphicsDevice.hpp"
#include "AllocationManager.hpp"
#include "SwapchainImpl.hpp"

#include <vulkan/vulkan.hpp>

#include <queue>
#include <memory>

namespace VkMana
{
	class GraphicsDevice::Impl
	{
	public:
		explicit Impl(const GraphicsDeviceOptions& options, SwapchainDescription* swapchainDescription, bool colorSrgb);
		~Impl();

		void CreateInstance(bool debug, const std::vector<const char*>& instanceExtensions);
		void CreatePhysicalDevice();
		void CreateLogicalDevice(vk::SurfaceKHR surface, const std::vector<const char*>& deviceExtensions);

		void SubmitCommands(CommandList& commandList, Fence* fence = nullptr);

		void SwapBuffers(Swapchain& swapchain);

		void WaitForIdle();

		auto GetInstance() const -> auto { return m_instance; }
		auto GetPhysicalDevice() const -> auto { return m_physicalDevice; }
		auto GetLogicalDevice() const -> auto { return m_device; }

		auto GetGraphicsFamilyIndex() const -> auto { return m_graphicsQueueIndex; }
		auto GetPresentFamilyIndex() const -> auto { return m_presentQueueIndex; }

		auto GetGraphicsQueue() const -> auto { return m_graphicsQueue; }
		// auto GetPresentQueue() const -> auto { return m_presentQueue; }

		auto SupportsDynamicRendering() const -> auto { return m_supportsDynamicRendering; }

		auto GetVulkanStats() -> auto& { return m_vulkanStats; }

	private:
		bool InstanceSupportsExtension(const char* extName);
		bool DeviceSupportsExtension(const char* extName);

		void GetQueueFamilyIndices(vk::SurfaceKHR surface);

		auto GetNextFreeFence() -> vk::Fence;

		// void FlushDeferredDisposals();

		struct FenceSubmitInfo
		{
			vk::Fence fence = nullptr;
			CommandList* cmdList = nullptr;
			vk::CommandBuffer cmdBuffer = nullptr;
		};
		void CheckSubmittedFences();						// Called in SubmitCommandList() & WaitForIdle()
		void CompleteFenceSubmission(FenceSubmitInfo info); // Called in CheckSubmittedFences()

	private:
		vk::Instance m_instance;
		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;

		std::int32_t m_graphicsQueueIndex = -1;
		std::int32_t m_presentQueueIndex = -1;

		vk::Queue m_graphicsQueue;

		AllocationManager m_allocationManager;

		std::queue<vk::Fence> m_freeFences;
		std::queue<FenceSubmitInfo> m_submittedFences;

		bool m_supportsDynamicRendering;

		VulkanStats m_vulkanStats;
	};
} // namespace VkMana