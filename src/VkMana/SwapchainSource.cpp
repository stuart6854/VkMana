#include "VkMana/SwapchainSource.hpp"

#include "Impl/Win32SwapchainSource.hpp"

namespace VkMana
{
	auto SwapchainSource::CreateWin32(HWND hwnd, HINSTANCE hInstance) -> std::unique_ptr<SwapchainSource>
	{
		return std::make_unique<Win32SwapchainSource>(hwnd, hInstance);
	}

} // namespace VkMana