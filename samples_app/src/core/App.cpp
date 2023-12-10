#include "App.hpp"

#include <VkMana/Context.hpp>

namespace VkMana::SamplesApp
{
	constexpr auto WindowTitle = "VkMana - Samples App";
	constexpr auto InitialWindowWidth = 1280;
	constexpr auto InitialWindowHeight = 720;

	void SamplesApp::Run()
	{
		Init();

		double lastFrameTime = double(SDL_GetTicks()) / 1000.0f;
		while (m_isRunning)
		{
			const auto currentFrameTime = double(SDL_GetTicks()) / 1000.0f;
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

		auto* sdlWindow = SDL_CreateWindow(WindowTitle,
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			InitialWindowWidth,
			InitialWindowHeight,
			SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		if (!sdlWindow)
		{
			VM_ERR("Failed to create SDL window");
			m_isRunning = false;
			return;
		}

		m_window = std::make_unique<Window>(sdlWindow);

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
	}

} // namespace VkMana::SamplesApp