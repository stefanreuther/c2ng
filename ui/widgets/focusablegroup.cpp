/**
  *  \file ui/widgets/focusablegroup.cpp
  *  \brief Class ui::widgets::FocusableGroup
  */

#include "ui/widgets/focusablegroup.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "ui/layout/hbox.hpp"

const int ui::widgets::FocusableGroup::DEFAULT_PAD;

ui::widgets::FocusableGroup::FocusableGroup(ui::layout::Manager& mgr, int pad)
    : LayoutableGroup(mgr),
      m_pad(pad)
{
    // ex UIFocusGroup::UIFocusGroup
}

ui::widgets::FocusableGroup::~FocusableGroup()
{ }

// LayoutableGroup:
gfx::Rectangle
ui::widgets::FocusableGroup::transformSize(gfx::Rectangle size, Transformation kind) const
{
    // ex UIFocusGroup::adjustSize
    switch (kind) {
     case OuterToInner:
        size.grow(-m_pad, -m_pad);
        break;
     case InnerToOuter:
        size.grow(m_pad, m_pad);
        break;
    }
    return size;
}

// Widget:
void
ui::widgets::FocusableGroup::draw(gfx::Canvas& can)
{
    // ex UIFocusGroup::drawContent, UIFocusGroup::drawEverything
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    const gfx::Rectangle& r = getExtent();
    if (getFocusState() == NoFocus) {
        gfx::drawBackground(ctx, gfx::Rectangle(r.getLeftX(),    r.getTopY(),      r.getWidth(), 1));
        gfx::drawBackground(ctx, gfx::Rectangle(r.getLeftX(),    r.getBottomY()-1, r.getWidth(), 1));
        gfx::drawBackground(ctx, gfx::Rectangle(r.getLeftX(),    r.getTopY()+1,    1,            r.getHeight()-2));
        gfx::drawBackground(ctx, gfx::Rectangle(r.getRightX()-1, r.getTopY()+1,    1,            r.getHeight()-2));
    } else {
        ctx.setColor(util::SkinColor::Contrast);
        gfx::drawRectangle(ctx, r);
    }

    defaultDrawChildren(can);

    if (hasState(DisabledState)) {
        gfx::Color_t color = getColorScheme().getColor(util::SkinColor::Background);
        if (can.getBitsPerPixel() < 16) {
            can.drawBar(r, color, gfx::TRANSPARENT_COLOR, gfx::FillPattern::GRAY50, gfx::OPAQUE_ALPHA);
        } else {
            can.drawBar(r, color, gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, 192);
        }
    }
}

void
ui::widgets::FocusableGroup::handleStateChange(State st, bool /*enable*/)
{
    // ex UIFocusGroup::onStateChange
    if (st == FocusedState || st == DisabledState) {
        requestRedraw();
    }
}

// EventConsumer:
bool
ui::widgets::FocusableGroup::handleKey(util::Key_t key, int prefix)
{
    // ex UIFocusGroup::handleEvent
    if (hasState(FocusedState) && !hasState(DisabledState)) {
        return defaultHandleKey(key, prefix);
    } else {
        return false;
    }
}

bool
ui::widgets::FocusableGroup::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UIFocusGroup::handleEvent
    if (hasState(DisabledState)) {
        return false;
    } else {
        if (!hasState(FocusedState) && !pressedButtons.empty() && getExtent().contains(pt)) {
            requestActive();
            requestFocus();
        }
        return defaultHandleMouse(pt, pressedButtons);
    }
}

ui::widgets::FocusableGroup&
ui::widgets::FocusableGroup::wrapWidget(afl::base::Deleter& del, int pad, Widget& widget)
{
    FocusableGroup& g = del.addNew(new FocusableGroup(ui::layout::HBox::instance0, pad));
    g.add(widget);
    return g;
}

ui::widgets::FocusableGroup&
ui::widgets::FocusableGroup::wrapWidget(afl::base::Deleter& del, Widget& widget)
{
    return wrapWidget(del, DEFAULT_PAD, widget);
}
