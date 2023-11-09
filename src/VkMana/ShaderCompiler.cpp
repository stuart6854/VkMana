#include "ShaderCompiler.hpp"

namespace VkMana
{
	bool CompileShader(
		ShaderBinary& outSpirv, const std::string& glslSource, shaderc_shader_kind kind, bool debug, const std::string& filename)
	{
		shaderc::CompileOptions options;
		if (debug)
			options.SetGenerateDebugInfo();
		else
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		options.SetSourceLanguage(shaderc_source_language_glsl);

		shaderc::Compiler compiler;
		auto result = compiler.CompileGlslToSpv(glslSource, kind, filename.c_str(), options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERR("Shader Compile Error: {}", result.GetErrorMessage());
			return false;
		}

		outSpirv.assign(result.cbegin(), result.cend());
		return true;
	}

} // namespace VkMana