#pragma once

#include "Vulkan_Headers.hpp"
#include "Context.hpp"
#include "Buffer.hpp"
#include "Image.hpp"

namespace VkMana
{
	class Device
	{
	public:
		// Device-based objects which need to poke at internal data structures.
		// Don't want to expose a lot of internal state for this to work.
		friend class Buffer;
		friend class Image;

		Device();
		~Device();

		// No move-copy
		Device(Device&&) = delete;
		void operator=(Device&&) = delete;

		// Only called by main thread, during setup.
		void SetContext(const Context& context);

		void InitFrameContexts(std::uint32_t count);

		/** Usually called automatically on `QueuePresent` when using WSI. */
		void NextFrameContext();

		auto RequestCommandBuffer() -> CommandBufferHandle;

		void Submit(CommandBufferHandle cmd, Fence* fence, std::vector<Semaphore*> semaphores);

		void WaitIdle() const;

		auto CreateBuffer(const BufferCreateInfo& info, const void* initialData = nullptr) -> BufferHandle;
		auto CreateImage(const ImageCreateInfo& info, const void* initialData = nullptr) -> ImageHandle;

	protected:
		struct PerFrame
		{
			PerFrame(Device* device, std::uint32_t index);
			~PerFrame();
			PerFrame(const PerFrame&) = delete;
			void operator=(const PerFrame&) = delete;

			void Begin();

			Device& Device;
			std::uint32_t FrameIndex;

			std::array<vk::CommandPool, QueueIndex_Count> CmdPools;

			std::vector<vk::Fence> WaitFences;
			std::vector<vk::Fence> RecycleFences;

			// std::array<std::vector<CommandBufferHandle>, QueueIndex_Count> Submissions;
			std::vector<vk::Buffer> DestroyedBuffers;
		};

	private:
		const Context* m_ctx;

		std::vector<std::unique_ptr<PerFrame>> m_perFrame;
	};

} // namespace VkMana
