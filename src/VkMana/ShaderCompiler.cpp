#include "ShaderCompiler.hpp"

#include <shaderc/shaderc.hpp>

#include <fstream>
#include <sstream>

namespace VkMana
{
	namespace
	{
		auto ReadFileStr(const std::filesystem::path& filename) -> std::optional<std::string>
		{
			std::ifstream stream(filename);
			if (!stream)
				return std::nullopt;

			std::stringstream ss;
			ss << stream.rdbuf();
			return ss.str();
		}

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
					VM_ERR("Unknown shader kind.");
					assert(false);
					return {};
			}
		}

	} // namespace

	auto CompileShaderSource(const ShaderCompileInfo& info) -> std::optional<ShaderBinary>
	{
		shaderc::CompileOptions options;
		if (info.Debug)
			options.SetGenerateDebugInfo();
		else
			options.SetOptimizationLevel(shaderc_optimization_level_performance);

		if (info.SrcLanguage == SourceLanguage::GLSL)
			options.SetSourceLanguage(shaderc_source_language_glsl);
		else if (info.SrcLanguage == SourceLanguage::HLSL)
			options.SetSourceLanguage(shaderc_source_language_hlsl);

		const char* sourceFile = "_no_file_";
		if (!info.SrcFilename.empty())
			sourceFile = info.SrcFilename.string().c_str();

		auto kind = GetShaderKind(info.Stage);
		shaderc::Compiler compiler;
		auto result = compiler.CompileGlslToSpv(info.SrcString, kind, sourceFile, options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			VM_ERR("Shader Compile Error:\n{}", result.GetErrorMessage());
			return std::nullopt;
		}

		ShaderBinary spirv;
		spirv.assign(result.cbegin(), result.cend());
		return spirv;
	}

	auto CompileShader(ShaderCompileInfo info) -> std::optional<ShaderBinary>
	{
		if (!info.SrcFilename.empty())
		{
			auto opt = ReadFileStr(info.SrcFilename);
			if (opt)
				info.SrcString = opt.value();
			else
			{
				VM_ERR("Failed to read Shader source from file: {}", info.SrcFilename.string());
			}
		}
		if (info.SrcString.empty())
		{
			VM_ERR("No shader source to compile.");
			return std::nullopt;
		}

		return CompileShaderSource(info);
	}

} // namespace VkMana