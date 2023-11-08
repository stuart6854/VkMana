#pragma once

#include "Vulkan_Common.hpp"

#include <vector>

namespace VkMana
{
	class Context;

	class Garbage : public IntrusivePtrEnabled<Garbage>
	{
	public:
		~Garbage();

		void Bin(vk::Semaphore semaphore);
		void Bin(vk::Fence fence);

		void EmptyBins();

	private:
		friend class Context;

		Garbage(Context* context);

	private:
		Context* m_ctx;
		std::vector<vk::Semaphore> m_semaphores;
		std::vector<vk::Fence> m_fences;
	};

} // namespace VkMana
