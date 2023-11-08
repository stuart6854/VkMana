#pragma once

#include "Vulkan_Common.hpp"
#include "Garbage.hpp"
#include "CommandPool.hpp"
#include "CommandBuffer.hpp"
#include "WSI.hpp"
#include "Descriptors.hpp"
#include "Pipeline.hpp"
#include "Image.hpp"

namespace VkMana
{
	class Context
	{
	public:
		Context() = default;
		~Context();

		bool Init(WSI* mainWSI);

		bool AddSurface(WSI* wsi);
		void RemoveSurface(WSI* wsi);

		/* State */

		void BeginFrame();
		void EndFrame();
		void Present();

		/* Commands & Submission (Per Frame) */

		auto RequestCmd() -> CmdBuffer;

		void Submit(CmdBuffer cmd);

		/* Resources */

		auto GetSurfaceRenderPass(WSI* wsi) -> RenderPassInfo;

		auto CreateSetLayout(std::vector<vk::DescriptorSetLayoutBinding> bindings) -> SetLayoutHandle;
		auto CreatePipelineLayout(const PipelineLayoutCreateInfo& info) -> PipelineLayoutHandle;
		auto CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& info) -> PipelineHandle;
		auto CreateImageView(const Image* image, const ImageViewCreateInfo& info) -> ImageViewHandle;

		void DestroySetLayout(vk::DescriptorSetLayout setLayout);
		void DestroyPipelineLayout(vk::PipelineLayout pipelineLayout);
		void DestroyPipeline(vk::Pipeline pipeline);
		void DestroyImage(vk::Image image);
		void DestroyImageView(vk::ImageView view);
		void DestroyAllocation(vma::Allocation alloc);

		/* Getters */

		auto GetDevice() const -> auto { return m_device; }
		auto GetAllocator() const -> auto { return m_allocator; }

	private:
		struct QueueInfo
		{
			uint32_t GraphicsFamilyIndex = 0;
			vk::Queue GraphicsQueue;
		};

		static void PrintInstanceInfo();
		static void PrintDeviceInfo(vk::PhysicalDevice gpu);

		static bool FindQueueFamily(uint32_t outFamilyIndex, vk::PhysicalDevice gpu, vk::QueueFlags flags);

		static bool InitInstance(vk::Instance& outInstance);
		static bool SelectGPU(vk::PhysicalDevice& outGPU, vk::Instance instance);
		static bool InitDevice(vk::Device& outDevice, QueueInfo& outQueueInfo, vk::PhysicalDevice gpu);

		bool SetupFrames();

		struct PerFrame
		{
			vk::Fence FrameFence; // Waited on at start of frame & submitted at end of frame.
			IntrusivePtr<CommandPool> CmdPool;

			IntrusivePtr<Garbage> Garbage;
		};
		std::vector<PerFrame> m_frames;
		uint32_t m_frameIndex;

		auto GetFrame() -> auto& { return m_frames[m_frameIndex]; }
		auto GetFrame() const -> const auto& { return m_frames[m_frameIndex]; }

		struct SurfaceInfo
		{
			WSI* WSI = nullptr;
			vk::SurfaceKHR Surface;
			vk::SwapchainKHR Swapchain;
			uint32_t ImageIndex = 0;
			std::vector<ImageHandle> Images;
		};
		static bool CreateSwapchain(vk::SwapchainKHR& outSwapchain,
			uint32_t& outWidth,
			uint32_t& outHeight,
			vk::Format& outFormat,
			vk::Device device,
			uint32_t width,
			uint32_t height,
			bool vsync,
			bool srgb,
			vk::SwapchainKHR oldSwapchain,
			vk::SurfaceKHR surface,
			vk::PhysicalDevice gpu);

	private:
		vk::Instance m_instance;
		vk::PhysicalDevice m_gpu;
		vk::Device m_device;
		vma::Allocator m_allocator;

		QueueInfo m_queueInfo{};

		std::vector<SurfaceInfo> m_surfaces;

		std::vector<vk::Semaphore> m_submitWaitSemaphores;
		std::vector<vk::PipelineStageFlags> m_submitWaitStageMasks;
	};

} // namespace VkMana
