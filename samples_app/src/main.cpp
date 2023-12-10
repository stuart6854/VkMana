#include "core/App.hpp"
#include "samples/HelloTriangle/HelloTriangle.hpp"
#include "samples/ModelLoading/ModelLoading.hpp"

int main()
{
	using namespace VkMana::SamplesApp;

	SamplesApp app{};

	/* Add Samples */
	app.AddSample<SampleHelloTriangle>();
	app.AddSample<SampleModelLoading>();

	/* Run App */
	app.Run();

	return 0;
}