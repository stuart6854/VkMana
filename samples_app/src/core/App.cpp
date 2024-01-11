#include "App.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VkMana::SamplesApp
{
    constexpr auto WindowTitle = "VkMana - Samples App";
    constexpr auto InitialWindowWidth = 1280;
    constexpr auto InitialWindowHeight = 720;

    void SamplesApp::Run()
    {
        Init();

        double lastFrameTime = glfwGetTime();
        while(m_isRunning)
        {
            const auto currentFrameTime = glfwGetTime();
            const auto deltaTime = float(currentFrameTime - lastFrameTime);
            lastFrameTime = currentFrameTime;

            m_window->NewFrame();

            if(!m_window->IsAlive())
            {
                m_isRunning = false;
                break;
            }

            const auto& input = m_window->GetInput();
            if(input.IsKeyUp(Key::One))
                SetSampleIndex(0);
            if(input.IsKeyUp(Key::Two))
                SetSampleIndex(1);
            if(input.IsKeyUp(Key::Three))
                SetSampleIndex(2);
            if(input.IsKeyUp(Key::Four))
                SetSampleIndex(3);
            if(input.IsKeyUp(Key::Five))
                SetSampleIndex(4);
            if(input.IsKeyUp(Key::Six))
                SetSampleIndex(5);
            if(input.IsKeyUp(Key::Seven))
                SetSampleIndex(6);
            if(input.IsKeyUp(Key::Eight))
                SetSampleIndex(7);
            if(input.IsKeyUp(Key::Nine))
                SetSampleIndex(8);
            if(input.IsKeyUp(Key::Zero))
                SetSampleIndex(9);

            m_ctx->BeginFrame();

            const auto& sample = m_samples[m_activeSampleIndex];
            sample->Tick(deltaTime, *this, *m_ctx);

            m_ctx->EndFrame();
            m_swapChain->Present();
        }

        Cleanup();
    }

    void SamplesApp::Init()
    {
        m_isRunning = true;

        if(!glfwInit())
        {
            VM_ERR("Failed to initialise GLFW");
            m_isRunning = false;
            return;
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        auto* glfwWindow = glfwCreateWindow(InitialWindowWidth, InitialWindowHeight, WindowTitle, nullptr, nullptr);
        if(glfwWindow == nullptr)
        {
            VM_ERR("Failed to create GLFW window");
            m_isRunning = false;
            return;
        }

        m_window = std::make_unique<Window>(glfwWindow);

        m_ctx = Context::New();
        if(!m_ctx->Init())
        {
            VM_ERR("Failed to initialise VkMana context.");
            m_isRunning = false;
            return;
        }

        VkSurfaceKHR vkSurface = nullptr;
        if(glfwCreateWindowSurface(m_ctx->GetInstance(), glfwWindow, nullptr, &vkSurface) != VK_SUCCESS)
        {
            VM_ERR("Failed to create Vulkan Surface");
            m_isRunning = false;
            return;
        }

        m_swapChain = m_ctx->CreateSwapChain(vkSurface, InitialWindowWidth, InitialWindowHeight);
        if(m_swapChain == nullptr)
        {
            VM_ERR("Failed to create VkMana SwapChain");
            m_isRunning = false;
            return;
        }

        m_activeSampleIndex = -1; // This is to ensure we load the first sample
        SetSampleIndex(0);

        /*if (!m_samples[m_activeSampleIndex]->Onload(*this, *m_ctx))
        {
                VM_ERR("Failed to load sample: {}", m_samples[m_activeSampleIndex]->GetName());
                m_isRunning = false;
                return;
        }*/
    }

    void SamplesApp::Cleanup()
    {
        m_samples[m_activeSampleIndex]->OnUnload(*this, *m_ctx);
        m_samples.clear();

        m_swapChain = nullptr;
        m_ctx = nullptr;

        m_window = nullptr;
        glfwTerminate();
    }

    void SamplesApp::SetSampleIndex(const int32_t index)
    {
        if(index < 0)
            return;
        if(index >= m_samples.size())
            return;

        if(index == m_activeSampleIndex)
            return;

        const auto lastSampleIndex = m_activeSampleIndex;
        if(m_activeSampleIndex >= 0 && m_activeSampleIndex < m_samples.size())
            m_samples[m_activeSampleIndex]->OnUnload(*this, *m_ctx);

        m_activeSampleIndex = index;

        VM_INFO("Loading sample: {}", m_samples[m_activeSampleIndex]->GetName());
        if(!m_samples[m_activeSampleIndex]->OnLoad(*this, *m_ctx))
        {
            VM_ERR("Failed to load sample: {}", m_samples[index]->GetName());

            m_activeSampleIndex = lastSampleIndex;
            if(m_activeSampleIndex >= 0 && m_activeSampleIndex < m_samples.size())
                m_samples[m_activeSampleIndex]->OnLoad(*this, *m_ctx); // It should be safe to assume the last sample loads okay... Right???
        }

        VM_INFO("Sample loaded: {}", m_samples[m_activeSampleIndex]->GetName());
    }

} // namespace VkMana::SamplesApp