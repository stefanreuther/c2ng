/**
  *  \file ui/widgets/basedecimalselector.cpp
  */

#include "ui/widgets/basedecimalselector.hpp"
#include "util/updater.hpp"

ui::widgets::BaseDecimalSelector::BaseDecimalSelector(afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step)
    : NumberSelector(value, min, max, step),
      m_mode(Normal),
      m_pPeer(0)
{
    // ex UIDecimalSelector::UIDecimalSelector
    value.sig_change.add(this, &BaseDecimalSelector::onChange);
}

ui::widgets::BaseDecimalSelector::~BaseDecimalSelector()
{ }

void
ui::widgets::BaseDecimalSelector::setMode(Mode m)
{
    // ex UIDecimalSelector::wState, UIDecimalSelector::setDisplayState
    if (util::Updater().set(m_mode, m)) {
        requestRedraw();
    }
}

ui::widgets::BaseDecimalSelector::Mode
ui::widgets::BaseDecimalSelector::getMode() const
{
    return m_mode;
}

void
ui::widgets::BaseDecimalSelector::setPeer(Peer& peer)
{
    m_pPeer = &peer;
}

ui::widgets::BaseDecimalSelector::Peer*
ui::widgets::BaseDecimalSelector::getPeer() const
{
    return m_pPeer;
}

void
ui::widgets::BaseDecimalSelector::handleStateChange(State st, bool enable)
{
    // ex UIDecimalSelector::onStateChange
    if (st == FocusedState) {
        setMode(enable ? TypeErase : Normal);
        requestRedraw();
    }
}

void
ui::widgets::BaseDecimalSelector::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

bool
ui::widgets::BaseDecimalSelector::handleKey(util::Key_t key, int prefix)
{
    // Note that setMode() must always follow setValue(), to override a mode change triggered by onChange().
    // Changes triggered from the outside (e.g. value().set(), increment()) will always set Normal mode.
    if (hasState(FocusedState)) {
        if (m_pPeer != 0 && m_pPeer->handleKey(*this, key, prefix)) {
            // Peer handled it
            return true;
        } else if (key >= '0' && key <= '9') {
            // Digit: type
            requestActive();
            int32_t n = key - '0';
            if (m_mode == Normal) {
                n += 10*getValue();
            }
            if (getMin() <= n && n <= getMax()) {
                setValue(n);
                setMode(Normal);
            }
            return true;
        } else if (key == util::Key_Backspace) {
            // Backspace: delete last digit
            // Note that this implementation allows you to zero a BaseDecimalSelector whose minimum value is not 0.
            // The internal value will be 1 (or whatever the minimum is), but be shown as empty;
            // typing will behave correctly.
            requestActive();
            int newValue = getValue() / 10;
            setValue(newValue);
            if (newValue == 0) {
                setMode(Zeroed);
            } else {
                setMode(Normal);
            }
            return true;
        } else if (key == util::Key_Delete) {
            // Del: clear if selected
            requestActive();
            if (m_mode == TypeErase) {
                setValue(0);
                setMode(Zeroed);
            }
            return true;
        } else if (key == util::KeyMod_Ctrl + util::Key_Delete || key == util::KeyMod_Ctrl + 'y') {
            // Ctrl-Del, Ctrl-Y: clear
            requestActive();
            setValue(0);
            setMode(Zeroed);
            return true;
        } else if (key == util::KeyMod_Ctrl + 'u') {
            // Ctrl-U: unselect
            requestActive();
            setMode(Normal);
            return true;
        } else if (key == util::KeyMod_Ctrl + 't') {
            // Ctrl-T: Swap last two digits
            // Because we cannot have leading zeroes, this will also turn 2 into 20 and vice versa.
            requestActive();
            int32_t n = getValue();
            setValue(100*(n/100)
                     + 10*(n%10)
                     + (n/10)%10);
            setMode(Normal);
            return true;
        } else {
            return defaultHandleKey(key, prefix);
        }
    } else {
        return false;
    }
}

bool
ui::widgets::BaseDecimalSelector::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestFocus();
        return true;
    } else {
        return false;
    }
}

void
ui::widgets::BaseDecimalSelector::onChange()
{
    setMode(Normal);
}
