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
		void Bin(vk::PipelineLayout layout);
		void Bin(vk::Pipeline pipeline);
		void Bin(vk::Image image);
		void Bin(vk::ImageView view);
		void Bin(vma::Allocation alloc);

		void EmptyBins();

	private:
		friend class Context;

		Garbage(Context* context);

	private:
		Context* m_ctx;
		std::vector<vk::Semaphore> m_semaphores;
		std::vector<vk::Fence> m_fences;
		std::vector<vk::PipelineLayout> m_pipelineLayouts;
		std::vector<vk::Pipeline> m_pipelines;
		std::vector<vk::Image> m_images;
		std::vector<vk::ImageView> m_imageViews;
		std::vector<vma::Allocation> m_allocs;
	};

} // namespace VkMana
