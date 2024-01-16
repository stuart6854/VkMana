#include "Renderer.hpp"
#include "StaticMesh.hpp"

#include <VkMana/ShaderCompiler.hpp>

#include <fstream>
#include <string>

constexpr auto MaxBindlessImages = 100;

using namespace VkMana;

namespace VkMana::SamplesApp
{
    bool Renderer::Init(Context& ctx, Window& mainWindow)
    {
        m_ctx = &ctx;
        m_mainWindow = &mainWindow;

        { // White image
            const std::vector<uint8_t> WhitePixels = { 255, 255, 255, 255 };
            const auto imageInfo = ImageCreateInfo::Texture(1, 1, 1);
            const ImageDataSource imageDataSrc{ .size = 1 * 1 * 4, .data = WhitePixels.data() };
            m_whiteImage = m_ctx->CreateImage(imageInfo, &imageDataSrc);
            m_whiteImage->SetDebugName("White");
        }
        { // Black image
            const std::vector<uint8_t> BlackPixels = { 0, 0, 0, 255 };
            const auto imageInfo = ImageCreateInfo::Texture(1, 1, 1);
            const ImageDataSource imageDataSrc{ .size = 1 * 1 * 4, .data = BlackPixels.data() };
            m_blackImage = m_ctx->CreateImage(imageInfo, &imageDataSrc);
            m_whiteImage->SetDebugName("Black");
        }

        m_bindlessSetLayout = m_ctx->CreateSetLayout({
            { 0,
             vk::DescriptorType::eCombinedImageSampler,
             MaxBindlessImages, vk::ShaderStageFlagBits::eFragment,
             vk::DescriptorBindingFlagBits::ePartiallyBound },
        });
        // #TODO: m_bindlessSetLayout->SetDebugName("Bindless");

        SetupGBufferPass();
        SetupCompositionPass();
        SetupScreenPass();

        return true;
    }

    void Renderer::SetSceneCamera(const glm::mat4& projMatrix, const glm::mat4& viewMatrix)
    {
        m_cameraUniformData.projMatrix = projMatrix;
        m_cameraUniformData.viewMatrix = viewMatrix;
    }

    void Renderer::Submit(StaticMesh* mesh, glm::mat4 transform)
    {
        m_staticInstances.emplace_back(Instance<StaticMesh>{ mesh, transform });

        for(auto& mat : mesh->GetMaterials())
        {
            GetMaterialIndex(mat.Get());

            for(auto& tex : mat->GetTextures())
            {
                GetImageIndex(tex.Get());
            }
        }
    }

    void Renderer::Flush(SwapChainHandle pSwapChain)
    {
        GetImageIndex(m_whiteImage.Get());
        GetImageIndex(m_blackImage.Get());

        m_bindlessSet = m_ctx->RequestDescriptorSet(m_bindlessSetLayout.Get());
        // #TODO: m_bindlessSet->SetDebugName("Bindless");
        m_bindlessSet->WriteArray(0, 0, m_bindlessImages, m_ctx->GetLinearSampler());

        auto cmd = m_ctx->RequestCmd();
        // #TODO: cmd->SetDebugName("Main");

        cmd->SetViewport(0, float(m_mainWindow->GetSurfaceHeight()), float(m_mainWindow->GetSurfaceWidth()), -float(m_mainWindow->GetSurfaceHeight()));
        cmd->SetScissor(0, 0, m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight());

        GBufferPass(cmd);
        CompositionPass(cmd);
        ScreenPass(cmd, pSwapChain);

        m_ctx->Submit(cmd);

        m_knownBindlessImages.clear();
        m_bindlessImages.clear();
        m_knownMaterials.clear();
        m_materials.clear();
    }

    auto Renderer::CreateMaterial() -> MaterialHandle { return IntrusivePtr(new Material(this)); }

    auto Renderer::CreateStaticMesh() -> StaticMeshHandle { return IntrusivePtr(new StaticMesh(this)); }

    void Renderer::SetupGBufferPass()
    {
        const auto depthImageInfo = ImageCreateInfo::DepthStencilTarget(m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight(), false);
        m_depthTargetImage = m_ctx->CreateImage(depthImageInfo);
        m_depthTargetImage->SetDebugName("Sandbox_DepthTarget");

        const auto positionImageInfo
            = ImageCreateInfo::ColorTarget(m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight(), vk::Format::eR16G16B16A16Sfloat);
        m_positionTargetImage = m_ctx->CreateImage(positionImageInfo);
        m_positionTargetImage->SetDebugName("Sandbox_PositionTarget");

        const auto normalImageInfo
            = ImageCreateInfo::ColorTarget(m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight(), vk::Format::eR16G16B16A16Sfloat);
        m_normalTargetImage = m_ctx->CreateImage(normalImageInfo);
        m_normalTargetImage->SetDebugName("Sandbox_NormalTarget");

        const auto albedoImageInfo
            = ImageCreateInfo::ColorTarget(m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight(), vk::Format::eR8G8B8A8Unorm);
        m_albedoTargetImage = m_ctx->CreateImage(albedoImageInfo);
        m_albedoTargetImage->SetDebugName("Sandbox_AlbedoTarget");

        m_gBufferPass.targets = {
            RenderPassTarget::DefaultColorTarget(m_positionTargetImage->GetImageView(ImageViewType::RenderTarget)),
            RenderPassTarget::DefaultColorTarget(m_normalTargetImage->GetImageView(ImageViewType::RenderTarget)),
            RenderPassTarget::DefaultColorTarget(m_albedoTargetImage->GetImageView(ImageViewType::RenderTarget)),
            RenderPassTarget::DefaultDepthStencilTarget(m_depthTargetImage->GetImageView(ImageViewType::RenderTarget)),
        };

        m_cameraSetLayout = m_ctx->CreateSetLayout({
            { 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
        });
        // #TODO: m_cameraSetLayout->SetDebugName()

        const PipelineLayoutCreateInfo layoutInfo{
            .PushConstantRange = { vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData) },
            .SetLayouts = { m_bindlessSetLayout.Get(), m_cameraSetLayout.Get() },
        };
        m_gBufferPipelineLayout = m_ctx->CreatePipelineLayout(layoutInfo);
        // #TODO: m_cameraSetLayout->SetDebugName()

        ShaderCompileInfo compileInfo{
            .srcLanguage = SourceLanguage::GLSL,
            .pSrcFilenameStr = "assets/shaders/deferred_gbuffer.vert",
            .stage = vk::ShaderStageFlagBits::eVertex,
            .debug = true,
        };
        auto vertexSpirvOpt = CompileShader(compileInfo);
        if(!vertexSpirvOpt)
        {
            VM_ERR("Failed to read/compile Vertex shader.");
            return;
        }

        compileInfo.stage = vk::ShaderStageFlagBits::eFragment;
        compileInfo.pSrcFilenameStr = "assets/shaders/deferred_gbuffer.frag";
        auto fragmentSpirvOpt = CompileShader(compileInfo);
        if(!fragmentSpirvOpt)
        {
            VM_ERR("Failed to read/compile Fragment shader.");
            return;
        }

        const GraphicsPipelineCreateInfo pipelineInfo{
            .vs = { vertexSpirvOpt.value().data(), uint32_t(vertexSpirvOpt.value().size()) },
            .fs = { fragmentSpirvOpt.value().data(), uint32_t(fragmentSpirvOpt.value().size()) },
            .vertexAttributes = {
                    vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertex, Position)),
                    vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32Sfloat, offsetof(StaticVertex, TexCoord)),
                    vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertex, Normal)),
                    vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(StaticVertex, Tangent)),
            },
            .vertexBindings = {
                    vk::VertexInputBindingDescription(0, sizeof(StaticVertex), vk::VertexInputRate::eVertex),
            },
            .primitiveTopology =  vk::PrimitiveTopology::eTriangleList,
            .colorTargetCount = 3,
            .colorFormats = { positionImageInfo.format, normalImageInfo.format, albedoImageInfo.format },
            .depthStencilFormat = depthImageInfo.format,
            .pPipelineLayout = m_gBufferPipelineLayout,
        };
        m_gBufferStaticPipeline = m_ctx->CreateGraphicsPipeline(pipelineInfo);
        m_gBufferStaticPipeline->SetDebugName("Sandbox_Static");

        const auto cameraUniformBufferInfo = BufferCreateInfo::Uniform(sizeof(m_cameraUniformData) * 2);
        m_cameraUniformBuffer = m_ctx->CreateBuffer(cameraUniformBufferInfo);
        m_cameraUniformBuffer->SetDebugName("Sandbox_Camera_Uniforms");
    }

    void Renderer::SetupCompositionPass()
    {
        const auto compositionImageInfo
            = ImageCreateInfo::ColorTarget(m_mainWindow->GetSurfaceWidth(), m_mainWindow->GetSurfaceHeight(), vk::Format::eR8G8B8A8Unorm);
        m_compositionTargetImage = m_ctx->CreateImage(compositionImageInfo);
        m_compositionTargetImage->SetDebugName("Sandbox_CompositeTarget");

        m_compositionPass.targets = {
            RenderPassTarget::DefaultColorTarget(m_compositionTargetImage->GetImageView(ImageViewType::RenderTarget)),
        };

        m_compositionSetLayout = m_ctx->CreateSetLayout({
            { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
            { 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
            { 2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        });

        const PipelineLayoutCreateInfo layoutInfo{
            .SetLayouts = { m_compositionSetLayout.Get() },
        };
        m_compositionPipelineLayout = m_ctx->CreatePipelineLayout(layoutInfo);

        ShaderCompileInfo compileInfo{
            .srcLanguage = SourceLanguage::GLSL,
            .pSrcFilenameStr = "assets/shaders/deferred_composition.vert",
            .stage = vk::ShaderStageFlagBits::eVertex,
            .debug = true,
        };
        auto vertexSpirvOpt = CompileShader(compileInfo);
        if(!vertexSpirvOpt)
        {
            VM_ERR("Failed to read/compile Vertex shader.");
            return;
        }

        compileInfo.stage = vk::ShaderStageFlagBits::eFragment;
        compileInfo.pSrcFilenameStr = "assets/shaders/deferred_composition.frag";
        auto fragmentSpirvOpt = CompileShader(compileInfo);
        if(!fragmentSpirvOpt)
        {
            VM_ERR("Failed to read/compile Fragment shader.");
            return;
        }

        const GraphicsPipelineCreateInfo pipelineInfo{
            .vs = { vertexSpirvOpt.value().data(), uint32_t(vertexSpirvOpt.value().size()) },
            .fs = { fragmentSpirvOpt.value().data(), uint32_t(fragmentSpirvOpt.value().size()) },
            .primitiveTopology = vk::PrimitiveTopology::eTriangleList,
            .colorTargetCount = 1,
            .colorFormats = { compositionImageInfo.format },
            .pPipelineLayout = m_compositionPipelineLayout,
        };
        m_compositionPipeline = m_ctx->CreateGraphicsPipeline(pipelineInfo);
        m_compositionPipeline->SetDebugName("Sandbox_Composition");
    }

    void Renderer::SetupScreenPass()
    {
        m_screenSetLayout = m_ctx->CreateSetLayout({
            { 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
        });

        const PipelineLayoutCreateInfo layoutInfo{
            .SetLayouts = { m_screenSetLayout.Get() },
        };
        m_screenPipelineLayout = m_ctx->CreatePipelineLayout(layoutInfo);

        ShaderCompileInfo compileInfo{
            .srcLanguage = SourceLanguage::GLSL,
            .pSrcFilenameStr = "assets/shaders/fullscreen_quad.vert",
            .stage = vk::ShaderStageFlagBits::eVertex,
            .debug = true,
        };
        auto vertexSpirvOpt = CompileShader(compileInfo);
        if(!vertexSpirvOpt)
        {
            VM_ERR("Failed to read/compile Vertex shader.");
            return;
        }

        compileInfo.stage = vk::ShaderStageFlagBits::eFragment;
        compileInfo.pSrcFilenameStr = "assets/shaders/fullscreen_quad.frag";
        auto fragmentSpirvOpt = CompileShader(compileInfo);
        if(!fragmentSpirvOpt)
        {
            VM_ERR("Failed to read/compile Fragment shader.");
            return;
        }

        const GraphicsPipelineCreateInfo pipelineInfo{
            .vs = { vertexSpirvOpt.value().data(), uint32_t(vertexSpirvOpt.value().size()) },
            .fs = { fragmentSpirvOpt.value().data(), uint32_t(fragmentSpirvOpt.value().size()) },
            .primitiveTopology = vk::PrimitiveTopology::eTriangleList,
            .colorTargetCount = 1,
            .colorFormats = { vk::Format::eB8G8R8A8Srgb },
            .pPipelineLayout = m_screenPipelineLayout,
        };
        m_screenPipeline = m_ctx->CreateGraphicsPipeline(pipelineInfo);
        m_screenPipeline->SetDebugName("Sandbox_Screen");
    }

    void Renderer::GBufferPass(CmdBuffer& cmd)
    {
        auto* dataPtr = m_cameraUniformBuffer->Map() + (sizeof(CameraUniformData) * m_ctx->GetFrameIndex());
        std::memcpy(dataPtr, &m_cameraUniformData, sizeof(CameraUniformData));
        m_cameraUniformBuffer->Unmap();

        auto cameraSet = m_ctx->RequestDescriptorSet(m_cameraSetLayout.Get());
        cameraSet->Write(
            0,
            m_cameraUniformBuffer.Get(),
            sizeof(CameraUniformData) * m_ctx->GetFrameIndex(),
            sizeof(CameraUniformData),
            vk::DescriptorType::eUniformBuffer
        );

        cmd->BeginRenderPass(m_gBufferPass);

        cmd->BindPipeline(m_gBufferStaticPipeline.Get());
        cmd->BindDescriptorSets(0, { m_bindlessSet.Get(), cameraSet.Get() }, {});

        for(const auto& instance : m_staticInstances)
        {
            m_pushConstantData.modelMatrix = instance.Transform;

            cmd->BindIndexBuffer(instance.Mesh->GetIndexBuffer(), 0);
            cmd->BindVertexBuffers(0, { instance.Mesh->GetVertexBuffer() }, { 0 });

            auto& materials = instance.Mesh->GetMaterials();
            for(auto& submesh : instance.Mesh->GetSubmeshes())
            {
                auto& material = materials[submesh.MaterialIndex];
                auto* albedoImage = material->GetAlbedoTexture();
                auto* normalImage = material->GetNormalTexture();

                m_pushConstantData.albedoMapIndex = GetImageIndex(albedoImage ? albedoImage : m_whiteImage.Get());
                m_pushConstantData.normalMapIndex = GetImageIndex(normalImage ? normalImage : m_blackImage.Get());
                cmd->SetPushConstants(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0, sizeof(PushConstantData), &m_pushConstantData);

                cmd->DrawIndexed(submesh.IndexCount, submesh.IndexOffset, submesh.VertexOffset);
            }
        }

        cmd->EndRenderPass();

        m_staticInstances.clear();
    }

    void Renderer::CompositionPass(CmdBuffer& cmd)
    {
        auto compositionSet = m_ctx->RequestDescriptorSet(m_compositionSetLayout.Get());
        compositionSet->Write(m_positionTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 0);
        compositionSet->Write(m_normalTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 1);
        compositionSet->Write(m_albedoTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 2);

        cmd->BeginRenderPass(m_compositionPass);
        cmd->BindPipeline(m_compositionPipeline.Get());
        cmd->BindDescriptorSets(0, { compositionSet.Get() }, {});
        cmd->Draw(3, 0);
        cmd->EndRenderPass();
    }

    void Renderer::ScreenPass(CmdBuffer& cmd, SwapChainHandle pSwapChain)
    {
        auto screenTextureSet = m_ctx->RequestDescriptorSet(m_screenSetLayout.Get());
        screenTextureSet->Write(m_compositionTargetImage->GetImageView(ImageViewType::Texture), m_ctx->GetLinearSampler(), 0);

        cmd->BeginRenderPass(pSwapChain->GetRenderPass());
        cmd->BindPipeline(m_screenPipeline.Get());
        cmd->BindDescriptorSets(0, { screenTextureSet.Get() }, {});
        cmd->Draw(3, 0);
        cmd->EndRenderPass();
    }

    auto Renderer::GetImageIndex(Image* image) -> uint32_t
    {
        const auto it = m_knownBindlessImages.find(image);
        if(it != m_knownBindlessImages.end())
        {
            return it->second;
        }

        const auto index = m_bindlessImages.size();
        m_bindlessImages.push_back(image->GetImageView(ImageViewType::Texture));
        m_knownBindlessImages[image] = index;
        return index;
    }

    auto Renderer::GetMaterialIndex(const Material* material) -> uint32_t
    {
        const auto it = m_knownMaterials.find(material);
        if(it != m_knownMaterials.end())
        {
            return it->second;
        }

        const auto index = m_materials.size();
        m_materials.push_back(material);
        m_knownMaterials[material] = index;
        return index;
    }

} // namespace VkMana::SamplesApp
