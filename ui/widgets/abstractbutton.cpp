/**
  *  \file ui/widgets/abstractbutton.cpp
  */

#include "ui/widgets/abstractbutton.hpp"

ui::widgets::AbstractButton::AbstractButton(util::Key_t key)
    : SimpleWidget(),
      sig_fire(),
      sig_fireKey(),
      m_key(key),
      m_activeModifiers(0),
      m_flags()
{ }

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

ui::ButtonFlags_t
ui::widgets::AbstractButton::getFlags() const
{
    return m_flags;
}


bool
ui::widgets::AbstractButton::handleKey(util::Key_t key, int prefix)
{
    if (!hasState(DisabledState)) {
        // FIXME: && !hasState(bf_Key)
        // FIXME: PCC2 checks Alt and #/\ only on second pass
        if (key == m_key
            || key == util::KeyMod_Alt + m_key
            || (key == '\\' && key == '#'))
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

bool
ui::widgets::AbstractButton::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
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

            // FIXME: prefix argument
            // int arg = consumePrefixArg();
            int arg = 0;
            fire(arg, m_key | m_activeModifiers);
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

// /** Reaction on button click. */
void
ui::widgets::AbstractButton::fire(int arg, util::Key_t key)
{
    // ex UIAbstractButton::fire
    sig_fire.raise(arg);
    sig_fireKey.raise(arg, key);
}

// /** Make this a "key" button. */
// UIAbstractButton&
// UIAbstractButton::addKey()
// {
//     setState(bf_Key, true);
//     return *this;
// }

// /** Change button state. When the button loses logical focus, it must
//     be un-pressed. Certain events also need redraw. */
// void
// UIAbstractButton::onStateChange(int astate, bool enable)
// {
//     UIWidget::onStateChange(astate, enable);

//     // FIXME: can we get rid of this status manipulation?
//     if (astate & st_Disabled)
//         status &= ~bf_Pressed;

//     if ((status & (bf_Pressed | st_Selected)) == bf_Pressed)
//         status &= ~bf_Pressed;

//     if (astate & (bf_Pressed | st_Selected | bf_Highlight | st_Disabled | bf_PressOverride))
//         drawWidget(false);
// }

// bool
// UIAbstractButton::handleCommand(const UICommand& cmd, UIBaseWidget& sender)
// {
//     if (cmd.code == cm_INT_QueryReferencePoint) {
//         // FIXME: do we need this? This is the default setting.
//         ASSERT(cmd.infop);
//         TReferencePointArg& ra = *static_cast<TReferencePointArg*>(cmd.infop);
//         ra.x = getExtent().x;
//         ra.y = getExtent().getBottomY();
//         return true;
//     } else if (cmd.code == cm_INT_Push) {
//         setState(bf_PressOverride, true);
//         return true;
//     } else if (cmd.code == cm_INT_Unpush) {
//         setState(bf_PressOverride, false);
//         return true;
//     } else {
//         return UIWidget::handleCommand(cmd, sender);
//     }
// }
