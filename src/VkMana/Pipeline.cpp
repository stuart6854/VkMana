#include "Pipeline.hpp"

#include "Context.hpp"

namespace VkMana
{
	auto PipelineLayout::New(Context* pContext, const PipelineLayoutCreateInfo& info) -> IntrusivePtr<PipelineLayout>
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

		// #TODO: Layout caching. e.g.
		// 	PipelineLayoutHandle cachedLayout = pContext->FindCachedPipelineLayout(hash)?
		// 	if(cachedLayout != nullptr) return cachedLayout;

		vk::PipelineLayoutCreateInfo layoutInfo{};
		if (info.PushConstantRange.size > 0)
			layoutInfo.setPushConstantRanges(info.PushConstantRange);
		layoutInfo.setSetLayouts(setLayouts);
		auto newPipelineLayout = pContext->GetDevice().createPipelineLayout(layoutInfo);
		if (newPipelineLayout == nullptr)
		{
			VM_ERR("Failed to create Pipeline Layout");
			return nullptr;
		}

		auto pNewPipelineLayout = IntrusivePtr(new PipelineLayout(pContext, newPipelineLayout, hash));
		// #TODO: pContext->CachePipelineLayout(pNewPipelineLayout);
		return pNewPipelineLayout;
	}

	PipelineLayout::~PipelineLayout()
	{
		if (m_layout)
			m_ctx->DestroyPipelineLayout(m_layout);
	}

	PipelineLayout::PipelineLayout(Context* context, vk::PipelineLayout layout, size_t hash)
		: m_ctx(context)
		, m_layout(layout)
		, m_hash(hash)
	{
	}

	auto Pipeline::NewGraphics(Context* pContext, const GraphicsPipelineCreateInfo& info) -> IntrusivePtr<Pipeline>
	{
		std::vector<vk::UniqueShaderModule> shaderModules;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		auto CreateShaderModule = [&](const auto& shaderInfo, auto shaderStage) {
			if (!shaderInfo.SPIRVBinary.empty())
			{
				vk::ShaderModuleCreateInfo moduleInfo{};
				moduleInfo.setCode(shaderInfo.SPIRVBinary);
				shaderModules.push_back(pContext->GetDevice().createShaderModuleUnique(moduleInfo));

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
		auto graphicsPipeline = pContext->GetDevice().createGraphicsPipeline({}, pipelineInfo).value; // #TODO: Pipeline Cache
		if (graphicsPipeline == nullptr)
		{
			VM_ERR("Failed to create Graphics Pipeline");
			return nullptr;
		}

		auto pNewPipeline = IntrusivePtr(new Pipeline(pContext, info.Layout, graphicsPipeline, vk::PipelineBindPoint::eGraphics));
		return pNewPipeline;
	}

	auto Pipeline::NewCompute(Context* pContext, const ComputePipelineCreateInfo& info) -> IntrusivePtr<Pipeline>
	{
		vk::UniqueShaderModule shaderModule{};
		vk::PipelineShaderStageCreateInfo stageInfo{};
		if (!info.compute.SPIRVBinary.empty())
		{
			vk::ShaderModuleCreateInfo moduleInfo{};
			moduleInfo.setCode(info.compute.SPIRVBinary);
			shaderModule = pContext->GetDevice().createShaderModuleUnique(moduleInfo);

			stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
			stageInfo.setModule(shaderModule.get());
			stageInfo.setPName(info.compute.EntryPoint.c_str());
		}

		vk::ComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.setStage(stageInfo);
		pipelineInfo.setLayout(info.layout->GetLayout());
		auto computePipeline = pContext->GetDevice().createComputePipeline({}, pipelineInfo).value; // #TODO: Pipeline cache
		if (computePipeline == nullptr)
		{
			VM_ERR("Failed to create Compute Pipeline");
			return nullptr;
		}

		auto pNewPipeline = IntrusivePtr(new Pipeline(pContext, info.layout, computePipeline, vk::PipelineBindPoint::eCompute));
		return pNewPipeline;
	}

	Pipeline::~Pipeline()
	{
		if (m_pipeline)
			m_ctx->DestroyPipeline(m_pipeline);
	}

	Pipeline::Pipeline(
		Context* pContext, const IntrusivePtr<PipelineLayout>& layout, vk::Pipeline pipeline, vk::PipelineBindPoint bindPoint)
		: m_ctx(pContext)
		, m_layout(layout)
		, m_pipeline(pipeline)
		, m_bindPoint(bindPoint)
	{
	}

} // namespace VkMana