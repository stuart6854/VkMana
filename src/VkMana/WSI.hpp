#pragma once

#include "Vulkan_Common.hpp"

namespace VkMana
{
    class WSI
    {
    public:
        virtual ~WSI() = default;

        virtual void PollEvents() = 0;

        virtual auto CreateSurface(vk::Instance instance) -> vk::SurfaceKHR = 0;
        virtual auto GetSurfaceWidth() -> uint32_t = 0;
        virtual auto GetSurfaceHeight() -> uint32_t = 0;
        virtual bool IsVSync() = 0;
        virtual bool IsAlive() = 0;

        virtual void HideCursor() = 0;
        virtual void ShowCursor() = 0;

        virtual auto CreateCursor(uint32_t cursorType) -> void* = 0;
        virtual void SetCursor(void* cursor) = 0;
    };
} // namespace VkMana