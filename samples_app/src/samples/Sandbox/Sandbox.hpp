#pragma once

#include "Renderer.hpp"
#include "ModelLoader.hpp"

#include "samples/SampleBase.hpp"

namespace VkMana::SamplesApp
{
	class SampleSandbox final : public SampleBase
	{
	public:
		explicit SampleSandbox()
			: SampleBase("Sandbox")
		{
		}
		~SampleSandbox() override = default;

		bool Onload(SamplesApp& app, Context& ctx) override;
		void OnUnload(SamplesApp& app, Context& ctx) override;

		void Tick(float deltaTime, SamplesApp& app, Context& ctx) override;

	private:
		IntrusivePtr<Renderer> m_renderer = nullptr;

		StaticMeshHandle m_staticMesh = nullptr;
	};

} // namespace VkMana::SamplesApp
