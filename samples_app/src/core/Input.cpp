#include "Input.hpp"

namespace VkMana::SamplesApp
{
    void Input::NewFrame() { m_lastFrame = m_thisFrame; }

    bool Input::IsKeyDown(Key key) const { return !m_lastFrame.Keys.contains(key) && m_thisFrame.Keys.contains(key); }

    bool Input::IsKeyUp(Key key) const { return m_lastFrame.Keys.contains(key) && !m_thisFrame.Keys.contains(key); }

    bool Input::IsKeyHeld(Key key) const { return m_lastFrame.Keys.contains(key) && m_thisFrame.Keys.contains(key); }

    bool Input::IsMouseBtnDown(MouseBtn btn) const { return !m_lastFrame.MouseBtns.contains(btn) && m_thisFrame.MouseBtns.contains(btn); }

    bool Input::IsMouseBtnUp(MouseBtn btn) const { return m_lastFrame.MouseBtns.contains(btn) && !m_thisFrame.MouseBtns.contains(btn); }

    bool Input::IsMouseBtnHeld(MouseBtn btn) const { return m_lastFrame.MouseBtns.contains(btn) && m_thisFrame.MouseBtns.contains(btn); }

    auto Input::GetScrollDelta() const -> glm::ivec2 { return m_thisFrame.ScrollWheel; }

    auto Input::GetCursorPos() const -> glm::ivec2 { return m_thisFrame.CursorPos; }

    auto Input::GetCursorDelta() const -> glm::vec2 { return glm::vec2(m_lastFrame.CursorPos) - glm::vec2(m_thisFrame.CursorPos); }

    void Input::SetKeyState(Key key, bool isDown)
    {
        if(isDown)
            m_thisFrame.Keys.insert(key);
        else
            m_thisFrame.Keys.erase(key);
    }

    void Input::SetMouseBtnState(MouseBtn btn, bool isDown)
    {
        if(isDown)
            m_thisFrame.MouseBtns.insert(btn);
        else
            m_thisFrame.MouseBtns.erase(btn);
    }

    void Input::SetMouseScrollState(glm::vec2 value) { m_thisFrame.ScrollWheel = value; }

    void Input::SetCursorPos(glm::ivec2 pos) { m_thisFrame.CursorPos = pos; }

} // namespace VkMana::SamplesApp
