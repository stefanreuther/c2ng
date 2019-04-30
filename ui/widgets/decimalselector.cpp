/**
  *  \file ui/widgets/decimalselector.cpp
  *  \brief Class ui::widgets::DecimalSelector
  */

#include "ui/widgets/decimalselector.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/translation.hpp"
#include "util/updater.hpp"

ui::widgets::DecimalSelector::DecimalSelector(Root& root, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step)
    : NumberSelector(value, min, max, step),
      m_root(root),
      m_mode(Normal),
      m_flags(),
      m_pPeer(0)
{
    // ex UIDecimalSelector::UIDecimalSelector
    value.sig_change.add(this, &DecimalSelector::onChange);
}

ui::widgets::DecimalSelector::~DecimalSelector()
{ }

void
ui::widgets::DecimalSelector::setFlag(Flag flag, bool enable)
{
    m_flags.set(flag, enable);
}

void
ui::widgets::DecimalSelector::setMode(Mode m)
{
    // ex UIDecimalSelector::wState, UIDecimalSelector::setDisplayState
    if (util::Updater().set(m_mode, m)) {
        requestRedraw();
    }
}

void
ui::widgets::DecimalSelector::setPeer(Peer& peer)
{
    m_pPeer = &peer;
}

void
ui::widgets::DecimalSelector::draw(gfx::Canvas& can)
{
    // ex UIDecimalSelector::drawContent
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    ctx.useFont(*font);

    String_t post, val;

    if (getFocusState() != NoFocus) {
        post = "_";              // FIXME: replace by proper cursor
    }

    if (m_flags.contains(ShowMaximum)) {
        // FIXME: translator, NumberFormatter
        post += afl::string::Format(_(" (max. %d)"), /*numToString*/(getMax()));
    }

    if (m_mode != Zeroed) {
        if (m_pPeer != 0) {
            val = m_pPeer->toString(*this, getValue());
        } else {
            // FIXME: NumberFormatter
            val = afl::string::Format("%d", getValue());
        }
    }

    int val_width  = font->getTextWidth(val);
    int post_width = font->getTextWidth(post);

    val_width = std::min(val_width, getExtent().getWidth());
    post_width = std::min(post_width, getExtent().getWidth() - val_width);

    int remain = getExtent().getWidth() - post_width - val_width;

    /* Left-justify:  | val_width | post_width | suffix |
       Right-justify: | prefix | val_width | post_width | */

    int pre_width, suf_width;
    if (m_flags.contains(RightJustified)) {
        pre_width = remain;
        suf_width = 0;
    } else {
        pre_width = 0;
        suf_width = remain;
    }

    // Draw it
    const int x = getExtent().getLeftX(), y = getExtent().getTopY(), h = getExtent().getHeight();
    if (m_mode == TypeErase) {
        // Focused and type-erase
        drawBackground(ctx, gfx::Rectangle(x, y, pre_width, h));
        drawBackground(ctx, gfx::Rectangle(x + pre_width + val_width, y, suf_width + post_width, h));
        drawSolidBar(ctx, gfx::Rectangle(x + pre_width, y, val_width, h), util::SkinColor::Input);
        ctx.setColor(util::SkinColor::Background);
    } else {
        // Not type-erase, thus everything on regular background
        drawBackground(ctx, getExtent());
        ctx.setColor(getFocusState() != NoFocus ? util::SkinColor::Input : util::SkinColor::Static);
    }

    outTextF(ctx, gfx::Point(x + pre_width, y), val_width, val);

    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, gfx::Point(x + pre_width + val_width, y), post_width, post);
}

void
ui::widgets::DecimalSelector::handleStateChange(State st, bool enable)
{
    // ex UIDecimalSelector::onStateChange
    if (st == FocusedState) {
        setMode(enable ? TypeErase : Normal);
        requestRedraw();
    }
}

void
ui::widgets::DecimalSelector::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::widgets::DecimalSelector::getLayoutInfo() const
{
    // ex UIDecimalSelector::getLayoutInfo
    int ems = (m_flags.contains(ShowMaximum) ? 10 : 5);
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest().addSize(1))->getCellSize().scaledBy(ems, 1);

    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}

bool
ui::widgets::DecimalSelector::handleKey(util::Key_t key, int prefix)
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
            // Note that this implementation allows you to zero a DecimalSelector whose minimum value is not 0.
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
ui::widgets::DecimalSelector::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestFocus();
        return true;
    } else {
        return false;
    }
}

void
ui::widgets::DecimalSelector::onChange()
{
    setMode(Normal);
}
