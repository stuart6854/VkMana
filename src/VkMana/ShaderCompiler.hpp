#pragma once

#include "Pipeline.hpp"
#include "VulkanCommon.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace VkMana
{
    enum class SourceLanguage : uint8_t
    {
        GLSL,
        HLSL,
    };

    struct ShaderCompileInfo
    {
        SourceLanguage srcLanguage;
        const char* pSrcFilenameStr;
        const char* pSrcStringStr;
        vk::ShaderStageFlagBits stage;
        const char* pEntryPointStr = "main"; // Ignored for GLSL
        bool debug = false;
    };

    bool CompileShader(ShaderByteCode& outSpirv, const std::string& glslSource, vk::ShaderStageFlagBits shaderStage, bool debug, const std::string& filename);

    auto CompileShader(const ShaderCompileInfo& info) -> std::optional<ShaderByteCode>;

} // namespace VkMana
