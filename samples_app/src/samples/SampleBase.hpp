#pragma once

#include <VkMana/Context.hpp>

#include <string>
#include <utility>

namespace VkMana::SamplesApp
{
	class SamplesApp;

	class SampleBase
	{
	public:
		explicit SampleBase(std::string sampleName)
			: m_sampleName(std::move(sampleName))
		{
		}
		virtual ~SampleBase() = default;

		auto GetName() const -> const auto& { return m_sampleName; }

		virtual bool Onload(SamplesApp& app, Context& ctx) = 0;
		virtual void OnUnload() = 0;

		virtual void Tick(float deltaTime, SamplesApp& app, Context& ctx) = 0;

	private:
		std::string m_sampleName;
	};

} // namespace VkMana::SamplesApp