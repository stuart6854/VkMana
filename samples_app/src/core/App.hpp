#pragma once

#include "Window.hpp"
#include "samples/SampleBase.hpp"

#include <VkMana/Context.hpp>

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

		auto GetWindow() const -> auto* { return m_window.get(); }

	private:
		void Init();
		void Cleanup();

	private:
		bool m_isRunning = false;
		std::unique_ptr<Window> m_window = nullptr;
		ContextHandle m_ctx = nullptr;

		std::vector<std::unique_ptr<SampleBase>> m_samples;
		uint32_t m_activeSampleIndex = 0;
	};

	template <typename T>
	void SamplesApp::AddSample()
	{
		m_samples.push_back(std::make_unique<T>());
	}

} // namespace VkMana::SamplesApp