/**
  *  \file ui/widgets/basebutton.cpp
  *  \brief Class ui::widgets::BaseButton
  */

#include "ui/widgets/basebutton.hpp"
#include "gfx/context.hpp"

// Constructor.
ui::widgets::BaseButton::BaseButton(ui::Root& root, util::Key_t key)
    : SimpleWidget(),
      sig_fire(),
      sig_fireKey(),
      m_root(root),
      m_key(key),
      m_activeModifiers(0),
      m_flags(),
      m_focusable(false),
      m_growthBehaviour(ui::layout::Info::Fixed),
      m_pIcon(0)
{ }

void
ui::widgets::BaseButton::setIcon(ui::icons::Icon& icon)
{
    m_pIcon = &icon;
}

// Set or clear a button flag.
void
ui::widgets::BaseButton::setFlag(ButtonFlag flag, bool value)
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
ui::widgets::BaseButton::getFlags() const
{
    ButtonFlags_t result = m_flags;
    result.set(DisabledButton, hasState(DisabledState));
    result.set(FocusedButton,  hasState(FocusedState));    // FIXME: or check PrimaryFocus?
    return result;
}

void
ui::widgets::BaseButton::draw(gfx::Canvas& can)
{
    if (m_pIcon != 0) {
        gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
        m_pIcon->draw(ctx, getExtent(), getFlags());
    }
}

ui::layout::Info
ui::widgets::BaseButton::getLayoutInfo() const
{
    const gfx::Point size = m_pIcon != 0
        ? m_pIcon->getSize()
        : gfx::Point();
    return ui::layout::Info(size, size, m_growthBehaviour);
}

// Implementation for handleStateChange.
void
ui::widgets::BaseButton::handleStateChange(State st, bool enable)
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

void
ui::widgets::BaseButton::handlePositionChange()
{
    requestRedraw();
}

// Implementation for handleKey.
bool
ui::widgets::BaseButton::handleKey(util::Key_t key, int prefix)
{
    if (m_focusable && !hasState(DisabledState) && hasState(FocusedState) && key == ' ') {
        requestActive();
        fire(prefix, key);
        return true;
    } else if (!hasState(DisabledState) && m_key != 0) {
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

// Implementation for handleMouse.
bool
ui::widgets::BaseButton::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (getExtent().contains(pt) && !hasState(DisabledState)) {
        // Mouse is in widget
        requestActive();
        if (!pressedButtons.empty()) {
            // Mouse pressed in button
            if (m_focusable) {
                requestFocus();
            }
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

// Dispatch key activation to another widget.
void
ui::widgets::BaseButton::dispatchKeyTo(gfx::KeyEventConsumer& target)
{
    class Handler : public afl::base::Closure<void(int, util::Key_t)> {
     public:
        Handler(gfx::KeyEventConsumer& target)
            : m_target(target)
            { }
        void call(int prefix, util::Key_t key)
            { m_target.handleKey(key, prefix); }
        Handler* clone() const
            { return new Handler(*this); }
     private:
        gfx::KeyEventConsumer& m_target;
    };
    sig_fireKey.addNewClosure(new Handler(target));
}

// Dispatch key activation to widget, and focus it.
void
ui::widgets::BaseButton::dispatchKeyAndFocus(Widget& target)
{
    class Handler : public afl::base::Closure<void(int, util::Key_t)> {
     public:
        Handler(Widget& target)
            : m_target(target)
            { }
        void call(int prefix, util::Key_t key)
            {
                m_target.requestFocus();
                m_target.handleKey(key, prefix);
            }
        Handler* clone() const
            { return new Handler(*this); }
     private:
        Widget& m_target;
    };
    sig_fireKey.addNewClosure(new Handler(target));
}

// Get associated key.
util::Key_t
ui::widgets::BaseButton::getKey() const
{
    return m_key;
}

// Set associated key.
void
ui::widgets::BaseButton::setKey(util::Key_t key)
{
    m_key = key;
}

// Make this button focusable.
void
ui::widgets::BaseButton::setIsFocusable(bool flag)
{
    m_focusable = flag;
}

// Set growth behaviour.
void
ui::widgets::BaseButton::setGrowthBehaviour(ui::layout::Info::Growth growth)
{
    m_growthBehaviour = growth;
}

// Get associated root.
ui::Root&
ui::widgets::BaseButton::root() const
{
    return m_root;
}

void
ui::widgets::BaseButton::fire(int arg, util::Key_t key)
{
    // ex UIBaseButton::fire
    if (m_focusable) {
        requestFocus();
    }
    sig_fire.raise(arg);
    if ((key & util::Key_Mask) != 0) {
        sig_fireKey.raise(arg, key);
    }
}
