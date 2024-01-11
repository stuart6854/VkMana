#pragma once

#include "Window.hpp"

#include "samples/SampleBase.hpp"

#include <VkMana/Context.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VkMana::SamplesApp
{
    class SamplesApp
    {
    public:
        SamplesApp() = default;
        ~SamplesApp() = default;

        template <typename T>
        void AddSample();

        void Run();

        auto GetWindow() const -> auto& { return *m_window; }
        auto GetSwapChain() -> auto& { return m_swapChain; }

    private:
        void Init();
        void Cleanup();

        void SetSampleIndex(int32_t index);

    private:
        bool m_isRunning = false;
        std::unique_ptr<Window> m_window = nullptr;
        ContextHandle m_ctx = nullptr;
        SwapChainHandle m_swapChain = nullptr;

        std::vector<std::unique_ptr<SampleBase>> m_samples;
        int32_t m_activeSampleIndex = -1;
    };

    template <typename T>
    void SamplesApp::AddSample()
    {
        m_samples.push_back(std::make_unique<T>());
    }

} // namespace VkMana::SamplesApp