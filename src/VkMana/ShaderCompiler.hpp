#pragma once

#include "Vulkan_Common.hpp"
#include "Pipeline.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace VkMana
{
	enum class SourceLanguage
	{
		GLSL,
		HLSL,
	};

	struct ShaderCompileInfo
	{
		SourceLanguage SrcLanguage;
		std::filesystem::path SrcFilename;
		std::string SrcString;
		vk::ShaderStageFlagBits Stage;
		std::string EntryPoint = "main"; // Must be "main" for GLSL.
		bool Debug = false;
	};

	bool CompileShader(ShaderBinary& outSpirv,
		const std::string& glslSource,
		vk::ShaderStageFlagBits shaderStage,
		bool debug,
		const std::string& filename);

	auto CompileShader(ShaderCompileInfo info) -> std::optional<ShaderBinary>;

} // namespace VkMana
