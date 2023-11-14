#pragma once

#include "Vulkan_Common.hpp"
#include "Pipeline.hpp"

#include <string>

namespace VkMana
{
	bool CompileShader(ShaderBinary& outSpirv,
		const std::string& glslSource,
		vk::ShaderStageFlagBits shaderStage,
		bool debug,
		const std::string& filename);

} // namespace VkMana
