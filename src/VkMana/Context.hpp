#pragma once

#include "Buffer.hpp"
#include "CommandBuffer.hpp"
#include "CommandPool.hpp"
#include "DescriptorAllocator.hpp"
#include "Descriptors.hpp"
#include "Garbage.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "QueryPool.hpp"
#include "SwapChain.hpp"
#include "Vulkan_Common.hpp"

#include <unordered_map>

namespace VkMana
{
    class Context : public IntrusivePtrEnabled<Context>
    {
    public:
        static auto New() -> IntrusivePtr<Context>;

        Context() = default;
        ~Context();

        bool Init();

        /* State */

        void BeginFrame();
        void EndFrame();

        /* Commands & Submission (Per Frame) */

        auto RequestCmd() -> CmdBuffer;

        void Submit(CmdBuffer cmd);
        void SubmitStaging(CmdBuffer cmd);

        /* Resources */

        auto CreateSwapChain(vk::SurfaceKHR surface, uint32_t width, uint32_t height) -> SwapChainHandle;

        auto RequestDescriptorSet(const SetLayout* layout) -> DescriptorSetHandle;

        auto CreateSetLayout(std::vector<SetLayoutBinding> bindings) -> SetLayoutHandle;
        auto CreatePipelineLayout(const PipelineLayoutCreateInfo& info) -> PipelineLayoutHandle;
        auto CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& info) -> PipelineHandle;
        auto CreateComputePipeline(const ComputePipelineCreateInfo& info) -> PipelineHandle;
        auto CreateImage(ImageCreateInfo info, const ImageDataSource* pInitialData = nullptr) -> ImageHandle;
        auto CreateImageView(const Image* image, const ImageViewCreateInfo& info) -> ImageViewHandle;
        auto CreateSampler(const SamplerCreateInfo& info) -> SamplerHandle;
        auto CreateBuffer(const BufferCreateInfo& info, const BufferDataSource* pInitialData = nullptr) -> BufferHandle;
        auto CreateQueryPool(const QueryPoolCreateInfo& info) -> QueryPoolHandle;

        void DestroySetLayout(vk::DescriptorSetLayout setLayout);
        void DestroyPipelineLayout(vk::PipelineLayout pipelineLayout);
        void DestroyPipeline(vk::Pipeline pipeline);
        void DestroyImage(vk::Image image);
        void DestroyImageView(vk::ImageView view);
        void DestroySampler(vk::Sampler sampler);
        void DestroyBuffer(vk::Buffer buffer);
        void DestroyAllocation(vma::Allocation alloc);
        void DestroyQueryPool(vk::QueryPool pool);

        /* Debug */

        void SetName(const Buffer& buffer, const std::string& name);
        void SetName(const Image& buffer, const std::string& name);
        void SetName(const DescriptorSet& set, const std::string& name);
        void SetName(const Pipeline& pipeline, const std::string& name);
        void SetName(const CommandBuffer& buffer, const std::string& name);
        void SetName(uint64_t object, vk::ObjectType type, const std::string& name);

        /* Getters */

        auto GetInstance() const -> auto { return m_instance; }
        auto GetPhysicalDevice() const -> auto { return m_gpu; }
        auto GetDevice() const -> auto { return m_device; }
        auto GetAllocator() const -> auto { return m_allocator; }

        auto GetGraphicsQueueFamily() const -> auto { return m_queueInfo.GraphicsFamilyIndex; }
        auto GetGraphicsQueue() const -> auto { return m_queueInfo.GraphicsQueue; }

        auto GetFrameBufferCount() const -> auto { return m_frames.size(); }
        auto GetFrameIndex() const -> auto { return m_frameIndex; }

        auto GetNearestSampler() const -> auto { return m_nearestSampler.Get(); }
        auto GetLinearSampler() const -> auto { return m_linearSampler.Get(); }

    private:
        struct QueueInfo
        {
            uint32_t GraphicsFamilyIndex = 0;
            vk::Queue GraphicsQueue;
        };

        static void PrintInstanceInfo();
        static void PrintDeviceInfo(vk::PhysicalDevice gpu);

        static bool FindQueueFamily(uint32_t& outFamilyIndex, vk::PhysicalDevice gpu, vk::QueueFlags flags);

        static bool InitInstance(vk::Instance& outInstance);
        static bool SelectGPU(vk::PhysicalDevice& outGPU, vk::Instance instance);
        static bool InitDevice(vk::Device& outDevice, QueueInfo& outQueueInfo, vk::PhysicalDevice gpu);

        bool SetupFrames();

        struct PerFrame
        {
            std::vector<vk::Fence> FrameFences; // Waited on at start of frame & submitted at end of frame.
            IntrusivePtr<CommandPool> CmdPool;

            DescriptorAllocatorHandle DescriptorAllocator;

            IntrusivePtr<GarbageBin> Garbage;
        };
        std::vector<PerFrame> m_frames;
        uint32_t m_frameIndex;

        auto GetFrame() -> auto& { return m_frames[m_frameIndex]; }
        auto GetFrame() const -> const auto& { return m_frames[m_frameIndex]; }

    private:
        vk::Instance m_instance;
        vk::PhysicalDevice m_gpu;
        vk::Device m_device;
        vma::Allocator m_allocator;
        vk::DescriptorPool m_descriptorPool;

        SamplerHandle m_nearestSampler;
        SamplerHandle m_linearSampler;

        QueueInfo m_queueInfo{};

        std::vector<vk::Semaphore> m_submitWaitSemaphores;
        std::vector<vk::PipelineStageFlags> m_submitWaitStageMasks;
        std::vector<vk::Semaphore> m_presentWaitSemaphores;
    };
    using ContextHandle = IntrusivePtr<Context>;

} // namespace VkMana
