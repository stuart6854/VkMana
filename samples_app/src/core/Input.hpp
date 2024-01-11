#pragma once

#include "InputCodes.hpp"

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_int2.hpp>

#include <unordered_set>

namespace VkMana::SamplesApp
{
    class Window;

    class Input
    {
    public:
        ~Input() = default;

        void NewFrame();

        bool IsKeyDown(Key key) const;
        bool IsKeyUp(Key key) const;
        bool IsKeyHeld(Key key) const;

        bool IsMouseBtnDown(MouseBtn btn) const;
        bool IsMouseBtnUp(MouseBtn btn) const;
        bool IsMouseBtnHeld(MouseBtn btn) const;

        auto GetScrollDelta() const -> glm::ivec2;

        auto GetCursorPos() const -> glm::ivec2;
        auto GetCursorDelta() const -> glm::vec2;

    private:
        friend class Window;
        Input() = default;

        void SetKeyState(Key key, bool isDown);
        void SetMouseBtnState(MouseBtn btn, bool isDown);
        void SetMouseScrollState(glm::vec2 value);
        void SetCursorPos(glm::ivec2 pos);

        struct Frame
        {
            std::unordered_set<Key> Keys{};
            std::unordered_set<MouseBtn> MouseBtns{};
            glm::vec2 ScrollWheel{};
            glm::ivec2 CursorPos{};
        };
        Frame m_thisFrame{};
        Frame m_lastFrame{};
    };

} // namespace VkMana::SamplesApp
