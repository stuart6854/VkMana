#include "Renderer.hpp"

#include <VkMana/ShaderCompiler.hpp>

#include <fstream>
#include <string>
#include <sstream>

using namespace VkMana;

namespace VkMana::Sample
{
	namespace Utils
	{
		bool ReadThenCompileShaderSource(
			ShaderBinary& outSpirv, Context& context, const shaderc_shader_kind kind, const std::string& filename)
		{
			const auto stream = std::ifstream(filename);
			if (!stream)
			{
				LOG_ERR("Failed to read file: {}", filename);
				return false;
			}

			std::stringstream ss;
			ss << stream.rdbuf();

			const auto glslSource = ss.str();

#ifdef _DEBUG
			constexpr bool debug = true;
#else
			constexpr bool debug = true;
#endif

			if (!CompileShader(outSpirv, glslSource, kind, debug, filename))
			{
				return false;
			}
			return true;
		}

	} // namespace Utils

	bool Renderer::Init(WSI* mainWindow)
	{
		m_ctx = IntrusivePtr(new Context);
		if (!m_ctx->Init(mainWindow))
		{
			return false;
		}
		m_mainWindow = mainWindow;
		m_mainWindowSize = { m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight() };

		const std::vector<uint8_t> WhitePixels = { 255, 255, 255, 255 };
		const auto whiteImageInfo = ImageCreateInfo::Texture(1, 1, 1);
		const ImageDataSource whiteImageDataSrc{ .Size = 1 * 1 * 4, .Data = WhitePixels.data() };
		m_whiteImage = m_ctx->CreateImage(whiteImageInfo, &whiteImageDataSrc);

		// #TODO: Make bindless.
		m_bindlessSetLayout = m_ctx->CreateSetLayout({
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
		});

		SetupGBufferPass();
		SetupCompositionPass();
		SetupScreenPass();

		return true;
	}

	void Renderer::SetSceneCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix)
	{
		m_cameraUniformData.projMatrix = projMatrix;
		m_cameraUniformData.viewMatrix = viewMatrix;
	}

	void Renderer::Submit(StaticMesh* mesh, glm::mat4 transform)
	{
		m_staticInstances.emplace_back(mesh, transform);
	}

	void Renderer::Flush()
	{
		m_ctx->BeginFrame();

		m_bindlessSet = m_ctx->RequestDescriptorSet(m_bindlessSetLayout.Get());
		m_bindlessSet->Write(m_whiteImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 0);

		auto cmd = m_ctx->RequestCmd();

		cmd->SetViewport(0, float(m_mainWindowSize.y), float(m_mainWindowSize.x), -float(m_mainWindowSize.y));
		cmd->SetScissor(0, 0, m_mainWindowSize.x, m_mainWindowSize.y);

		GBufferPass(cmd);
		CompositionPass(cmd);
		ScreenPass(cmd);

		m_ctx->Submit(cmd);

		m_ctx->EndFrame();
		m_ctx->Present();
	}

	auto Renderer::CreateMaterial() -> MaterialHandle
	{
		return IntrusivePtr(new Material(this));
	}

	auto Renderer::CreateStaticMesh() -> StaticMeshHandle
	{
		return IntrusivePtr(new StaticMesh(this));
	}

	void Renderer::SetupGBufferPass()
	{
		const auto depthImageInfo = ImageCreateInfo::DepthStencilTarget(m_mainWindowSize.x, m_mainWindowSize.y, false);
		m_depthTargetImage = m_ctx->CreateImage(depthImageInfo);

		const auto positionImageInfo =
			ImageCreateInfo::ColorTarget(m_mainWindowSize.x, m_mainWindowSize.y, vk::Format::eR16G16B16A16Sfloat);
		m_positionTargetImage = m_ctx->CreateImage(positionImageInfo);

		const auto normalImageInfo = ImageCreateInfo::ColorTarget(m_mainWindowSize.x, m_mainWindowSize.y, vk::Format::eR16G16B16A16Sfloat);
		m_normalTargetImage = m_ctx->CreateImage(normalImageInfo);

		const auto albedoImageInfo = ImageCreateInfo::ColorTarget(m_mainWindowSize.x, m_mainWindowSize.y, vk::Format::eR8G8B8A8Unorm);
		m_albedoTargetImage = m_ctx->CreateImage(albedoImageInfo);

		m_gBufferPass.Targets = {
			RenderPassTarget::DefaultColorTarget(m_positionTargetImage->GetImageView(ImageViewType::RenderTarget)),
			RenderPassTarget::DefaultColorTarget(m_normalTargetImage->GetImageView(ImageViewType::RenderTarget)),
			RenderPassTarget::DefaultColorTarget(m_albedoTargetImage->GetImageView(ImageViewType::RenderTarget)),
			RenderPassTarget::DefaultDepthStencilTarget(m_depthTargetImage->GetImageView(ImageViewType::RenderTarget)),
		};

		m_cameraSetLayout = m_ctx->CreateSetLayout({
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
		});

		const PipelineLayoutCreateInfo layoutInfo{
			.PushConstantRange = { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData) },
			.SetLayouts = { m_bindlessSetLayout.Get(), m_cameraSetLayout.Get() },
		};
		m_gBufferPipelineLayout = m_ctx->CreatePipelineLayout(layoutInfo);

		ShaderBinary vertexBinary;
		if (!Utils::ReadThenCompileShaderSource(vertexBinary, *m_ctx, shaderc_vertex_shader, "assets/shaders/deferred_gbuffer.vert"))
		{
			LOG_ERR("Failed to read/compile Vertex shader.");
			return;
		}
		ShaderBinary fragmentBinary;
		if (!Utils::ReadThenCompileShaderSource(fragmentBinary, *m_ctx, shaderc_fragment_shader, "assets/shaders/deferred_gbuffer.frag"))
		{
			LOG_ERR("Failed to read/compile Fragment shader.");
			return;
		}

		const GraphicsPipelineCreateInfo pipelineInfo{
			.Vertex = vertexBinary,
			.Fragment = fragmentBinary,
			.VertexAttributes = {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertex, Position)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(StaticVertex, TexCoord)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertex, Normal)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertex, Tangent)),
			},
			.VertexBindings = {
				vk::VertexInputBindingDescription(0, sizeof(StaticVertex), vk::VertexInputRate::eVertex),
			},
			.Topology =  vk::PrimitiveTopology::eTriangleList,
			.ColorTargetFormats = { positionImageInfo.Format, normalImageInfo.Format, albedoImageInfo.Format },
			.DepthTargetFormat = depthImageInfo.Format,
			.Layout = m_gBufferPipelineLayout.Get(),
		};
		m_gBufferStaticPipeline = m_ctx->CreateGraphicsPipeline(pipelineInfo);

		const auto cameraUniformBufferInfo = BufferCreateInfo::Uniform(sizeof(m_cameraUniformData) * 2);
		m_cameraUniformBuffer = m_ctx->CreateBuffer(cameraUniformBufferInfo);
	}

	void Renderer::SetupCompositionPass()
	{
		const auto compositionImageInfo = ImageCreateInfo::ColorTarget(m_mainWindowSize.x, m_mainWindowSize.y, vk::Format::eR8G8B8A8Unorm);
		m_compositionTargetImage = m_ctx->CreateImage(compositionImageInfo);

		m_compositionPass.Targets = {
			RenderPassTarget::DefaultColorTarget(m_compositionTargetImage->GetImageView(ImageViewType::RenderTarget)),
		};

		m_compositionSetLayout = m_ctx->CreateSetLayout({
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
			vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
		});

		const PipelineLayoutCreateInfo layoutInfo{
			.SetLayouts = { m_compositionSetLayout.Get() },
		};
		m_compositionPipelineLayout = m_ctx->CreatePipelineLayout(layoutInfo);

		ShaderBinary vertexBinary;
		if (!Utils::ReadThenCompileShaderSource(vertexBinary, *m_ctx, shaderc_vertex_shader, "assets/shaders/deferred_composition.vert"))
		{
			LOG_ERR("Failed to read/compile Vertex shader.");
			return;
		}
		ShaderBinary fragmentBinary;
		if (!Utils::ReadThenCompileShaderSource(
				fragmentBinary, *m_ctx, shaderc_fragment_shader, "assets/shaders/deferred_composition.frag"))
		{
			LOG_ERR("Failed to read/compile Fragment shader.");
			return;
		}

		const GraphicsPipelineCreateInfo pipelineInfo{
			.Vertex = vertexBinary,
			.Fragment = fragmentBinary,
			.Topology = vk::PrimitiveTopology::eTriangleList,
			.ColorTargetFormats = { compositionImageInfo.Format },
			.Layout = m_compositionPipelineLayout.Get(),
		};
		m_compositionPipeline = m_ctx->CreateGraphicsPipeline(pipelineInfo);
	}

	void Renderer::SetupScreenPass()
	{
		m_screenSetLayout = m_ctx->CreateSetLayout({
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment),
		});

		const PipelineLayoutCreateInfo layoutInfo{
			.SetLayouts = { m_screenSetLayout.Get() },
		};
		m_screenPipelineLayout = m_ctx->CreatePipelineLayout(layoutInfo);

		ShaderBinary vertexBinary;
		if (!Utils::ReadThenCompileShaderSource(vertexBinary, *m_ctx, shaderc_vertex_shader, "assets/shaders/fullscreen_quad.vert"))
		{
			LOG_ERR("Failed to read/compile Vertex shader.");
			return;
		}
		ShaderBinary fragmentBinary;
		if (!Utils::ReadThenCompileShaderSource(fragmentBinary, *m_ctx, shaderc_fragment_shader, "assets/shaders/fullscreen_quad.frag"))
		{
			LOG_ERR("Failed to read/compile Fragment shader.");
			return;
		}

		const GraphicsPipelineCreateInfo pipelineInfo{
			.Vertex = vertexBinary,
			.Fragment = fragmentBinary,
			.Topology = vk::PrimitiveTopology::eTriangleList,
			.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
			.Layout = m_screenPipelineLayout.Get(),
		};
		m_screenPipeline = m_ctx->CreateGraphicsPipeline(pipelineInfo);
	}

	void Renderer::GBufferPass(CmdBuffer& cmd)
	{
		m_cameraUniformBuffer->WriteHostAccessible(
			sizeof(CameraUniformData) * m_ctx->GetFrameIndex(), sizeof(CameraUniformData), &m_cameraUniformData);

		auto cameraSet = m_ctx->RequestDescriptorSet(m_cameraSetLayout.Get());
		cameraSet->Write(m_cameraUniformBuffer.Get(),
			0,
			vk::DescriptorType::eUniformBuffer,
			sizeof(CameraUniformData) * m_ctx->GetFrameIndex(),
			sizeof(CameraUniformData));

		cmd->BeginRenderPass(m_gBufferPass);

		cmd->BindPipeline(m_gBufferStaticPipeline.Get());
		cmd->BindDescriptorSets(0, { m_bindlessSet.Get(), cameraSet.Get() }, {});

		for (const auto& instance : m_staticInstances)
		{
			m_pushConstantData.modelMatrix = instance.Transform;

			cmd->BindIndexBuffer(instance.Mesh->GetIndexBuffer(), 0);
			cmd->BindVertexBuffers(0, { instance.Mesh->GetVertexBuffer() }, { 0 });
			cmd->SetPushConstants(
				vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData), &m_pushConstantData);

			for (auto& submesh : instance.Mesh->GetSubmeshes())
			{
				cmd->DrawIndexed(submesh.IndexCount, submesh.IndexOffset, submesh.VertexOffset);
			}
		}

		cmd->EndRenderPass();

		m_staticInstances.clear();
	}

	void Renderer::CompositionPass(CmdBuffer& cmd)
	{
		auto compositionSet = m_ctx->RequestDescriptorSet(m_compositionSetLayout.Get());
		compositionSet->Write(m_positionTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 0);
		compositionSet->Write(m_normalTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 1);
		compositionSet->Write(m_albedoTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 2);

		cmd->BeginRenderPass(m_compositionPass);
		cmd->BindPipeline(m_compositionPipeline.Get());
		cmd->BindDescriptorSets(0, { compositionSet.Get() }, {});
		cmd->Draw(3, 0);
		cmd->EndRenderPass();
	}

	void Renderer::ScreenPass(CmdBuffer& cmd)
	{
		auto screenTextureSet = m_ctx->RequestDescriptorSet(m_screenSetLayout.Get());
		screenTextureSet->Write(m_compositionTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 0);

		cmd->BeginRenderPass(m_ctx->GetSurfaceRenderPass(m_mainWindow));
		cmd->BindPipeline(m_screenPipeline.Get());
		cmd->BindDescriptorSets(0, { screenTextureSet.Get() }, {});
		cmd->Draw(3, 0);
		cmd->EndRenderPass();
	}

} // namespace VkMana::Sample
