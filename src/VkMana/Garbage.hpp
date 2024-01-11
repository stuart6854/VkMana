#pragma once

#include "Vulkan_Common.hpp"

#include <vector>

namespace VkMana
{
    class Context;

    class GarbageBin : public IntrusivePtrEnabled<GarbageBin>
    {
    public:
        ~GarbageBin();

        void Bin(vk::Semaphore semaphore);
        void Bin(vk::Fence fence);
        void Bin(vk::DescriptorSetLayout layout);
        void Bin(vk::PipelineLayout layout);
        void Bin(vk::Pipeline pipeline);
        void Bin(vk::Image image);
        void Bin(vk::ImageView view);
        void Bin(vk::Sampler sampler);
        void Bin(vk::Buffer buffer);
        void Bin(vma::Allocation alloc);
        void Bin(vk::QueryPool pool);

        void EmptyBins();

    private:
        friend class Context;

        GarbageBin(Context* context);

    private:
        Context* m_ctx;
        std::vector<vk::Semaphore> m_semaphores;
        std::vector<vk::Fence> m_fences;
        std::vector<vk::DescriptorSetLayout> m_setLayouts;
        std::vector<vk::PipelineLayout> m_pipelineLayouts;
        std::vector<vk::Pipeline> m_pipelines;
        std::vector<vk::Image> m_images;
        std::vector<vk::ImageView> m_imageViews;
        std::vector<vk::Sampler> m_samplers;
        std::vector<vk::Buffer> m_buffers;
        std::vector<vma::Allocation> m_allocs;
        std::vector<vk::QueryPool> m_queryPools;
    };

} // namespace VkMana
