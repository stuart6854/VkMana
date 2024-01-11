#include "ShaderCompiler.hpp"

#include <shaderc/shaderc.hpp>

#ifdef _WIN32
    #include <wrl/client.h>
    #define CComPtr Microsoft::WRL::ComPtr
#endif

#ifdef __linux__
    #include <dxc/WinAdapter.h>
#endif

#include <dxc/dxcapi.h>

#include <fstream>
#include <sstream>

namespace VkMana
{
    namespace
    {
        auto ReadFileStr(const std::filesystem::path& filename) -> std::optional<std::string>
        {
            std::ifstream stream(filename);
            if(!stream)
                return std::nullopt;

            std::stringstream ss;
            ss << stream.rdbuf();
            return ss.str();
        }

        auto GetShaderKind(const vk::ShaderStageFlagBits shaderStage) -> shaderc_shader_kind
        {
            switch(shaderStage)
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

        auto SelectHLSLTargetProfile(vk::ShaderStageFlagBits shaderStage) -> std::wstring
        {
            switch(shaderStage)
            {
            case vk::ShaderStageFlagBits::eVertex:
                return L"vs_6_1";
            case vk::ShaderStageFlagBits::eTessellationControl:
                return L"hs_6_1";
            case vk::ShaderStageFlagBits::eTessellationEvaluation:
                return L"ds_6_1";
            case vk::ShaderStageFlagBits::eGeometry:
                return L"gs_6_1";
            case vk::ShaderStageFlagBits::eFragment:
                return L"ps_6_1";
            case vk::ShaderStageFlagBits::eCompute:
                return L"cs_6_1";
            default:
                VM_ERR("Unknown shader kind.");
                assert(false);
                return {};
            }
        }

    } // namespace

    bool CompileShader(ShaderBinary& outSpirv, const std::string& glslSource, vk::ShaderStageFlagBits shaderStage, bool debug, const std::string& filename)
    {
        assert(!filename.empty());

        shaderc::CompileOptions options;
        if(debug)
            options.SetGenerateDebugInfo();
        else
            options.SetOptimizationLevel(shaderc_optimization_level_performance);

        auto kind = GetShaderKind(shaderStage);
        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(glslSource, kind, filename.c_str(), options);
        if(result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            VM_ERR("Shader Compile Error: \n{}", result.GetErrorMessage());
            return false;
        }

        outSpirv.assign(result.cbegin(), result.cend());
        return true;
    }

    auto CompileGLSL(const ShaderCompileInfo& info) -> std::optional<ShaderBinary>
    {
        shaderc::CompileOptions options;
        if(info.Debug)
            options.SetGenerateDebugInfo();
        else
            options.SetOptimizationLevel(shaderc_optimization_level_performance);

        const char* sourceFile = "_no_file_";
        if(!info.SrcFilename.empty())
            sourceFile = info.SrcFilename.string().c_str();

        auto kind = GetShaderKind(info.Stage);
        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(info.SrcString, kind, sourceFile, options);
        if(result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            VM_ERR("Shader Compile Error:\n{}", result.GetErrorMessage());
            return std::nullopt;
        }

        ShaderBinary spirv;
        spirv.assign(result.cbegin(), result.cend());
        return spirv;
    }

    auto CompileHLSL(const ShaderCompileInfo& info) -> std::optional<ShaderBinary>
    {
        HRESULT hres;
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wlanguage-extension-token"
#endif
        CComPtr<IDxcLibrary> library;
        hres = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
        if(FAILED(hres))
        {
            VM_ERR("Failed to initialise DXC library.");
            return std::nullopt;
        }

        CComPtr<IDxcCompiler3> compiler;
        hres = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        if(FAILED(hres))
        {
            VM_ERR("Failed to inititalise DXC compiler.");
            return std::nullopt;
        }

        CComPtr<IDxcUtils> utils;
        hres = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
        if(FAILED(hres))
        {
            VM_ERR("Failed to inititalise DXC Utility.");
            return std::nullopt;
        }
#ifdef __clang__
    #pragma clang diagnostic push
#endif

        uint32_t codePage = DXC_CP_ACP;
        CComPtr<IDxcBlobEncoding> sourceBlob;
        hres = utils->CreateBlob(info.SrcString.data(), uint32_t(info.SrcString.size()), codePage, &sourceBlob);
        if(FAILED(hres))
        {
            VM_ERR("Could not create shader source blob.");
            return std::nullopt;
        }

        auto targetProfile = SelectHLSLTargetProfile(info.Stage);

        auto tmpStr = info.SrcFilename.string();
        auto srcFilename = std::wstring(tmpStr.begin(), tmpStr.end());
        auto entryPoint = std::wstring(info.EntryPoint.begin(), info.EntryPoint.end());

        // Compiler args
        std::vector<LPCWSTR> arguments{
            srcFilename.c_str(), // (Optional) Name of the shader file to be displayed e.g in error messages
            L"-E",               // Shader main entry point
            entryPoint.c_str(),
            L"-T", // Shader target profile
            targetProfile.c_str(),
            L"-spirv" // Compile to SPIRV
        };

        DxcBuffer buffer{};
        buffer.Encoding = DXC_CP_ACP;
        buffer.Ptr = sourceBlob->GetBufferPointer();
        buffer.Size = sourceBlob->GetBufferSize();

        CComPtr<IDxcResult> result{ nullptr };
        hres = compiler->Compile(&buffer, arguments.data(), uint32_t(arguments.size()), nullptr, IID_PPV_ARGS(&result));
        if(SUCCEEDED(hres))
        {
            result->GetStatus(&hres);
        }

        if(FAILED(hres) && result)
        {
            CComPtr<IDxcBlobEncoding> errorBlob;
            hres = result->GetErrorBuffer(&errorBlob);
            if(SUCCEEDED(hres) && errorBlob)
            {
                VM_ERR("Shader Compilation Failed:\n");
                VM_ERR("{}", static_cast<const char*>(errorBlob->GetBufferPointer()));
                return std::nullopt;
            }
        }

        CComPtr<IDxcBlob> code;
        result->GetResult(&code);

        auto* p = reinterpret_cast<uint32_t*>(code->GetBufferPointer());
        auto n = code->GetBufferSize() / sizeof(uint32_t);
        ShaderBinary spirv;
        spirv.reserve(n);
        std::copy_n(p, n, std::back_inserter(spirv));
        return spirv;
    }

    auto CompileShader(ShaderCompileInfo info) -> std::optional<ShaderBinary>
    {
        if(!info.SrcFilename.empty())
        {
            auto opt = ReadFileStr(info.SrcFilename);
            if(opt)
                info.SrcString = opt.value();
            else
            {
                VM_ERR("Failed to read Shader source from file: {}", info.SrcFilename.string());
            }
        }
        if(info.SrcString.empty())
        {
            VM_ERR("No shader source to compile.");
            return std::nullopt;
        }

        switch(info.SrcLanguage)
        {
        case SourceLanguage::GLSL:
            return CompileGLSL(info);
        case SourceLanguage::HLSL:
            return CompileHLSL(info);
        }
        VM_ERR("Unknown Shader SourceLanguage.");
        assert(false);
        return std::nullopt;
    }

} // namespace VkMana