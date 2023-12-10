#include "HelloTriangle.hpp"

#include "core/App.hpp"

#include <VkMana/ShaderCompiler.hpp>

const std::string TriangleVertexShaderSrc = R"(
#version 450

layout(location = 0) out vec4 outColor;

void main()
{
	const vec3 positions[3] = vec3[3](
		vec3(0.5, 0.5, 0.0),
		vec3(-0.5, 0.5, 0.0),
		vec3(0.0, -0.5, 0.0)
	);

	const vec4 colors[3] = vec4[3](
		vec4(1.0, 0.0, 0.0, 1.0),
		vec4(0.0, 1.0, 0.0, 1.0),
		vec4(0.0, 0.0, 1.0, 1.0)
	);

	gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
	outColor = colors[gl_VertexIndex];
}
)";
const std::string TriangleFragmentShaderSrc = R"(
#version 450

layout (location = 0) in vec4 inColor;
layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = inColor;
}
)";

namespace VkMana::SamplesApp
{
	bool SampleHelloTriangle::Onload(SamplesApp& app, Context& ctx)
	{
		const PipelineLayoutCreateInfo pipelineLayoutInfo{};
		auto pipelineLayout = ctx.CreatePipelineLayout(pipelineLayoutInfo);

		ShaderCompileInfo compileInfo{
			.SrcLanguage = SourceLanguage::GLSL,
			.SrcFilename = "",
			.SrcString = TriangleVertexShaderSrc,
			.Stage = vk::ShaderStageFlagBits::eVertex,
			.Debug = true,
		};
		const auto vertSpirvOpt = CompileShader(compileInfo);
		if (!vertSpirvOpt)
		{
			VM_ERR("Failed to compiler VERTEX shader.");
			return false;
		}

		compileInfo.SrcString = TriangleFragmentShaderSrc;
		compileInfo.Stage = vk::ShaderStageFlagBits::eFragment;
		const auto fragSpirvOpt = CompileShader(compileInfo);
		if (!fragSpirvOpt)
		{
			VM_ERR("Failed to compiler FRAGMENT shader.");
			return false;
		}

		const GraphicsPipelineCreateInfo pipelineInfo{
			.Vertex = vertSpirvOpt.value(),
			.Fragment = fragSpirvOpt.value(),
			.Topology = vk::PrimitiveTopology::eTriangleList,
			.ColorTargetFormats = { vk::Format::eB8G8R8A8Srgb },
			.Layout = pipelineLayout.Get(),
		};
		m_pipeline = ctx.CreateGraphicsPipeline(pipelineInfo);

		return m_pipeline != nullptr;
	}

	void SampleHelloTriangle::OnUnload(SamplesApp& app, Context& ctx)
	{
		m_pipeline = nullptr;
	}

	void SampleHelloTriangle::Tick(float deltaTime, SamplesApp& app, Context& ctx)
	{
		auto& window = app.GetWindow();
		const auto windowWidth = window.GetSurfaceWidth();
		const auto windowHeight = window.GetSurfaceHeight();

		auto cmd = ctx.RequestCmd();

		const auto rpInfo = ctx.GetSurfaceRenderPass(&app.GetWindow());
		cmd->BeginRenderPass(rpInfo);
		cmd->BindPipeline(m_pipeline.Get());
		cmd->SetViewport(0, 0, float(windowWidth), float(windowHeight));
		cmd->SetScissor(0, 0, windowWidth, windowHeight);
		cmd->Draw(3, 0);
		cmd->EndRenderPass();

		ctx.Submit(cmd);
	}

} // namespace VkMana::SamplesApp