#pragma once

#include "VkMana/SwapchainSource.hpp"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#endif

namespace VkMana
{
	class Win32SwapchainSource : public SwapchainSource
	{
	public:
		Win32SwapchainSource(HWND hwnd, HINSTANCE hInstance)
			: m_hwnd(hwnd)
			, m_hInstance(hInstance)
		{
		}

		auto GetHWnd() const -> auto { return m_hwnd; }
		auto GetHInstance() const -> auto { return m_hInstance; }

	private:
		HWND m_hwnd;
		HINSTANCE m_hInstance;
	};
} // namespace VkMana