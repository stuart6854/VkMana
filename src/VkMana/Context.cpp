#include "Context.hpp"
#include "vulkan/vulkan_wayland.h"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace VkMana
{
	Context::~Context()
	{
		if (m_device)
			m_device.waitIdle();

		while (!m_surfaces.empty())
		{
			RemoveSurface(m_surfaces.front().WindowSurface);
		}
		m_surfaces.clear();

		if (m_device)
		{
			m_linearSampler = nullptr;
			m_nearestSampler = nullptr;

			m_frames.clear();

			m_device.destroy(m_descriptorPool);

			if (m_allocator)
				m_allocator.destroy();

			m_device.destroy();
		}
		if (m_instance)
		{
			m_instance.destroy();
		}
	}

	bool Context::Init(WSI* mainWSI)
	{
		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		if (!InitInstance(m_instance))
			return false;
		if (!SelectGPU(m_gpu, m_instance))
			return false;
		if (!InitDevice(m_device, m_queueInfo, m_gpu))
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

		if (!SetupFrames())
			return false;

		if (!AddSurface(mainWSI))
			return false;

		std::vector<vk::DescriptorPoolSize> poolSizes{
			{ vk::DescriptorType::eUniformBuffer, 25 },
			{ vk::DescriptorType::eUniformBufferDynamic, 25 },
			{ vk::DescriptorType::eStorageBuffer, 25 },
			{ vk::DescriptorType::eStorageBufferDynamic, 25 },
			{ vk::DescriptorType::eCombinedImageSampler, 25 },
		};
		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.setPoolSizes(poolSizes);
		poolInfo.setMaxSets(500);
		poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind);
		m_descriptorPool = m_device.createDescriptorPool(poolInfo);

		m_nearestSampler = CreateSampler({
			.MinFilter = vk::Filter::eNearest,
			.MagFilter = vk::Filter::eNearest,
			.MipMapMode = vk::SamplerMipmapMode::eNearest,
		});
		m_linearSampler = CreateSampler({
			.MinFilter = vk::Filter::eLinear,
			.MagFilter = vk::Filter::eLinear,
			.MipMapMode = vk::SamplerMipmapMode::eLinear,
		});

		return true;
	}

	bool Context::AddSurface(WSI* wsi)
	{
		auto& newSurfaceInfo = m_surfaces.emplace_back();
		newSurfaceInfo.WindowSurface = wsi;
		newSurfaceInfo.Surface = wsi->CreateSurface(m_instance);

		uint32_t imageWidth = 0;
		uint32_t imageHeight = 0;
		vk::Format imageFormat = vk::Format::eUndefined;
		if (!CreateSwapchain(newSurfaceInfo.Swapchain,
				imageWidth,
				imageHeight,
				imageFormat,
				m_device,
				wsi->GetSurfaceWidth(),
				wsi->GetSurfaceHeight(),
				wsi->IsVSync(),
				true,
				{},
				newSurfaceInfo.Surface,
				m_gpu))
		{
			m_instance.destroy(newSurfaceInfo.Surface);
			m_surfaces.erase(m_surfaces.end() - 1);
			return false;
		}

		auto swapchainImages = m_device.getSwapchainImagesKHR(newSurfaceInfo.Swapchain);
		newSurfaceInfo.Images.resize(swapchainImages.size());
		for (auto i = 0u; i < swapchainImages.size(); ++i)
		{
			ImageCreateInfo info{
				.Width = imageWidth,
				.Height = imageHeight,
				.Depth = 1,
				.MipLevels = 1,
				.ArrayLayers = 1,
				.Format = imageFormat,
			};
			newSurfaceInfo.Images[i] = IntrusivePtr(new Image(this, swapchainImages[i], info));
		}

		return true;
	}

	void Context::RemoveSurface(WSI* wsi)
	{
		auto it = std::ranges::find_if(m_surfaces, [&](const SurfaceInfo& surfaceInfo) { return surfaceInfo.WindowSurface == wsi; });
		if (it == m_surfaces.end())
			return;

		auto& surfaceInfo = *it;
		m_device.destroy(surfaceInfo.Swapchain);
		m_instance.destroy(surfaceInfo.Surface);

		m_surfaces.erase(it);
	}

	void Context::BeginFrame()
	{
		m_frameIndex = (m_frameIndex + 1) % m_frames.size();
		auto& frame = GetFrame();

		if (!frame.FrameFences.empty())
		{
			UNUSED(m_device.waitForFences(frame.FrameFences, true, UINT64_MAX));
			m_device.resetFences(frame.FrameFences);
			frame.FrameFences.clear();
		}

		frame.CmdPool->ResetPool();
		frame.DescriptorAllocator->ResetAllocator();
		frame.Garbage->EmptyBins();

		for (auto& surfaceInfo : m_surfaces)
		{
			surfaceInfo.WindowSurface->PollEvents();

			auto acquireSemaphore = m_device.createSemaphore({});
			auto result = m_device.acquireNextImageKHR(surfaceInfo.Swapchain, UINT64_MAX, acquireSemaphore);
			surfaceInfo.ImageIndex = result.value;

			m_submitWaitSemaphores.push_back(acquireSemaphore);
			m_submitWaitStageMasks.emplace_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);
			frame.Garbage->Bin(acquireSemaphore);
		}
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

	void Context::Present()
	{
		std::vector<vk::SwapchainKHR> swapchains;
		std::vector<uint32_t> imageIndices;
		swapchains.reserve(m_surfaces.size());
		imageIndices.reserve(m_surfaces.size());
		for (auto& surface : m_surfaces)
		{
			swapchains.push_back(surface.Swapchain);
			imageIndices.push_back(surface.ImageIndex);
		}

		std::vector<vk::Result> results(swapchains.size());

		vk::PresentInfoKHR presentInfo{};
		presentInfo.setWaitSemaphores(m_presentWaitSemaphores);
		presentInfo.setSwapchains(swapchains);
		presentInfo.setImageIndices(imageIndices);
		presentInfo.setResults(results);

		UNUSED(m_queueInfo.GraphicsQueue.presentKHR(presentInfo));
		for (auto i = 0; i < results.size(); ++i)
		{
			// #TODO: Handle present result.
			// auto result = results[i];
			// auto& surface = m_surfaces[i];
		}

		m_presentWaitSemaphores.clear();
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

	auto Context::GetSurfaceRenderPass(WSI* wsi) -> RenderPassInfo
	{
		auto it = std::ranges::find_if(m_surfaces, [&](const SurfaceInfo& surfaceInfo) { return surfaceInfo.WindowSurface == wsi; });
		if (it == m_surfaces.end())
			return {};

		auto& surfaceInfo = *it;

		auto& image = surfaceInfo.Images.at(surfaceInfo.ImageIndex);

		RenderPassInfo info{
			.Targets = {
				RenderPassTarget{
					.Image = image->GetImageView(ImageViewType::RenderTarget),
					.IsDepthStencil = false,
					.Clear = true,
					.Store = true,
					.ClearValue = { 0.0f, 0.0f, 0.0f, 0.0f },
					.PreLayout = vk::ImageLayout::eUndefined,
					.PostLayout = vk::ImageLayout::ePresentSrcKHR,
				},
			},
		};
		return info;
	}

	auto Context::RequestDescriptorSet(const SetLayout* layout) -> DescriptorSetHandle
	{
		auto& frame = GetFrame();

		auto descriptorSet = frame.DescriptorAllocator->Allocate(layout->GetLayout());
		return IntrusivePtr(new DescriptorSet(this, descriptorSet));
	}

	auto Context::CreateSetLayout(std::vector<SetLayoutBinding> bindings) -> SetLayoutHandle
	{
		std::sort(bindings.begin(), bindings.end(), [](const auto& a, const auto& b) { return a.Binding < b.Binding; });

		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings(bindings.size());
		std::vector<vk::DescriptorBindingFlags> bindingFlags(bindings.size());
		for (auto i = 0u; i < bindings.size(); ++i)
		{
			const auto& binding = bindings[i];
			layoutBindings[i] = { binding.Binding, binding.Type, binding.Count, binding.StageFlags };
			bindingFlags[i] = binding.BindingFlags;
		}

		size_t hash = 0;
		for (auto i = 0; i < bindings.size(); ++i)
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

		return IntrusivePtr(new PipelineLayout(this, layout, hash, info));
	}

	auto Context::CreateGraphicsPipeline(const GraphicsPipelineCreateInfo& info) -> PipelineHandle
	{
		std::vector<vk::UniqueShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		auto CreateShaderModule = [&](const auto& shaderInfo, auto shaderStage) {
			if (!shaderInfo.SPIRVBinary.empty())
			{
				vk::ShaderModuleCreateInfo moduleInfo{};
				moduleInfo.setCode(shaderInfo.SPIRVBinary);
				shaderModules.push_back(m_device.createShaderModuleUnique(moduleInfo));

				auto& stageInfo = shaderStages.emplace_back();
				stageInfo.setStage(shaderStage);
				stageInfo.setModule(shaderModules.back().get());
				stageInfo.setPName(shaderInfo.EntryPoint.c_str());
			}
		};

		CreateShaderModule(info.Vertex, vk::ShaderStageFlagBits::eVertex);
		CreateShaderModule(info.Fragment, vk::ShaderStageFlagBits::eFragment);

		vk::PipelineVertexInputStateCreateInfo vertexInputState{};
		vertexInputState.setVertexAttributeDescriptions(info.VertexAttributes);
		vertexInputState.setVertexBindingDescriptions(info.VertexBindings);

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{};
		inputAssemblyState.setTopology(info.Topology);

		vk::PipelineTessellationStateCreateInfo tessellationState{};

		vk::PipelineViewportStateCreateInfo viewportState{};
		viewportState.setViewportCount(1); // Dynamic State
		viewportState.setScissorCount(1);  // Dynamic State

		vk::PipelineRasterizationStateCreateInfo rasterizationState{};
		rasterizationState.setFrontFace(vk::FrontFace::eClockwise);	 // #TODO: Make dynamic state.
		rasterizationState.setPolygonMode(vk::PolygonMode::eFill);	 // #TODO: Make dynamic state.
		rasterizationState.setCullMode(vk::CullModeFlagBits::eNone); // #TODO: Make dynamic state.
		rasterizationState.setLineWidth(1.0f);						 // #TODO: Make dynamic state.

		vk::PipelineMultisampleStateCreateInfo multisampleState{};

		vk::PipelineDepthStencilStateCreateInfo depthStencilState{};
		depthStencilState.setDepthTestEnable(VK_TRUE);			   // #TODO: Make dynamic state.
		depthStencilState.setDepthWriteEnable(VK_TRUE);			   // #TODO: Make dynamic state.
		depthStencilState.setDepthCompareOp(vk::CompareOp::eLess); // #TODO: Make dynamic state.

		vk::PipelineColorBlendAttachmentState defaultBlendAttachment{};
		defaultBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
			| vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
		// #TODO: Enable alpha blending (should just be below).
		defaultBlendAttachment.setBlendEnable(VK_TRUE);
		defaultBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha);
		defaultBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
		defaultBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd);
		defaultBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
		defaultBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
		defaultBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd);
		std::vector<vk::PipelineColorBlendAttachmentState> blendAttachments(info.ColorTargetFormats.size(), defaultBlendAttachment);
		vk::PipelineColorBlendStateCreateInfo colorBlendState{};
		colorBlendState.setAttachments(blendAttachments);

		std::vector<vk::DynamicState> dynStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.setDynamicStates(dynStates);

		vk::PipelineRenderingCreateInfo renderingInfo{};
		renderingInfo.setColorAttachmentFormats(info.ColorTargetFormats);
		renderingInfo.setDepthAttachmentFormat(info.DepthTargetFormat);

		vk::GraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.setStages(shaderStages);
		pipelineInfo.setPVertexInputState(&vertexInputState);
		pipelineInfo.setPInputAssemblyState(&inputAssemblyState);
		pipelineInfo.setPTessellationState(&tessellationState);
		pipelineInfo.setPViewportState(&viewportState);
		pipelineInfo.setPRasterizationState(&rasterizationState);
		pipelineInfo.setPMultisampleState(&multisampleState);
		pipelineInfo.setPDepthStencilState(&depthStencilState);
		pipelineInfo.setPColorBlendState(&colorBlendState);
		pipelineInfo.setPDynamicState(&dynamicState);
		pipelineInfo.setLayout(info.Layout->GetLayout());
		pipelineInfo.setPNext(&renderingInfo);
		auto pipeline = m_device.createGraphicsPipeline({}, pipelineInfo).value; // #TODO: Pipeline Cache

		return IntrusivePtr(new Pipeline(this, info.Layout, pipeline, vk::PipelineBindPoint::eGraphics));
	}

	auto Context::CreateComputePipeline(const ComputePipelineCreateInfo& info) -> PipelineHandle
	{
		vk::UniqueShaderModule shaderModule{};
		vk::PipelineShaderStageCreateInfo stageInfo{};
		if (!info.compute.SPIRVBinary.empty())
		{
			vk::ShaderModuleCreateInfo moduleInfo{};
			moduleInfo.setCode(info.compute.SPIRVBinary);
			shaderModule = m_device.createShaderModuleUnique(moduleInfo);

			stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
			stageInfo.setModule(shaderModule.get());
			stageInfo.setPName(info.compute.EntryPoint.c_str());
		}

		vk::ComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.setStage(stageInfo);
		pipelineInfo.setLayout(info.layout->GetLayout());
		auto pipeline = m_device.createComputePipeline({}, pipelineInfo).value; // #TODO: Pipeline cache

		return IntrusivePtr(new Pipeline(this, info.layout, pipeline, vk::PipelineBindPoint::eCompute));
	}

	auto Context::CreateImage(ImageCreateInfo info, const ImageDataSource* initialData) -> ImageHandle
	{
		if (info.MipLevels == -1)
			info.MipLevels = uint32_t(std::floor(std::log2(std::max(info.Width, info.Height)))) + 1;

		vk::ImageCreateInfo imageInfo{};
		imageInfo.setExtent({ info.Width, info.Height, info.Depth });
		imageInfo.setMipLevels(info.MipLevels);
		imageInfo.setArrayLayers(info.ArrayLayers);
		imageInfo.setFormat(info.Format);
		imageInfo.setUsage(info.Usage);
		imageInfo.setImageType(vk::ImageType::e2D);		   // #TODO: Make auto.
		imageInfo.setSamples(vk::SampleCountFlagBits::e1); // #TODO: Make optional.

		vma::AllocationCreateInfo allocInfo{};

		auto [image, allocation] = m_allocator.createImage(imageInfo, allocInfo);

		auto imageHandle = IntrusivePtr(new Image(this, image, allocation, info));

		if (initialData)
		{
			// Staging buffer.
			BufferDataSource bufferSource{
				.Size = initialData->Size,
				.Data = initialData->Data,
			};
			auto stagingBuffer = CreateBuffer(BufferCreateInfo::Staging(initialData->Size), &bufferSource);
			SetName(*stagingBuffer, "image_upload_staging_buffer");
			auto cmd = RequestCmd();

			// Transition all mip levels to TransferDst.
			ImageTransitionInfo preTransitionInfo{
				.TargetImage = imageHandle.Get(),
				.OldLayout = vk::ImageLayout::eUndefined,
				.NewLayout = vk::ImageLayout::eTransferDstOptimal,
				.MipLevelCount = uint32_t(info.MipLevels),
			};
			cmd->TransitionImage(preTransitionInfo); // #TODO: Transition all mip levels to TransferDst

			BufferToImageCopyInfo copyInfo{
				.SrcBuffer = stagingBuffer.Get(),
				.DstImage = imageHandle.Get(),
			};
			cmd->CopyBufferToImage(copyInfo);

			if (info.Flags & ImageCreateFlags_GenMipMaps)
			{
				// Generate MipMaps.

				auto mipWidth = int32_t(info.Width);
				auto mipHeight = int32_t(info.Height);

				for (auto i = 1u; i < uint32_t(info.MipLevels); ++i)
				{
					// Transition src mip to TransferSrc
					ImageTransitionInfo srcTransition{
						.TargetImage = imageHandle.Get(),
						.OldLayout = vk::ImageLayout::eTransferDstOptimal,
						.NewLayout = vk::ImageLayout::eTransferSrcOptimal,
						.BaseMipLevel = i - 1,
						.MipLevelCount = 1,
					};
					cmd->TransitionImage(srcTransition);

					// Blit src mip to dst mip

					ImageBlitInfo blitInfo{
						.SrcImage = imageHandle.Get(),
						.SrcRectEnd = { mipWidth, mipHeight, 1 },
						.SrcMipLevel = i - 1,
						.DstImage = imageHandle.Get(),
						.DstRectEnd = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 },
						.DstMipLevel = i,
						.Filter = vk::Filter::eLinear,
					};
					cmd->BlitImage(blitInfo);

					// Transition src mip to ShaderReadOnly
					ImageTransitionInfo shaderReadTransition{
						.TargetImage = imageHandle.Get(),
						.OldLayout = vk::ImageLayout::eTransferSrcOptimal,
						.NewLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
						.BaseMipLevel = i - 1,
						.MipLevelCount = 1,
					};
					cmd->TransitionImage(shaderReadTransition);

					if (mipWidth > 1)
						mipWidth /= 2;
					if (mipHeight > 1)
						mipHeight /= 2;
				}

				// Transition last mip level to ShaderReadOnly
				ImageTransitionInfo postTransitionInfo{
					.TargetImage = imageHandle.Get(),
					.OldLayout = vk::ImageLayout::eTransferDstOptimal,
					.NewLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
					.BaseMipLevel = uint32_t(info.MipLevels) - 1,
					.MipLevelCount = 1,
				};
				cmd->TransitionImage(postTransitionInfo);
			}
			else
			{
				// Transition all mip levels to ShaderReadOnly
				ImageTransitionInfo postTransitionInfo{
					.TargetImage = imageHandle.Get(),
					.OldLayout = vk::ImageLayout::eTransferDstOptimal,
					.NewLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
					.MipLevelCount = uint32_t(info.MipLevels),
				};
				cmd->TransitionImage(postTransitionInfo);
			}

			SubmitStaging(cmd);
		}

		return imageHandle;
	}

	auto Context::CreateImageView(const Image* image, const ImageViewCreateInfo& info) -> ImageViewHandle
	{
		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.setImage(image->GetImage());
		viewInfo.setFormat(image->GetFormat());
		viewInfo.setViewType(vk::ImageViewType::e2D);
		viewInfo.subresourceRange.setAspectMask(image->GetAspect());
		viewInfo.subresourceRange.setBaseMipLevel(info.BaseMipLevel);
		viewInfo.subresourceRange.setLevelCount(info.MipLevelCount);
		viewInfo.subresourceRange.setBaseArrayLayer(info.BaseArrayLayer);
		viewInfo.subresourceRange.setLayerCount(info.ArrayLayerCount);
		auto view = m_device.createImageView(viewInfo);

		return IntrusivePtr(new ImageView(this, image, view, info));
	}

	auto Context::CreateSampler(const SamplerCreateInfo& info) -> SamplerHandle
	{
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.setMinFilter(info.MinFilter);
		samplerInfo.setMagFilter(info.MagFilter);
		samplerInfo.setMipmapMode(info.MipMapMode);
		samplerInfo.setAddressModeU(info.AddressMode);
		samplerInfo.setAddressModeV(info.AddressMode);
		samplerInfo.setAddressModeW(info.AddressMode);
		samplerInfo.setMinLod(0.0f);
		samplerInfo.setMaxLod(VK_LOD_CLAMP_NONE);
		auto sampler = m_device.createSampler(samplerInfo);

		return IntrusivePtr(new Sampler(this, sampler));
	}

	auto Context::CreateBuffer(const BufferCreateInfo& info, const BufferDataSource* initialData) -> BufferHandle
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.setSize(info.Size);
		bufferInfo.setUsage(info.Usage);

		vma::AllocationCreateInfo allocInfo{};
		allocInfo.setUsage(info.MemUsage);
		allocInfo.setFlags(info.AllocFlags);

		auto [buffer, allocation] = m_allocator.createBuffer(bufferInfo, allocInfo);

		auto bufferHandle = IntrusivePtr(new Buffer(this, buffer, allocation, info));

		if (initialData)
		{
			if (bufferHandle->IsHostAccessible())
			{
				auto* mapped = m_allocator.mapMemory(allocation);
				std::memcpy(mapped, initialData->Data, initialData->Size);
				m_allocator.unmapMemory(allocation);
			}
			else
			{
				// Staging buffer.
				auto stagingBuffer = CreateBuffer(BufferCreateInfo::Staging(info.Size), initialData);
				SetName(*stagingBuffer, "buffer_upload_staging_buffer");
				auto cmd = RequestCmd();

				BufferCopyInfo copyInfo{
					.SrcBuffer = stagingBuffer.Get(),
					.DstBuffer = bufferHandle.Get(),
					.Size = info.Size,
				};
				cmd->CopyBuffer(copyInfo);
				SubmitStaging(cmd);
			}
		}

		return bufferHandle;
	}

	auto Context::CreateQueryPool(const QueryPoolCreateInfo& info) -> QueryPoolHandle
	{
		vk::QueryPoolCreateInfo poolInfo{};
		poolInfo.setQueryType(info.queryType);
		poolInfo.setQueryCount(info.queryCount);
		poolInfo.setPipelineStatistics(info.pipelineStatistics);
		auto pool = m_device.createQueryPool(poolInfo);

		return IntrusivePtr(new QueryPool(this, pool, info.queryCount));
	}

	void Context::DestroySetLayout(vk::DescriptorSetLayout setLayout)
	{
		GetFrame().Garbage->Bin(setLayout);
	}

	void Context::DestroyPipelineLayout(vk::PipelineLayout pipelineLayout)
	{
		GetFrame().Garbage->Bin(pipelineLayout);
	}

	void Context::DestroyPipeline(vk::Pipeline pipeline)
	{
		GetFrame().Garbage->Bin(pipeline);
	}

	void Context::DestroyImage(vk::Image image)
	{
		GetFrame().Garbage->Bin(image);
	}
	/**/
	void Context::DestroyImageView(vk::ImageView view)
	{
		GetFrame().Garbage->Bin(view);
	}

	void Context::DestroySampler(vk::Sampler sampler)
	{
		GetFrame().Garbage->Bin(sampler);
	}

	void Context::DestroyBuffer(vk::Buffer buffer)
	{
		GetFrame().Garbage->Bin(buffer);
	}

	void Context::DestroyAllocation(vma::Allocation alloc)
	{
		GetFrame().Garbage->Bin(alloc);
	}

	void Context::DestroyQueryPool(vk::QueryPool pool)
	{
		GetFrame().Garbage->Bin(pool);
	}

	void Context::SetName(const Buffer& buffer, const std::string& name)
	{
		SetName(uint64_t(VkBuffer(buffer.GetBuffer())), buffer.GetBuffer().objectType, name);
	}

	void Context::SetName(const Image& buffer, const std::string& name)
	{
		SetName(uint64_t(VkImage(buffer.GetImage())), buffer.GetImage().objectType, name);
	}

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

	auto Context::GetSwapchain(WSI& wsi) const -> vk::SwapchainKHR
	{
		for (const auto& surface : m_surfaces)
		{
			if (surface.WindowSurface == &wsi)
				return surface.Swapchain;
		}
		return nullptr;
	}

	void Context::PrintInstanceInfo()
	{
		auto layers = vk::enumerateInstanceLayerProperties();
		VM_INFO("{} Instance Layers:", layers.size());
		for (auto& layer : layers)
			VM_INFO("  - {}", layer.layerName.data());

		auto exts = vk::enumerateInstanceExtensionProperties();
		VM_INFO("{} Instance Extensions:", exts.size());
		for (auto& ext : exts)
			VM_INFO("  - {}", ext.extensionName.data());
	}

	void Context::PrintDeviceInfo(vk::PhysicalDevice gpu)
	{
		auto exts = gpu.enumerateDeviceExtensionProperties();
		VM_INFO("{} Device Extensions:", exts.size());
		for (auto& ext : exts)
			VM_INFO("  - {}", ext.extensionName.data());
	}

	bool Context::FindQueueFamily(uint32_t& outFamilyIndex, vk::PhysicalDevice gpu, vk::QueueFlags flags)
	{
		auto families = gpu.getQueueFamilyProperties();
		for (auto i = 0u; i < families.size(); ++i)
		{
			if (families[i].queueFlags & flags)
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
		for (const auto* layer : enabledLayers)
			VM_INFO("  - {}", layer);

		VM_INFO("Enabled Instance extensions:");
		for (const auto* ext : enabledExtensions)
			VM_INFO("  - {}", ext);

		return outInstance != VK_NULL_HANDLE;
	}

	bool Context::SelectGPU(vk::PhysicalDevice& outGPU, vk::Instance instance)
	{
		auto gpus = instance.enumeratePhysicalDevices();
		if (gpus.empty())
			return false;

		// #TODO: Prefer dedicated gpu with most device memory.
		outGPU = gpus[0];

		auto gpuProps = outGPU.getProperties();
		VM_INFO("GPU - {}", gpuProps.deviceName.data());
		VM_INFO("  API - {}.{}.{}",
			VK_API_VERSION_MAJOR(gpuProps.apiVersion),
			VK_API_VERSION_MINOR(gpuProps.apiVersion),
			VK_API_VERSION_PATCH(gpuProps.apiVersion));
		VM_INFO("  Driver - {}.{}.{}",
			VK_API_VERSION_MAJOR(gpuProps.driverVersion),
			VK_API_VERSION_MINOR(gpuProps.driverVersion),
			VK_API_VERSION_PATCH(gpuProps.driverVersion));

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

		if (!FindQueueFamily(outQueueInfo.GraphicsFamilyIndex, gpu, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer))
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
		descriptorIndexingFeatures.setRuntimeDescriptorArray(VK_TRUE);			// Support SPIRV RuntimeDescriptorArray capability.
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
		for (const auto* ext : enabledExtensions)
			VM_INFO("  - {}", ext);

		return outDevice != VK_NULL_HANDLE;
	}

	bool Context::SetupFrames()
	{
		m_frames.resize(2);
		for (auto& frame : m_frames)
		{
			frame.CmdPool = IntrusivePtr(new CommandPool(this, m_queueInfo.GraphicsFamilyIndex));
			frame.Garbage = IntrusivePtr(new GarbageBin(this));
			frame.DescriptorAllocator = IntrusivePtr(new DescriptorAllocator(this, 100));
		}

		m_frameIndex = 0;

		return true;
	}

	bool Context::CreateSwapchain(vk::SwapchainKHR& outSwapchain,
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
		vk::PhysicalDevice gpu)
	{
		if (!surface || !gpu || !device)
			return false;

		auto surfaceCaps = gpu.getSurfaceCapabilitiesKHR(surface);

		uint32_t minImageCount = surfaceCaps.minImageCount + 1;
		if (surfaceCaps.maxImageCount != 0 && minImageCount < surfaceCaps.maxImageCount)
			minImageCount = surfaceCaps.maxImageCount;

		auto surfaceFormat = vk::SurfaceFormatKHR(vk::Format::eB8G8R8A8Srgb);

		width = std::clamp(width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width);
		height = std::clamp(height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height);

		auto presentMode = vk::PresentModeKHR::eFifo;

		vk::SwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.setSurface(surface);
		swapchainInfo.setMinImageCount(minImageCount);
		swapchainInfo.setImageFormat(surfaceFormat.format);
		swapchainInfo.setImageColorSpace(surfaceFormat.colorSpace);
		swapchainInfo.setImageExtent({ width, height });
		swapchainInfo.setImageArrayLayers(1);
		swapchainInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
		swapchainInfo.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
		swapchainInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
		swapchainInfo.setPresentMode(presentMode);
		swapchainInfo.setClipped(VK_TRUE);
		swapchainInfo.setOldSwapchain(oldSwapchain);
		outSwapchain = device.createSwapchainKHR(swapchainInfo);

		outWidth = width;
		outHeight = height;
		outFormat = surfaceFormat.format;

		return outSwapchain != VK_NULL_HANDLE;
	}

} // namespace VkMana
