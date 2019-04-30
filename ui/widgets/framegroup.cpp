/**
  *  \file ui/widgets/framegroup.cpp
  *  \brief Class ui::widgets::FrameGroup
  */

#include "ui/widgets/framegroup.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "afl/base/optional.hpp"
#include "ui/layout/hbox.hpp"

// Constructor.
ui::widgets::FrameGroup::FrameGroup(ui::layout::Manager& mgr,
                                    ColorScheme& colors,
                                    Type type)
    : LayoutableGroup(mgr),
      m_colors(colors),
      m_frameType(type),
      m_frameWidth(1),
      m_padding(0)
{ }

// Set frame width.
void
ui::widgets::FrameGroup::setFrameWidth(int size)
{
    m_frameWidth = size;
    requestRedraw();
}

// Set padding.
void
ui::widgets::FrameGroup::setPadding(int size)
{
    m_padding = size;
    requestRedraw();
}

// Set type (color).
void
ui::widgets::FrameGroup::setType(Type type)
{
    // ex WColorFrame::setColor
    if (m_frameType != type) {
        m_frameType = type;
        requestRedraw();
    }
}

// Get type (color).
ui::widgets::FrameGroup::Type
ui::widgets::FrameGroup::getType() const
{
    return m_frameType;
}

// Wrap a single widget within a FrameGroup.
ui::widgets::FrameGroup&
ui::widgets::FrameGroup::wrapWidget(afl::base::Deleter& del, ColorScheme& colors, Type type, Widget& widget)
{
    FrameGroup& f = del.addNew(new FrameGroup(ui::layout::HBox::instance0, colors, type));
    f.add(widget);
    return f;
}

gfx::Rectangle
ui::widgets::FrameGroup::transformSize(gfx::Rectangle size, Transformation kind) const
{
    // ex UIFrameGroup::adjustSize, WColorFrame::adjustSize
    int delta = m_padding + m_frameWidth;
    switch (kind) {
     case OuterToInner:
        size.grow(-delta, -delta);
        break;
     case InnerToOuter:
        size.grow(delta, delta);
        break;
    }
    return size;
}

// Widget:
void
ui::widgets::FrameGroup::draw(gfx::Canvas& can)
{
    // ex UIFrameGroup::drawContent, WColorFrame::drawContent
    // Determine colors
    afl::base::Optional<uint8_t> leftOuter, leftInner, rightOuter, rightInner;
    switch (m_frameType) {
     case NoFrame:
        // Draw nothing
        break;

     case RedFrame:
        leftOuter = rightOuter = Color_Fire + 6;
        leftInner = rightInner = Color_Fire + 8;
        break;

     case YellowFrame:
        leftOuter = rightOuter = Color_DarkYellow;
        leftInner = rightInner = Color_BrightYellow;
        break;

     case GreenFrame:
        leftOuter = rightOuter = Color_GreenScale + 8;
        leftInner = rightInner = Color_GreenScale + 10;
        break;

     case RaisedFrame:
        leftOuter = leftInner = Color_White;
        rightOuter = rightInner = Color_Black;
        break;

     case LoweredFrame:
        leftOuter = leftInner = Color_Black;
        rightOuter = rightInner = Color_White;
        break;
    }

    // Determine widths
    // These formulas make a 1px frame use the outer color and evenly split a 2px frame.
    int innerWidth = m_frameWidth / 2;
    int outerWidth = m_frameWidth - innerWidth;

    // Draw
    gfx::Context<uint8_t> ctx(can, m_colors);
    gfx::Rectangle r(getExtent());
    if (outerWidth > 0 && outerWidth < r.getWidth() && outerWidth < r.getHeight()) {
        uint8_t color;
        if (leftOuter.get(color)) {
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY(),              r.getWidth() - outerWidth, outerWidth),                 color);
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY() + outerWidth, outerWidth,                r.getHeight() - outerWidth), color);
        }
        if (rightOuter.get(color)) {
            drawSolidBar(ctx, gfx::Rectangle(r.getRightX() - outerWidth, r.getTopY(),                 outerWidth,                r.getHeight() - outerWidth), color);
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX() + outerWidth,  r.getBottomY() - outerWidth, r.getWidth() - outerWidth, outerWidth),                 color);
        }
    }
    r.grow(-outerWidth, -outerWidth);
    if (innerWidth > 0 && innerWidth < r.getWidth() && innerWidth < r.getHeight()) {
        uint8_t color;
        if (leftInner.get(color)) {
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY(), r.getWidth() - innerWidth, innerWidth), color);
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX(), r.getTopY() + innerWidth, innerWidth, r.getHeight() - innerWidth), color);
        }
        if (rightInner.get(color)) {
            drawSolidBar(ctx, gfx::Rectangle(r.getRightX() - innerWidth, r.getTopY(), innerWidth, r.getHeight() - innerWidth), color);
            drawSolidBar(ctx, gfx::Rectangle(r.getLeftX() + innerWidth,  r.getBottomY() - innerWidth, r.getWidth() - innerWidth, innerWidth), color);
        }
    }

    // Draw children
    defaultDrawChildren(can);
}

void
ui::widgets::FrameGroup::handleStateChange(State /*st*/, bool /*enable*/)
{ }

// EventConsumer:
bool
ui::widgets::FrameGroup::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::FrameGroup::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}
