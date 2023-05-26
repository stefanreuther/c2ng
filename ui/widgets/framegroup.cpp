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
ui::widgets::FrameGroup::FrameGroup(const ui::layout::Manager& mgr,
                                    ColorScheme& colors,
                                    FrameType type)
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
ui::widgets::FrameGroup::setType(FrameType type)
{
    // ex WColorFrame::setColor
    if (m_frameType != type) {
        m_frameType = type;
        requestRedraw();
    }
}

// Get type (color).
ui::FrameType
ui::widgets::FrameGroup::getType() const
{
    return m_frameType;
}

// Wrap a single widget within a FrameGroup.
ui::widgets::FrameGroup&
ui::widgets::FrameGroup::wrapWidget(afl::base::Deleter& del, ColorScheme& colors, FrameType type, Widget& widget)
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
    // Draw frame
    gfx::Context<uint8_t> ctx(can, m_colors);
    drawFrame(ctx, getExtent(), m_frameType, m_frameWidth);

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
