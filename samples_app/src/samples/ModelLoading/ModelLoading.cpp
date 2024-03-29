#include "ModelLoading.hpp"

#include "core/App.hpp"
#include "core/Vertex.hpp"

#include <VkMana/ShaderCompiler.hpp>

#include <tiny_obj_loader.h>

#include <stb_image.h>

#include <string>
#include <vector>

const std::string VertexShaderSrc = R"(
#version 450

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

layout(location = 0) out vec2 outTexCoord;

layout(push_constant) uniform PushConstants
{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} uConsts;

void main()
{
	gl_Position = uConsts.projMatrix * uConsts.viewMatrix * uConsts.modelMatrix * vec4(aPosition, 1.0f);
	outTexCoord = aTexCoord;
}
)";
const std::string FragmentShaderSrc = R"(
#version 450

layout (location = 0) in vec2 inTexCoord;

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D uTexture;

void main()
{
	// outFragColor = vec4(inTexCoord, 0, 1);
	outFragColor = texture(uTexture, inTexCoord);
}
)";

namespace VkMana::SamplesApp
{
    bool SampleModelLoading::OnLoad(SamplesApp& app, Context& ctx)
    {
        auto& window = app.GetWindow();

        auto depthImageInfo = VkMana::ImageCreateInfo::DepthStencilTarget(window.GetSurfaceWidth(), window.GetSurfaceHeight(), false);
        m_depthTarget = ctx.CreateImage(depthImageInfo, nullptr);

        std::vector<VkMana::SetLayoutBinding> setBindings{
            { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        };
        m_setLayout = ctx.CreateSetLayout(setBindings);
        // #TODO: m_setLayout->SetDebugName("ModelLoading")

        VkMana::PipelineLayoutCreateInfo pipelineLayoutInfo{
			.PushConstantRange = { vk::ShaderStageFlagBits::eVertex, 0, 192 },
			.SetLayouts = {
				m_setLayout.Get(),
		   },
	   };
        m_pipelineLayout = ctx.CreatePipelineLayout(pipelineLayoutInfo);
        // #TODO: m_pipelineLayout->SetDebugName("ModelLoading")

        ShaderCompileInfo compileInfo{
            .srcLanguage = SourceLanguage::GLSL,
            .pSrcFilenameStr = "",
            .pSrcStringStr = VertexShaderSrc.c_str(),
            .stage = vk::ShaderStageFlagBits::eVertex,
            .debug = true,
        };
        const auto vertSpirvOpt = CompileShader(compileInfo);
        if(!vertSpirvOpt)
        {
            VM_ERR("Failed to compiler VERTEX shader.");
            return false;
        }

        compileInfo.pSrcStringStr = FragmentShaderSrc.c_str();
        compileInfo.stage = vk::ShaderStageFlagBits::eFragment;
        const auto fragSpirvOpt = CompileShader(compileInfo);
        if(!fragSpirvOpt)
        {
            VM_ERR("Failed to compiler FRAGMENT shader.");
            return false;
        }

        const VkMana::GraphicsPipelineCreateInfo pipelineInfo{
            .vs = { vertSpirvOpt.value().data(), uint32_t(vertSpirvOpt.value().size()) },
            .fs = { fragSpirvOpt.value().data(), uint32_t(fragSpirvOpt.value().size()) },
            .vertexAttributes = {
                    vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Position)),
                    vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal)),
                    vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord)),
            },
            .vertexBindings = {
                    vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex),
            },
            .primitiveTopology = vk::PrimitiveTopology::eTriangleList,
            .colorTargetCount = 1,
            .colorFormats = { vk::Format::eB8G8R8A8Srgb },
            .depthStencilFormat = vk::Format::eD24UnormS8Uint,
            .pPipelineLayout = m_pipelineLayout,
        };
        m_pipeline = ctx.CreateGraphicsPipeline(pipelineInfo);
        if(m_pipeline == nullptr)
            return false;

        m_pipeline->SetDebugName("ModelLoading");

        if(!LoadObjMesh(m_mesh, ctx, "assets/models/viking_room.obj"))
        {
            VM_ERR("Failed to load mesh.");
            return false;
        }
        m_mesh.IndexBuffer->SetDebugName("VikingRoom_Index");
        m_mesh.VertexBuffer->SetDebugName("VikingRoom_Vertex");

        if(!LoadTexture(m_texture, ctx, "assets/models/viking_room.png"))
        {
            VM_ERR("Failed to load texture.");
            return false;
        }
        m_texture->SetDebugName("VikingRoom");

        m_pushConsts.viewMatrix = glm::lookAtLH(glm::vec3(-1, 0.5f, -1), glm::vec3(0, -0.2f, 0), glm::vec3(0, 1, 0));
        m_pushConsts.modelMatrix = glm::mat4(1.0f);

        return true;
    }

    void SampleModelLoading::OnUnload(SamplesApp& app, Context& ctx)
    {
        m_mesh = {};
        m_texture = nullptr;
        m_pipeline = nullptr;
        m_pipelineLayout = nullptr;
        m_setLayout = nullptr;
        m_depthTarget = nullptr;
    }

    void SampleModelLoading::Tick(float deltaTime, SamplesApp& app, Context& ctx)
    {
        auto& window = app.GetWindow();
        const auto windowWidth = window.GetSurfaceWidth();
        const auto windowHeight = window.GetSurfaceHeight();
        const auto windowAspect = float(windowWidth) / float(windowHeight);

        m_pushConsts.projMatrix = glm::perspectiveLH_ZO(glm::radians(60.0f), windowAspect, 0.1f, 600.0f);

        auto cmd = ctx.RequestCmd();
        // #TODO: cmd->SetDebugName("Main")

        auto textureSet = ctx.RequestDescriptorSet(m_setLayout.Get());
        // #TODO: textureSet->SetDebugName("ModelLoading_Texture")
        textureSet->Write(m_texture->GetImageView(VkMana::ImageViewType::Texture), ctx.GetLinearSampler(), 0);
        const auto rpDepthTarget = VkMana::RenderPassTarget::DefaultDepthStencilTarget(m_depthTarget->GetImageView(VkMana::ImageViewType::RenderTarget));
        auto rpInfo = app.GetSwapChain()->GetRenderPass();
        rpInfo.targets.push_back(rpDepthTarget);

        cmd->BeginRenderPass(rpInfo);
        cmd->BindPipeline(m_pipeline.Get());
        cmd->SetViewport(0, float(windowHeight), float(windowWidth), -float(windowHeight));
        cmd->SetScissor(0, 0, windowWidth, windowHeight);
        cmd->BindDescriptorSets(0, { textureSet.Get() }, {});
        cmd->SetPushConstants(vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants), &m_pushConsts);
        cmd->BindIndexBuffer(m_mesh.IndexBuffer.Get());
        cmd->BindVertexBuffers(0, { m_mesh.VertexBuffer.Get() }, { 0 });
        cmd->DrawIndexed(m_mesh.IndexCount, 0, 0);
        cmd->EndRenderPass();

        ctx.Submit(cmd);
    }

    bool SampleModelLoading::LoadObjMesh(Mesh& outMesh, VkMana::Context& ctx, const std::string& filename)
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

        if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str()))
        {
            VM_ERR("{}", err);
            if(!warn.empty())
                VM_WARN("{}", warn);
            return false;
        }

        std::unordered_map<Vertex, uint16_t> uniqueVertices{};

        for(const auto& shape : shapes)
        {
            for(const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
                vertex.Normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
                if(!uniqueVertices.contains(vertex))
                {
                    uniqueVertices[vertex] = uint16_t(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        VkMana::BufferDataSource vtxDataSrc{
            .size = sizeof(Vertex) * vertices.size(),
            .pData = vertices.data(),
        };
        auto vtxBufferInfo = VkMana::BufferCreateInfo::Vertex(vtxDataSrc.size);
        outMesh.VertexBuffer = ctx.CreateBuffer(vtxBufferInfo, &vtxDataSrc);

        VkMana::BufferDataSource idxDataSrc{
            .size = sizeof(uint16_t) * indices.size(),
            .pData = indices.data(),
        };
        auto idxBufferInfo = VkMana::BufferCreateInfo::Index(idxDataSrc.size);
        outMesh.IndexBuffer = ctx.CreateBuffer(idxBufferInfo, &idxDataSrc);

        outMesh.IndexCount = indices.size();

        return true;
    }

    bool SampleModelLoading::LoadTexture(VkMana::ImageHandle& outImage, VkMana::Context& ctx, const std::string& filename)
    {
        int32_t width;
        int32_t height;
        int32_t comps;
        stbi_set_flip_vertically_on_load(true);
        auto* pixels = stbi_load(filename.c_str(), &width, &height, &comps, 4);
        if(!pixels)
        {
            VM_ERR("Failed to load image: {}", filename);
            return false;
        }

        auto imageInfo = VkMana::ImageCreateInfo::Texture(width, height);
        VkMana::ImageDataSource dataSource{
            .size = uint32_t(width * height * 4),
            .data = pixels,
        };
        outImage = ctx.CreateImage(imageInfo, &dataSource);

        stbi_image_free(pixels);
        return true;
    }

} // namespace VkMana::SamplesApp