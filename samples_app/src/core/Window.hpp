#pragma once

#include "Input.hpp"

#include <VkMana/WSI.hpp>

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace VkMana::SamplesApp
{
    class Window : public WSI
    {
    public:
        explicit Window(GLFWwindow* window)
            : m_window(window)
        {
            glfwSetWindowUserPointer(m_window, this);

            SetupCallbacks();
        }
        ~Window() override = default;

        void NewFrame()
        {
            m_input.NewFrame();
            PollEvents();
        }

        void PollEvents() override { glfwPollEvents(); }

        auto CreateSurface(vk::Instance instance) -> vk::SurfaceKHR override
        {
            VkSurfaceKHR surface = nullptr;
            if(glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS)
                return nullptr;

            return surface;
        }

        /*auto GetInstanceExtension() -> std::vector<const char*> override
        {
                uint32_t extCount = 0;
                SDL_Vulkan_GetInstanceExtensions(Window, &extCount, nullptr);
                std::vector<const char*> exts(extCount);
                SDL_Vulkan_GetInstanceExtensions(Window, &extCount, exts.data());
                return exts;
        }*/

        auto GetSurfaceWidth() -> uint32_t override
        {
            int32_t w = 0;
            int32_t h = 0;
            glfwGetFramebufferSize(m_window, &w, &h);
            return w;
        }
        auto GetSurfaceHeight() -> uint32_t override
        {
            int32_t w = 0;
            int32_t h = 0;
            glfwGetFramebufferSize(m_window, &w, &h);
            return h;
        }

        bool IsVSync() override { return true; }
        bool IsAlive() override { return !glfwWindowShouldClose(m_window); }

        void HideCursor() override { }
        void ShowCursor() override { }

        auto CreateCursor(uint32_t cursorType) -> void* override { return nullptr; }
        void SetCursor(void* cursor) override { }

        auto GetInput() -> auto& { return m_input; }

    private:
        void SetupCallbacks() const;

    private:
        GLFWwindow* m_window;
        bool m_isAlive = true;

        Input m_input;
    };

    inline void Window::SetupCallbacks() const
    {
        glfwSetKeyCallback(
            m_window,
            [](GLFWwindow* window, int key, int scancode, int action, int mods)
            {
                auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

                if(key < 0 || key > GLFW_KEY_LAST)
                    return;

                if(action != GLFW_PRESS && action != GLFW_RELEASE)
                    return;

                myWindow->GetInput().SetKeyState(Key(key), action != GLFW_RELEASE);
            }
        );
        glfwSetMouseButtonCallback(
            m_window,
            [](GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
            {
                auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

                if(button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
                    return;

                if(action != GLFW_PRESS && action != GLFW_RELEASE)
                    return;

                myWindow->GetInput().SetMouseBtnState(MouseBtn(button), action != GLFW_RELEASE);
            }
        );

        glfwSetScrollCallback(
            m_window,
            [](GLFWwindow* window, double x, double y)
            {
                auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

                myWindow->GetInput().SetMouseScrollState({ float(x), float(y) });
            }
        );

        glfwSetCursorPosCallback(
            m_window,
            [](GLFWwindow* window, double x, double y)
            {
                auto* myWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));

                myWindow->GetInput().SetCursorPos({ int32_t(x), int32_t(y) });
            }
        );
    }

} // namespace VkMana::SamplesApp