#include "Sandbox.hpp"

#include "core/App.hpp"
#include "core/Vertex.hpp"

namespace VkMana::SamplesApp
{
    bool SampleSandbox::Onload(SamplesApp& app, Context& ctx)
    {
        auto& window = app.GetWindow();

        m_renderer = IntrusivePtr(new Renderer);
        if(!m_renderer->Init(ctx, app.GetWindow()))
        {
            VM_ERR("Failed to initialise renderer");
            return false;
        }

        m_staticMesh = m_renderer->CreateStaticMesh();
        if(!m_staticMesh->LoadFromFile("assets/models/viking_room.obj"))
        {
            VM_ERR("Failed to load mesh");
            return false;
        }

        if(!LoadGLTFModel(*m_staticMesh, *m_renderer, "assets/models/runestone/scene.gltf"))
        // if (!LoadGLTFModel(*staticMesh, renderer, "assets/models/submesh_test/scene.gltf"))
        {
            VM_ERR("Failed to load GLTF model");
            return false;
        }

        return true;
    }

    void SampleSandbox::OnUnload(SamplesApp& app, Context& ctx)
    {
        m_staticMesh = nullptr;
        m_renderer = nullptr;
    }

    void SampleSandbox::Tick(float deltaTime, SamplesApp& app, Context& ctx)
    {
        const auto windowWidth = app.GetSwapChain()->GetBackBufferWidth();
        const auto windowHeight = app.GetSwapChain()->GetBackBufferHeight();
        const auto windowAspect = float(windowWidth) / float(windowHeight);

        const auto projMat = glm::perspectiveLH_ZO(glm::radians(60.0f), windowAspect, 0.1f, 500.0f);
        constexpr glm::vec3 cameraPos = { -3, 3, -4 };
        constexpr glm::vec3 cameraTarget = { 0, 0.5f, 0 };
        const auto viewMat = glm::lookAtLH(cameraPos, cameraTarget, glm::vec3(0, 1, 0));

        m_renderer->SetSceneCamera(projMat, viewMat);

        glm::mat4 transformMat = glm::mat4(1.0f);
        transformMat = glm::scale(transformMat, glm::vec3(0.5f));
        m_renderer->Submit(m_staticMesh.Get(), transformMat);

        m_renderer->Flush(app.GetSwapChain());
    }

} // namespace VkMana::SamplesApp