#include "Context.hpp"

#include "ShaderCompiler.hpp"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace VkMana
{
    auto Context::New() -> IntrusivePtr<Context> { return IntrusivePtr(new Context); }

    Context::~Context()
    {
        if(m_device)
            m_device.waitIdle();

        if(m_device)
        {
            m_linearSampler = nullptr;
            m_nearestSampler = nullptr;

            m_frames.clear();

            m_device.destroy(m_descriptorPool);

            if(m_allocator)
                m_allocator.destroy();

            m_device.destroy();
        }
        if(m_instance)
        {
            m_instance.destroy();
        }
    }

    bool Context::Init()
    {
        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        if(!InitInstance(m_instance))
            return false;
        if(!SelectGPU(m_gpu, m_instance))
            return false;
        if(!InitDevice(m_device, m_queueInfo, m_gpu))
            return false;

        vma::VulkanFunctions vulkanFunctions{};
        vulkanFunctions.setVkGetInstanceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr);
        vulkanFunctions.setVkGetDeviceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);
        vma::AllocatorCreateInfo allocInfo{};
        allocInfo.setInstance(m_instance);
        allocInfo.setPhysicalDevice(m_gpu);
        allocInfo.setDevice(m_device);
        allocInfo.setVulkanApiVersion(VK_API_VERSION_1_3);
        allocInfo.setPVulkanFunctions(&vulkanFunctions);
        m_allocator = vma::createAllocator(allocInfo);

        if(!SetupFrames())
            return false;

        std::vector<vk::DescriptorPoolSize> poolSizes{
            {        vk::DescriptorType::eUniformBuffer, 25 },
            { vk::DescriptorType::eUniformBufferDynamic, 25 },
            {        vk::DescriptorType::eStorageBuffer, 25 },
            { vk::DescriptorType::eStorageBufferDynamic, 25 },
            { vk::DescriptorType::eCombinedImageSampler, 25 },
        };
        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.setPoolSizes(poolSizes);
        poolInfo.setMaxSets(500);
        poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
        m_descriptorPool = m_device.createDescriptorPool(poolInfo);

        m_nearestSampler = CreateSampler({
            .minFilter = vk::Filter::eNearest,
            .magFilter = vk::Filter::eNearest,
            .mipMapMode = vk::SamplerMipmapMode::eNearest,
        });
        m_linearSampler = CreateSampler({
            .minFilter = vk::Filter::eLinear,
            .magFilter = vk::Filter::eLinear,
            .mipMapMode = vk::SamplerMipmapMode::eLinear,
        });

        {
            m_singleImageSetLayout = CreateSetLayout({
                { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
            });
            auto fullscreenQuadPipelineLayout = CreatePipelineLayout({
                .PushConstantRange = {},
                .SetLayouts = { m_singleImageSetLayout.Get() },
            });

            const std::string vsSource = R"(
#version 460 core
layout(location = 0) out vec2 outTexCoord;
void main()
{
    outTexCoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outTexCoord * 2.0 - 1.0, 0.0, 1.0);
})";
            const std::string fsSource = R"(
#version 460 core
layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outFragColor;
layout(set = 0, binding = 0) uniform sampler2D uTexture;
void main()
{
    outFragColor = vec4(texture(uTexture, inTexCoord).rgb, 1.0);
})";

            ShaderCompileInfo shaderInfo{};
            shaderInfo.srcLanguage = SourceLanguage::GLSL;
            shaderInfo.pSrcStringStr = vsSource.c_str();
            shaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
            shaderInfo.debug = false;
            auto vsByteCode = CompileShader(shaderInfo);

            shaderInfo.pSrcStringStr = fsSource.c_str();
            shaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
            auto fsByteCode = CompileShader(shaderInfo);

            GraphicsPipelineCreateInfo fullscreenQuadPipelineInfo{};
            fullscreenQuadPipelineInfo.vs = {
                { vsByteCode->data(), uint32_t(vsByteCode->size()) }
            };
            fullscreenQuadPipelineInfo.fs = {
                { fsByteCode->data(), uint32_t(fsByteCode->size()) }
            };
            fullscreenQuadPipelineInfo.primitiveTopology = vk::PrimitiveTopology::eTriangleList;
            fullscreenQuadPipelineInfo.colorTargetCount = 1;
            fullscreenQuadPipelineInfo.colorFormats = { vk::Format::eB8G8R8A8Srgb };
            fullscreenQuadPipelineInfo.pPipelineLayout = fullscreenQuadPipelineLayout;
            m_fullscreenQuadPipeline = CreateGraphicsPipeline(fullscreenQuadPipelineInfo);
        }

        return true;
    }

    void Context::BeginFrame()
    {
        m_frameIndex = (m_frameIndex + 1) % m_frames.size();
        auto& frame = GetFrame();

        if(!frame.FrameFences.empty())
        {
            UNUSED(m_device.waitForFences(frame.FrameFences, true, UINT64_MAX));
            m_device.resetFences(frame.FrameFences);
            frame.FrameFences.clear();
        }

        frame.CmdPool->ResetPool();
        frame.DescriptorAllocator->ResetAllocator();
        frame.Garbage->EmptyBins();
    }

    void Context::EndFrame()
    {
        auto& frame = GetFrame();

        const auto signalSemaphore = m_device.createSemaphore({});
        const auto fence = m_device.createFence({});

        vk::SubmitInfo submitInfo{};
        submitInfo.setSignalSemaphores(signalSemaphore);
        m_queueInfo.GraphicsQueue.submit(submitInfo, fence);

        m_presentWaitSemaphores.push_back(signalSemaphore);
        frame.Garbage->Bin(signalSemaphore);

        frame.FrameFences.push_back(fence);
        frame.Garbage->Bin(fence);
    }

    auto Context::RequestCmd() -> CmdBuffer
    {
        auto cmd = GetFrame().CmdPool->RequestCmd();
        vk::CommandBufferBeginInfo beginInfo{};
        cmd.begin(beginInfo);
        return IntrusivePtr(new CommandBuffer(this, cmd));
    }

    void Context::Submit(CmdBuffer cmd)
    {
        auto commandBuffer = cmd->GetCmd();
        commandBuffer.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.setWaitSemaphores(m_submitWaitSemaphores);
        submitInfo.setWaitDstStageMask(m_submitWaitStageMasks);
        submitInfo.setCommandBuffers(commandBuffer);
        m_queueInfo.GraphicsQueue.submit(submitInfo);

        m_submitWaitSemaphores.clear();
        m_submitWaitStageMasks.clear();
    }

    void Context::SubmitStaging(CmdBuffer cmd)
    {
        Submit(std::move(cmd));

        auto fence = m_device.createFence({});
        m_queueInfo.GraphicsQueue.submit({}, fence);

        GetFrame().FrameFences.push_back(fence);
        GetFrame().Garbage->Bin(fence);
    }

    void Context::DrawFullScreenQuad(CmdBuffer& cmd, ImageHandle& image)
    {
        auto imageSet = RequestDescriptorSet(m_singleImageSetLayout.Get());
        imageSet->Write(image->GetImageView(ImageViewType::Texture), GetLinearSampler(), 0);

        cmd->BindPipeline(m_fullscreenQuadPipeline.Get());
        cmd->BindDescriptorSets(0, { imageSet.Get() }, {});
        cmd->Draw(3, 0);
    }

    auto Context::CreateSurface(void* windowHandle) -> vk::SurfaceKHR
    {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        vk::Win32SurfaceCreateInfoKHR surfaceInfo{};
        surfaceInfo.hinstance = GetModuleHandle(nullptr);
        surfaceInfo.hwnd = static_cast<HWND>(windowHandle);
        return m_instance.createWin32SurfaceKHR(surfaceInfo);
#else
        assert(false);
#endif
    }

    auto Context::CreateSwapChain(vk::SurfaceKHR surface, uint32_t width, uint32_t height) -> SwapChainHandle
    {
        return SwapChain::New(this, surface, width, height);
    }

    auto Context::RequestDescriptorSet(const SetLayout* layout) -> DescriptorSetHandle
    {
        auto& frame = GetFrame();

        auto descriptorSet = frame.DescriptorAllocator->Allocate(layout->GetLayout());
        return IntrusivePtr(new DescriptorSet(this, descriptorSet));
    }

    auto Context::CreateSetLayout(std::vector<SetLayoutBinding> bindings) -> SetLayoutHandle
    {
        std::sort(bindings.begin(), bindings.end(), [](const auto& a, const auto& b) { return a.binding < b.binding; });

        std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(bindings.size());
        std::vector<vk::DescriptorBindingFlags> bindingFlags(bindings.size());
        for(auto i = 0u; i < bindings.size(); ++i)
        {
            const auto& binding = bindings[i];
            layoutBindings[i] = { binding.binding, binding.type, binding.count, binding.stageFlags };
            bindingFlags[i] = binding.bindingFlags;
        }

        size_t hash = 0;
        for(auto i = 0; i < bindings.size(); ++i)
        {
            HashCombine(hash, layoutBindings[i]);
            HashCombine(hash, bindingFlags[i]);
        }

        // #TODO: Layout caching

        vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingsFlagsInfo{};
        bindingsFlagsInfo.setBindingFlags(bindingFlags);

        vk::DescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.setBindings(layoutBindings);
        layoutInfo.setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT);
        layoutInfo.setPNext(&bindingsFlagsInfo);
        const auto layout = m_device.createDescriptorSetLayout(layoutInfo);

        return IntrusivePtr(new SetLayout(this, layout, hash));
    }

    auto Context::CreatePipelineLayout(const PipelineLayoutCreateInfo& info) -> PipelineLayoutHandle
    {
#if 0
		size_t hash = 0;
		HashCombine(hash, info.PushConstantRange);

		std::vector<vk::DescriptorSetLayout> setLayouts(info.SetLayouts.size());
		for (auto i = 0u; i < info.SetLayouts.size(); ++i)
		{
			HashCombine(hash, i);
			if (info.SetLayouts[i])
			{
				HashCombine(hash, info.SetLayouts[i]->GetHash());
				setLayouts[i] = info.SetLayouts[i]->GetLayout();
			}
			else
				HashCombine(hash, 0);
		}

		// #TODO: Layout caching

		vk::PipelineLayoutCreateInfo layoutInfo{};
		if (info.PushConstantRange.size > 0)
			layoutInfo.setPushConstantRanges(info.PushConstantRange);
		layoutInfo.setSetLayouts(setLayouts);
		auto layout = m_device.createPipelineLayout(layoutInfo);

		return IntrusivePtr(new PipelineLayout(this, layout, hash));
#endif

        return PipelineLayout::New(this, info);
    }

    auto Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& info) -> PipelineHandle { return Pipeline::NewGraphics(this, info); }

    auto Context::CreateComputePipeline(const ComputePipelineCreateInfo& info) -> PipelineHandle { return Pipeline::NewCompute(this, info); }

    auto Context::CreateImage(ImageCreateInfo info, const ImageDataSource* pInitialData) -> ImageHandle
    {
        auto pImage = Image::New(this, info);

        if(pInitialData)
        {
            // Staging buffer.
            BufferDataSource bufferSource{
                .size = pInitialData->size,
                .pData = pInitialData->data,
            };
            auto pStagingBuffer = CreateBuffer(BufferCreateInfo::Staging(pInitialData->size), &bufferSource);
            assert(pStagingBuffer->IsHostAccessible());
            SetName(*pStagingBuffer, "image_upload_staging_buffer");
            auto cmd = RequestCmd();

            // Transition all mip levels to TransferDst.
            ImageTransitionInfo preTransitionInfo{
                .pImage = pImage.Get(),
                .oldLayout = vk::ImageLayout::eUndefined,
                .newLayout = vk::ImageLayout::eTransferDstOptimal,
                .mipLevelCount = pImage->GetMipLevels(),
            };
            cmd->TransitionImage(preTransitionInfo); // #TODO: Transition all mip levels to TransferDst

            BufferToImageCopyInfo copyInfo{
                .pSrcBuffer = pStagingBuffer.Get(),
                .pDstImage = pImage.Get(),
            };
            cmd->CopyBufferToImage(copyInfo);

            if(info.flags & ImageCreateFlags_GenMipMaps)
            {
                // Generate MipMaps.
                auto mipWidth = int32_t(pImage->GetWidth());
                auto mipHeight = int32_t(pImage->GetHeight());

                for(auto i = 1u; i < pImage->GetMipLevels(); ++i)
                {
                    // Transition src mip to TransferSrc
                    ImageTransitionInfo srcTransition{
                        .pImage = pImage.Get(),
                        .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                        .newLayout = vk::ImageLayout::eTransferSrcOptimal,
                        .baseMipLevel = i - 1,
                        .mipLevelCount = 1,
                    };
                    cmd->TransitionImage(srcTransition);

                    // Blit src mip to dst mip

                    ImageBlitInfo blitInfo{
                        .pSrcImage = pImage.Get(),
                        .srcRectEnd = {mipWidth, mipHeight, 1, },
                        .srcMipLevel = i - 1,
                        .pDstImage = pImage.Get(),
                        .dstRectEnd = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1, },
                        .dstMipLevel = i,
                        .filter = vk::Filter::eLinear,
                    };
                    cmd->BlitImage(blitInfo);

                    // Transition src mip to ShaderReadOnly
                    ImageTransitionInfo shaderReadTransition{
                        .pImage = pImage.Get(),
                        .oldLayout = vk::ImageLayout::eTransferSrcOptimal,
                        .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                        .baseMipLevel = i - 1,
                        .mipLevelCount = 1,
                    };
                    cmd->TransitionImage(shaderReadTransition);

                    if(mipWidth > 1)
                        mipWidth /= 2;
                    if(mipHeight > 1)
                        mipHeight /= 2;
                }

                // Transition last mip level to ShaderReadOnly
                ImageTransitionInfo postTransitionInfo{
                    .pImage = pImage.Get(),
                    .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                    .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                    .baseMipLevel = pImage->GetMipLevels() - 1,
                    .mipLevelCount = 1,
                };
                cmd->TransitionImage(postTransitionInfo);
            }
            else
            {
                // Transition all mip levels to ShaderReadOnly
                ImageTransitionInfo postTransitionInfo{
                    .pImage = pImage.Get(),
                    .oldLayout = vk::ImageLayout::eTransferDstOptimal,
                    .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
                    .mipLevelCount = pImage->GetMipLevels(),
                };
                cmd->TransitionImage(postTransitionInfo);
            }

            SubmitStaging(cmd);
        }

        return pImage;
    }

    auto Context::CreateImageView(const Image* image, const ImageViewCreateInfo& info) -> ImageViewHandle
    {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.setImage(image->GetImage());
        viewInfo.setFormat(image->GetFormat());
        viewInfo.setViewType(vk::ImageViewType::e2D);
        viewInfo.subresourceRange.setAspectMask(image->GetAspect());
        viewInfo.subresourceRange.setBaseMipLevel(info.baseMipLevel);
        viewInfo.subresourceRange.setLevelCount(info.mipLevelCount);
        viewInfo.subresourceRange.setBaseArrayLayer(info.baseArrayLayer);
        viewInfo.subresourceRange.setLayerCount(info.arrayLayerCount);
        auto view = m_device.createImageView(viewInfo);

        return IntrusivePtr(new ImageView(this, image, view, info));
    }

    auto Context::CreateSampler(const SamplerCreateInfo& info) -> SamplerHandle
    {
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.setMinFilter(info.minFilter);
        samplerInfo.setMagFilter(info.magFilter);
        samplerInfo.setMipmapMode(info.mipMapMode);
        samplerInfo.setAddressModeU(info.addressMode);
        samplerInfo.setAddressModeV(info.addressMode);
        samplerInfo.setAddressModeW(info.addressMode);
        samplerInfo.setMinLod(0.0f);
        samplerInfo.setMaxLod(VK_LOD_CLAMP_NONE);
        auto sampler = m_device.createSampler(samplerInfo);

        return IntrusivePtr(new Sampler(this, sampler));
    }

    auto Context::CreateBuffer(const BufferCreateInfo& info, const BufferDataSource* pInitialData) -> BufferHandle
    {
        auto pBuffer = Buffer::New(this, info);

        if(pInitialData != nullptr)
        {
            if(pBuffer->IsHostAccessible())
            {
                auto* dataPtr = pBuffer->Map();
                std::memcpy(dataPtr, pInitialData->pData, pInitialData->size);
                pBuffer->Unmap();
            }
            else
            {
                // Staging buffer.
                auto pStagingBuffer = CreateBuffer(BufferCreateInfo::Staging(info.size), pInitialData);
                assert(pStagingBuffer->IsHostAccessible());
                SetName(*pStagingBuffer, "buffer_upload_staging_buffer");
                auto cmd = RequestCmd();

                BufferCopyInfo copyInfo{
                    .pSrcBuffer = pStagingBuffer.Get(),
                    .pDstBuffer = pBuffer.Get(),
                    .size = info.size,
                };
                cmd->CopyBuffer(copyInfo);
                SubmitStaging(cmd);
            }
        }

        return pBuffer;
    }

    auto Context::CreateQueryPool(const QueryPoolCreateInfo& info) -> QueryPoolHandle { return QueryPool::New(this, info); }

    void Context::DestroySetLayout(vk::DescriptorSetLayout setLayout) { GetFrame().Garbage->Bin(setLayout); }

    void Context::DestroyPipelineLayout(vk::PipelineLayout pipelineLayout) { GetFrame().Garbage->Bin(pipelineLayout); }

    void Context::DestroyPipeline(vk::Pipeline pipeline) { GetFrame().Garbage->Bin(pipeline); }

    void Context::DestroyImage(vk::Image image) { GetFrame().Garbage->Bin(image); }

    void Context::DestroyImageView(vk::ImageView view) { GetFrame().Garbage->Bin(view); }

    void Context::DestroySampler(vk::Sampler sampler) { GetFrame().Garbage->Bin(sampler); }

    void Context::DestroyBuffer(vk::Buffer buffer) { GetFrame().Garbage->Bin(buffer); }

    void Context::DestroyAllocation(vma::Allocation alloc) { GetFrame().Garbage->Bin(alloc); }

    void Context::DestroyQueryPool(vk::QueryPool pool) { GetFrame().Garbage->Bin(pool); }

    void Context::SetName(const Buffer& buffer, const std::string& name)
    {
        SetName(uint64_t(VkBuffer(buffer.GetBuffer())), buffer.GetBuffer().objectType, name);
    }

    void Context::SetName(const Image& buffer, const std::string& name) { SetName(uint64_t(VkImage(buffer.GetImage())), buffer.GetImage().objectType, name); }

    void Context::SetName(const DescriptorSet& set, const std::string& name)
    {
        SetName(uint64_t(VkDescriptorSet(set.GetSet())), set.GetSet().objectType, name);
    }

    void Context::SetName(const Pipeline& pipeline, const std::string& name)
    {
        SetName(uint64_t(VkPipeline(pipeline.GetPipeline())), pipeline.GetPipeline().objectType, name);
    }

    void Context::SetName(const CommandBuffer& buffer, const std::string& name)
    {
        SetName(uint64_t(VkCommandBuffer(buffer.GetCmd())), buffer.GetCmd().objectType, name);
    }

    void Context::SetName(uint64_t object, vk::ObjectType type, const std::string& name)
    {
        vk::DebugUtilsObjectNameInfoEXT nameInfo{};
        nameInfo.setObjectType(type);
        nameInfo.setObjectHandle(object);
        nameInfo.setPObjectName(name.c_str());
        m_device.setDebugUtilsObjectNameEXT(nameInfo);
    }

    void Context::PrintInstanceInfo()
    {
        auto layers = vk::enumerateInstanceLayerProperties();
        VM_INFO("{} Instance Layers:", layers.size());
        for(auto& layer : layers)
            VM_INFO("  - {}", layer.layerName.data());

        auto exts = vk::enumerateInstanceExtensionProperties();
        VM_INFO("{} Instance Extensions:", exts.size());
        for(auto& ext : exts)
            VM_INFO("  - {}", ext.extensionName.data());
    }

    void Context::PrintDeviceInfo(vk::PhysicalDevice gpu)
    {
        auto exts = gpu.enumerateDeviceExtensionProperties();
        VM_INFO("{} Device Extensions:", exts.size());
        for(auto& ext : exts)
            VM_INFO("  - {}", ext.extensionName.data());
    }

    bool Context::FindQueueFamily(uint32_t& outFamilyIndex, vk::PhysicalDevice gpu, vk::QueueFlags flags)
    {
        auto families = gpu.getQueueFamilyProperties();
        for(auto i = 0u; i < families.size(); ++i)
        {
            if(families[i].queueFlags & flags)
            {
                outFamilyIndex = i;
                return true;
            }
        }

        return false;
    }

    bool Context::InitInstance(vk::Instance& outInstance)
    {
        // PrintInstanceInfo();

        vk::ApplicationInfo appInfo{};
        appInfo.setApiVersion(VK_API_VERSION_1_3);

        std::vector<const char*> enabledLayers{
#ifdef _DEBUG
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_KHRONOS_synchronization2",
#endif
        };
        std::vector<const char*> enabledExtensions{
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif __linux__
            VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
        };

        vk::InstanceCreateInfo instanceInfo{};
        instanceInfo.setPApplicationInfo(&appInfo);
        instanceInfo.setPEnabledLayerNames(enabledLayers);
        instanceInfo.setPEnabledExtensionNames(enabledExtensions);
        outInstance = vk::createInstance(instanceInfo);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(outInstance);

        VM_INFO("Enabled Instance layers:");
        for(const auto* layer : enabledLayers)
            VM_INFO("  - {}", layer);

        VM_INFO("Enabled Instance extensions:");
        for(const auto* ext : enabledExtensions)
            VM_INFO("  - {}", ext);

        return outInstance != VK_NULL_HANDLE;
    }

    bool Context::SelectGPU(vk::PhysicalDevice& outGPU, vk::Instance instance)
    {
        auto gpus = instance.enumeratePhysicalDevices();
        if(gpus.empty())
            return false;

        // #TODO: Prefer dedicated gpu with most device memory.
        outGPU = gpus[0];

        auto gpuProps = outGPU.getProperties();
        VM_INFO("GPU - {}", gpuProps.deviceName.data());
        VM_INFO(
            "  API - {}.{}.{}",
            VK_API_VERSION_MAJOR(gpuProps.apiVersion),
            VK_API_VERSION_MINOR(gpuProps.apiVersion),
            VK_API_VERSION_PATCH(gpuProps.apiVersion)
        );
        VM_INFO(
            "  Driver - {}.{}.{}",
            VK_API_VERSION_MAJOR(gpuProps.driverVersion),
            VK_API_VERSION_MINOR(gpuProps.driverVersion),
            VK_API_VERSION_PATCH(gpuProps.driverVersion)
        );

        return outGPU != VK_NULL_HANDLE;
    }

    bool Context::InitDevice(vk::Device& outDevice, QueueInfo& outQueueInfo, vk::PhysicalDevice gpu)
    {
        // PrintDeviceInfo(gpu);

        /* Extensions */

        std::vector<const char*> enabledExtensions{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        /* Queues */

        if(!FindQueueFamily(outQueueInfo.GraphicsFamilyIndex, gpu, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer))
            return false;

        float queuePriority = 0.5f;
        std::vector<vk::DeviceQueueCreateInfo> queueInfos{};
        auto& graphicsQueueInfo = queueInfos.emplace_back();
        graphicsQueueInfo.setQueueFamilyIndex(outQueueInfo.GraphicsFamilyIndex);
        graphicsQueueInfo.setQueuePriorities(queuePriority);
        graphicsQueueInfo.setQueueCount(1);

        /* Features */

        vk::PhysicalDeviceFeatures enabledFeatures{};

        /* Extension Features */

        vk::PhysicalDeviceHostQueryResetFeatures hostQueryResetFeatures{};
        hostQueryResetFeatures.setHostQueryReset(VK_TRUE);

        vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
        descriptorIndexingFeatures.setRuntimeDescriptorArray(VK_TRUE);          // Support SPIRV RuntimeDescriptorArray capability.
        descriptorIndexingFeatures.setDescriptorBindingPartiallyBound(VK_TRUE); // Descriptor sets do not need to have valid descriptors.
        descriptorIndexingFeatures.setShaderSampledImageArrayNonUniformIndexing(VK_TRUE);
        descriptorIndexingFeatures.setShaderUniformBufferArrayNonUniformIndexing(VK_TRUE);
        descriptorIndexingFeatures.setShaderStorageBufferArrayNonUniformIndexing(VK_TRUE);
        descriptorIndexingFeatures.setDescriptorBindingSampledImageUpdateAfterBind(VK_TRUE);
        descriptorIndexingFeatures.setDescriptorBindingStorageBufferUpdateAfterBind(VK_TRUE);
        descriptorIndexingFeatures.setDescriptorBindingUniformBufferUpdateAfterBind(VK_TRUE);
        descriptorIndexingFeatures.setPNext(&hostQueryResetFeatures);

        vk::PhysicalDeviceSynchronization2Features sync2Features{};
        sync2Features.setSynchronization2(VK_TRUE);
        sync2Features.setPNext(&descriptorIndexingFeatures);

        vk::PhysicalDeviceDynamicRenderingFeatures dynRenderFeatures{};
        dynRenderFeatures.setDynamicRendering(VK_TRUE);
        dynRenderFeatures.setPNext(&sync2Features);

        /* Device Create Info */

        vk::DeviceCreateInfo deviceInfo{};
        deviceInfo.setPEnabledExtensionNames(enabledExtensions);
        deviceInfo.setQueueCreateInfos(queueInfos);
        deviceInfo.setPEnabledFeatures(&enabledFeatures);
        deviceInfo.setPNext(&dynRenderFeatures);
        outDevice = gpu.createDevice(deviceInfo);

        VULKAN_HPP_DEFAULT_DISPATCHER.init(outDevice);

        outQueueInfo.GraphicsQueue = outDevice.getQueue(outQueueInfo.GraphicsFamilyIndex, 0);

        VM_INFO("Enabled Device extensions:");
        for(const auto* ext : enabledExtensions)
            VM_INFO("  - {}", ext);

        return outDevice != VK_NULL_HANDLE;
    }

    bool Context::SetupFrames()
    {
        m_frames.resize(2);
        for(auto& frame : m_frames)
        {
            frame.CmdPool = IntrusivePtr(new CommandPool(this, m_queueInfo.GraphicsFamilyIndex));
            frame.Garbage = IntrusivePtr(new GarbageBin(this));
            frame.DescriptorAllocator = IntrusivePtr(new DescriptorAllocator(this, 100));
        }

        m_frameIndex = 0;

        return true;
    }
} // namespace VkMana
