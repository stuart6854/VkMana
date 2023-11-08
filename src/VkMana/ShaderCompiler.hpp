#pragma once

#include "Vulkan_Common.hpp"

#include <shaderc/shaderc.hpp>

#include <string>
#include <vector>

namespace VkMana
{
	bool CompileShader(
		std::vector<uint32_t>& outSpirv, const std::string& glslSource, shaderc_shader_kind, bool debug, const std::string& filename = "");

} // namespace VkMana
