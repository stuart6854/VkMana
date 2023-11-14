#include "ShaderCompiler.hpp"

#include <shaderc/shaderc.hpp>

namespace VkMana
{
	namespace
	{
		auto GetShaderKind(const vk::ShaderStageFlagBits shaderStage) -> shaderc_shader_kind
		{
			switch (shaderStage)
			{
				case vk::ShaderStageFlagBits::eVertex:
					return shaderc_vertex_shader;
				case vk::ShaderStageFlagBits::eTessellationControl:
					return shaderc_tess_control_shader;
				case vk::ShaderStageFlagBits::eTessellationEvaluation:
					return shaderc_tess_evaluation_shader;
				case vk::ShaderStageFlagBits::eGeometry:
					return shaderc_geometry_shader;
				case vk::ShaderStageFlagBits::eFragment:
					return shaderc_fragment_shader;
				case vk::ShaderStageFlagBits::eCompute:
					return shaderc_compute_shader;
				default:
					LOG_ERR("Unknown shader kind.");
					assert(false);
					return {};
			}
		}

	} // namespace

	bool CompileShader(
		ShaderBinary& outSpirv, const std::string& glslSource, vk::ShaderStageFlagBits shaderStage, bool debug, const std::string& filename)
	{
		assert(!filename.empty());

		shaderc::CompileOptions options;
		if (debug)
			options.SetGenerateDebugInfo();
		else
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		auto kind = GetShaderKind(shaderStage);
		shaderc::Compiler compiler;
		auto result = compiler.CompileGlslToSpv(glslSource, kind, filename.c_str(), options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERR("Shader Compile Error: \n{}", result.GetErrorMessage());
			return false;
		}

		outSpirv.assign(result.cbegin(), result.cend());
		return true;
	}

} // namespace VkMana