#pragma once

#include "Pipeline.hpp"
#include "Vulkan_Common.hpp"

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
        const char* srcFilename;
        const char* srcString;
        vk::ShaderStageFlagBits stage;
        const char* entryPoint = "main"; // Ignored for GLSL
        bool debug = false;
    };

    bool CompileShader(ShaderByteCode& outSpirv, const std::string& glslSource, vk::ShaderStageFlagBits shaderStage, bool debug, const std::string& filename);

    auto CompileShader(const ShaderCompileInfo& info) -> std::optional<ShaderByteCode>;

} // namespace VkMana
