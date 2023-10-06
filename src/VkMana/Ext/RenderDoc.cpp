#include "VkMana/Ext/RenderDoc.hpp"

#include <RenderDoc/renderdoc_app.h>

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
	#include <ShlObj_core.h>
#endif

#include <string>

namespace VkMana::Ext
{
#ifdef _WIN32
	HMODULE gModule = nullptr;
#endif
	RENDERDOC_API_1_6_0* gRDocApi = nullptr;

	bool RenderDocInit()
	{
#ifdef _WIN32

		TCHAR pf[MAX_PATH];
		SHGetSpecialFolderPathA(nullptr, pf, CSIDL_PROGRAM_FILES, MAX_PATH);
		auto dllPath = std::string(pf) + "\\RenderDoc\\renderdoc.dll";
		gModule = LoadLibraryA(dllPath.c_str());
		if (gModule != nullptr)
		{
			auto renderDocGetApi = pRENDERDOC_GetAPI(GetProcAddress(gModule, "RENDERDOC_GetAPI"));
			int ret = renderDocGetApi(eRENDERDOC_API_Version_1_6_0, (void**)&gRDocApi);
			if (ret == 1)
				return true;
		}
#endif

		return false;
	}

	bool RenderDocDeinit()
	{
		gRDocApi = nullptr;
#ifdef _WIN32
		FreeLibrary(gModule);
#endif
		return true;
	}

	void RenderDocStartFrameCapture()
	{
		if (gRDocApi)
			gRDocApi->StartFrameCapture(nullptr, nullptr);
	}

	void RenderDocEndFrameCapture()
	{
		if (gRDocApi)
			gRDocApi->EndFrameCapture(nullptr, nullptr);
	}

	void RenderDocLaunchReplayUI()
	{
		if (gRDocApi)
			gRDocApi->LaunchReplayUI(1, nullptr);
	}

} // namespace VkMana::Ext