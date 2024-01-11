#include "core/App.hpp"
#include "samples/HelloTriangle/HelloTriangle.hpp"
#include "samples/ModelLoading/ModelLoading.hpp"
#include "samples/Sandbox/Sandbox.hpp"

int main()
{
    using namespace VkMana::SamplesApp;

    SamplesApp app{};

    /* Add Samples */
    app.AddSample<SampleHelloTriangle>();
    app.AddSample<SampleModelLoading>();

    app.AddSample<SampleSandbox>();

    /* Run App */
    app.Run();

    return 0;
}