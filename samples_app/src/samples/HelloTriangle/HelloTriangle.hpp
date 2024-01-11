#pragma once

#include <VkMana/Pipeline.hpp>

#include "samples/SampleBase.hpp"

namespace VkMana::SamplesApp
{
    class SampleHelloTriangle final : public SampleBase
    {
    public:
        explicit SampleHelloTriangle()
            : SampleBase("Hello Triangle")
        {
        }
        ~SampleHelloTriangle() override = default;

        bool OnLoad(SamplesApp& app, Context& ctx) override;
        void OnUnload(SamplesApp& app, Context& ctx) override;

        void Tick(float deltaTime, SamplesApp& app, Context& ctx) override;

    private:
        PipelineHandle m_pipeline = nullptr;
    };

} // namespace VkMana::SamplesApp
