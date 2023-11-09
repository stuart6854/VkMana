#pragma once

#include "Vulkan_Common.hpp"
#include "Pipeline.hpp"

#include <shaderc/shaderc.hpp>

#include <string>

namespace VkMana
{
	bool CompileShader(
		ShaderBinary& outSpirv, const std::string& glslSource, shaderc_shader_kind, bool debug, const std::string& filename = "");

} // namespace VkMana
