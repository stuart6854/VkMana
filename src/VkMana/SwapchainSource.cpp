#include "VkMana/SwapchainSource.hpp"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

namespace VkMana
{
	class Win32SwapchainSource : public SwapchainSource
	{
	public:
		Win32SwapchainSource(HWND hwnd, HINSTANCE hInstance)
			: m_hwnd(hwnd), m_hInstance(hInstance)
		{
		}

		auto GetHWnd() const -> auto { return m_hwnd; }
		auto GetHInstance() const -> auto { return m_hInstance; }

	private:
		HWND m_hwnd;
		HINSTANCE m_hInstance;
	};

	auto SwapchainSource::CreateWin32(HWND hwnd, HINSTANCE hInstance) -> std::unique_ptr<SwapchainSource>
	{
		return std::make_unique<Win32SwapchainSource>(hwnd, hInstance);
	}

} // namespace VkMana