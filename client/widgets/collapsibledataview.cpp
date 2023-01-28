/**
  *  \file client/widgets/collapsibledataview.cpp
  */

#include "client/widgets/collapsibledataview.hpp"
#include "gfx/context.hpp"
#include "gfx/complex.hpp"
#include "util/skincolor.hpp"

client::widgets::CollapsibleDataView::CollapsibleDataView(ui::Root& root)
    : m_root(root),
      m_viewState(Complete),
      m_title()
{ }

client::widgets::CollapsibleDataView::~CollapsibleDataView()
{ }

void
client::widgets::CollapsibleDataView::draw(gfx::Canvas& can)
{
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    drawBackground(ctx, getExtent());

    // Title
    if (m_viewState != DataOnly) {
        ctx.useFont(*getTitleFont());
        ctx.setColor(util::SkinColor::Heading);
        outTextF(ctx, getExtent().getTopLeft(), getExtent().getWidth(), m_title);
    }

    // Content
    if (m_viewState != HeadingOnly) {
        defaultDrawChildren(can);
    }
}

void
client::widgets::CollapsibleDataView::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
client::widgets::CollapsibleDataView::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    switch (m_viewState) {
     case Complete:
     case DataOnly:
        requestRedraw(area);
        break;

     case HeadingOnly:
        break;
    }
}

void
client::widgets::CollapsibleDataView::handleChildAdded(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::CollapsibleDataView::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
client::widgets::CollapsibleDataView::handlePositionChange()
{
    setChildPositions();
}

void
client::widgets::CollapsibleDataView::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
client::widgets::CollapsibleDataView::getLayoutInfo() const
{
    gfx::Point size;
    switch (m_viewState) {
     case Complete:
        size = getPreferredChildSize() + gfx::Point(0, getTitleFont()->getTextHeight(m_title));
        break;

     case DataOnly:
        size = getPreferredChildSize();
        break;

     case HeadingOnly:
        size = gfx::Point(getPreferredChildSize().getX(), getTitleFont()->getTextHeight(m_title));
        break;
    }
    return size;
}

bool
client::widgets::CollapsibleDataView::handleKey(util::Key_t key, int prefix)
{
    bool result = false;
    switch (m_viewState) {
     case Complete:
     case DataOnly:
        result = defaultHandleKey(key, prefix);
        break;

     case HeadingOnly:
        break;
    }
    return result;
}

bool
client::widgets::CollapsibleDataView::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    bool result = false;
    switch (m_viewState) {
     case Complete:
     case DataOnly:
        result = defaultHandleMouse(pt, pressedButtons);
        break;

     case HeadingOnly:
        break;
    }
    return result;
}

// New nonvirtuals:
void
client::widgets::CollapsibleDataView::setViewState(ViewState state)
{
    if (state != m_viewState) {
        m_viewState = state;
        setChildPositions();
        requestRedraw();
    }
}

void
client::widgets::CollapsibleDataView::setTitle(String_t title)
{
    if (title != m_title) {
        m_title = title;
        requestRedraw();
    }
}

gfx::Point
client::widgets::CollapsibleDataView::getAnchorPoint(int flags) const
{
    // Find initial anchor point
    gfx::Point result;
    if ((flags & LeftAligned) != 0) {
        result = getExtent().getTopLeft();
    } else {
        result = gfx::Point(getExtent().getRightX(), getExtent().getTopY());
    }

    // Adjust
    if (m_viewState == Complete && (flags & DataAligned) != 0) {
        result += gfx::Point(0, getTitleFont()->getTextHeight(m_title));
    }
    return result;
}

ui::Root&
client::widgets::CollapsibleDataView::root() const
{
    return m_root;
}

afl::base::Ref<gfx::Font>
client::widgets::CollapsibleDataView::getTitleFont() const
{
    return root().provider().getFont(gfx::FontRequest().addSize(1));
}
