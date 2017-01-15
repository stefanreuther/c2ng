/**
  *  \file ui/widgets/abstractbutton.cpp
  *  \brief Class ui::widgets::AbstractButton
  */

#include "ui/widgets/abstractbutton.hpp"

// Constructor.
ui::widgets::AbstractButton::AbstractButton(ui::Root& root, util::Key_t key)
    : SimpleWidget(),
      sig_fire(),
      sig_fireKey(),
      m_root(root),
      m_key(key),
      m_activeModifiers(0),
      m_flags()
{ }

// Set or clear a button flag.
void
ui::widgets::AbstractButton::setFlag(ButtonFlag flag, bool value)
{
    bool doit = false;
    if (value) {
        if (!m_flags.contains(flag)) {
            m_flags += flag;
            doit = true;
        }
    } else {
        if (m_flags.contains(flag)) {
            m_flags -= flag;
            doit = true;
        }
    }
    if (doit) {
        requestRedraw();
    }
}

// Get current flags.
ui::ButtonFlags_t
ui::widgets::AbstractButton::getFlags() const
{
    return m_flags;
}

// Default implementation for handleStateChange.
void
ui::widgets::AbstractButton::defaultHandleStateChange(State st, bool enable)
{
    if (st == ActiveState) {
        setFlag(ActiveButton, enable);
        if (!enable) {
            // We lost logical focus: un-press the button
            setFlag(PressedButton, enable);
        }
    }
    requestRedraw();
}

// Default implementation for handleKey.
bool
ui::widgets::AbstractButton::defaultHandleKey(util::Key_t key, int prefix)
{
    if (!hasState(DisabledState)) {
        // FIXME: && !hasState(bf_Key)
        // FIXME: PCC2 checks Alt and #/\ only on second pass
        if (key == m_key
            || key == util::KeyMod_Alt + m_key
            || (key == '\\' && m_key == '#'))
        {
            requestActive();
            fire(prefix, key);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

// Default implementation for handleMouse.
bool
ui::widgets::AbstractButton::defaultHandleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (getExtent().contains(pt) && !hasState(DisabledState)) {
        // Mouse is in widget
        requestActive();
        if (!pressedButtons.empty()) {
            // Mouse pressed in button
            setFlag(PressedButton, true);
            m_activeModifiers = 0;
            if (pressedButtons.contains(ShiftKey)) {
                m_activeModifiers |= util::KeyMod_Shift;
            }
            if (pressedButtons.contains(CtrlKey)) {
                m_activeModifiers |= util::KeyMod_Ctrl;
            }
            if (pressedButtons.contains(AltKey)) {
                m_activeModifiers |= util::KeyMod_Alt;
            }
            if (pressedButtons.contains(MetaKey)) {
                m_activeModifiers |= util::KeyMod_Meta;
            }
        } else if (m_flags.contains(PressedButton)) {
            // Mouse released in button
            setFlag(PressedButton, false);
            fire(m_root.consumeMousePrefixArgument(), m_key | m_activeModifiers);
            m_activeModifiers = 0;
        } else {
            // Mouse still down
        }
        return true;
    } else {
        dropActive();

        // Let others handle the event, too.
        return false;
    }
}

// Get associated key.
util::Key_t
ui::widgets::AbstractButton::getKey() const
{
    return m_key;
}

// Get associated root.
ui::Root&
ui::widgets::AbstractButton::root() const
{
    return m_root;
}

void
ui::widgets::AbstractButton::fire(int arg, util::Key_t key)
{
    // ex UIAbstractButton::fire
    sig_fire.raise(arg);
    sig_fireKey.raise(arg, key);
}
