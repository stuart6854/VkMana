#include "App.hpp"

#include <VkMana/Context.hpp>

#define GLFW_INCLUDE_NONE
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
		while (m_isRunning)
		{
			const auto currentFrameTime = glfwGetTime();
			const auto deltaTime = float(currentFrameTime - lastFrameTime);
			lastFrameTime = currentFrameTime;

			if (!m_window->IsAlive())
			{
				m_isRunning = false;
				break;
			}

			m_ctx->BeginFrame();

			const auto& sample = m_samples[m_activeSampleIndex];
			sample->Tick(deltaTime, *this, *m_ctx);

			m_ctx->EndFrame();
			m_ctx->Present();
		}

		Cleanup();
	}

	void SamplesApp::Init()
	{
		m_isRunning = true;

		if (!glfwInit())
		{
			VM_ERR("Failed to initialise GLFW");
			m_isRunning = false;
			return;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		auto* glfwWindow = glfwCreateWindow(InitialWindowWidth, InitialWindowHeight, WindowTitle, nullptr, nullptr);
		if (glfwWindow == nullptr)
		{
			VM_ERR("Failed to create GLFW window");
			m_isRunning = false;
			return;
		}

		m_window = std::make_unique<Window>(glfwWindow);

		m_ctx = IntrusivePtr(new Context);
		if (!m_ctx->Init(m_window.get()))
		{
			VM_ERR("Failed to initialise VkMana context.");
			m_isRunning = false;
			return;
		}

		if (!m_samples[m_activeSampleIndex]->Onload(*this, *m_ctx))
		{
			VM_ERR("Failed to load sample: {}", m_samples[m_activeSampleIndex]->GetName());
			m_isRunning = false;
			return;
		}
	}

	void SamplesApp::Cleanup()
	{
		m_samples[m_activeSampleIndex]->OnUnload();
		m_samples.clear();

		m_ctx = nullptr;

		m_window = nullptr;
		glfwTerminate();
	}

} // namespace VkMana::SamplesApp