/**
  *  \file client/widgets/techbar.cpp
  */

#include "client/widgets/techbar.hpp"
#include "gfx/complex.hpp"
#include "afl/string/format.hpp"
#include "gfx/context.hpp"
#include "ui/draw.hpp"

// @change PCC2's tech bar has its own focus frame. This one doesn't. Use FocusableGroup.

client::widgets::TechBar::TechBar(ui::Root& root, afl::base::Observable<int32_t>& value, int32_t low, int32_t high, String_t name)
    : NumberSelector(value, low, high, 1),
      m_root(root),
      m_name(name)
{
    // ex WTechBar::WTechBar
}

client::widgets::TechBar::~TechBar()
{ }

// SimpleWidget:
void
client::widgets::TechBar::draw(gfx::Canvas& can)
{
    static const int dif[7] = { -9, -6, -3, 0, +4, +7, +10 };

    // Metrics
    const int PAD = 5;
    gfx::Rectangle r = getBarPosition();
    gfx::Rectangle x = getExtent();
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest());
    int numberWidth = 2 * font->getEmWidth();
    int textWidth = x.getWidth() - r.getWidth() - numberWidth - PAD;

    // Name
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.useFont(*font);
    ctx.setTextAlign(0, 1);
    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, x.splitX(textWidth), m_name);

    // Value
    ctx.setTextAlign(2, 1);
    outTextF(ctx, x.splitX(numberWidth), afl::string::Format("%d", getValue()));

    // Tech bar
    gfx::Context<uint8_t> uctx(can, m_root.colorScheme());
    ui::drawFrameDown(uctx, r);
    r.grow(-1, -1);

    int xt = r.getWidth() * getMin() / 10;
    int xs = r.getWidth() * getValue() / 10;

    if (getMin() == getValue()) {
        drawSolidBar(uctx, gfx::Rectangle(r.getLeftX(), r.getTopY(), xt, r.getHeight()), ui::Color_GreenBlack);
    } else {
        drawSolidBar(uctx, gfx::Rectangle(r.getLeftX(), r.getTopY(), xt-9, r.getHeight()), ui::Color_GreenBlack);
        for (int i = 0; i < 6; ++i) {
            drawSolidBar(uctx, gfx::Rectangle(r.getLeftX() + xt + dif[i], r.getTopY(), dif[i+1]-dif[i], r.getHeight()), uint8_t(ui::Color_GreenScale6 + i));
        }
        drawSolidBar(uctx, gfx::Rectangle(r.getLeftX() + xt + 10, r.getTopY(), xs - xt - 10,  r.getHeight()), ui::Color_Green);
    }
    if (xs < r.getWidth()) {
        drawSolidBar(uctx, gfx::Rectangle(r.getLeftX() + xs, r.getTopY(), r.getWidth() - xs, r.getHeight()), ui::Color_Black);
    }

    int xlim = r.getWidth() * getMax() / 10;
    if (xlim < r.getWidth()) {
        can.drawBar(gfx::Rectangle(r.getLeftX() + xlim, r.getTopY(), r.getWidth() - xlim, r.getHeight()),
                    m_root.colorScheme().getColor(ui::Color_Gray),
                    gfx::TRANSPARENT_COLOR,
                    gfx::FillPattern::LTSLASH,
                    gfx::OPAQUE_ALPHA);
    }
}

void
client::widgets::TechBar::handleStateChange(State /*st*/, bool /*enable*/)
{
    // ex WTechBar::onStateChange
}

void
client::widgets::TechBar::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    requestRedraw();
}

ui::layout::Info
client::widgets::TechBar::getLayoutInfo() const
{
    // ex WTechBar::getLayoutInfo
    // FIXME: this is a direct port. Make it smarter.
    return gfx::Point(2*20*10, 20);
}

bool
client::widgets::TechBar::handleKey(util::Key_t key, int prefix)
{
    // ex WTechBar::handleEvent
    if (hasState(FocusedState) && key >= '0' && key <= '9') {
        requestActive();
        setValue((key == '0') ? 10 : key - '0');
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}

bool
client::widgets::TechBar::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex WTechBar::handleEvent
    if (!pressedButtons.empty() && getExtent().contains(pt)) {
        requestActive();
        requestFocus();

        gfx::Rectangle barPos = getBarPosition();
        if (barPos.contains(pt)) {
            setValue(1 + (pt.getX() - barPos.getLeftX()) * 10 / barPos.getWidth());
        }
        return true;
    } else {
        return defaultHandleMouse(pt, pressedButtons);
    }
}

gfx::Rectangle
client::widgets::TechBar::getBarPosition() const
{
    // ex WTechBar::getBarPosition
    gfx::Rectangle r = getExtent();
    r.consumeX(r.getWidth() / 2);
    return r;
}
