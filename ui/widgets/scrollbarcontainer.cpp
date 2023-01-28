/**
  *  \file ui/widgets/scrollbarcontainer.cpp
  *  \brief Class ui::widgets::ScrollbarContainer
  */

#include "ui/widgets/scrollbarcontainer.hpp"

ui::widgets::ScrollbarContainer::ScrollbarContainer(ScrollableWidget& widget, Root& root)
    : Widget(),
      m_widget(widget),
      m_scrollbar(widget, root),
      m_hasScrollbar(false),
      conn_change(widget.sig_change.add(this, &ScrollbarContainer::onChange))
{
    addChild(m_widget, 0);
}

ui::widgets::ScrollbarContainer::~ScrollbarContainer()
{ }

void
ui::widgets::ScrollbarContainer::draw(gfx::Canvas& can)
{
    defaultDrawChildren(can);
}

void
ui::widgets::ScrollbarContainer::handleStateChange(State /*st*/, bool /*enable*/)
{ }

void
ui::widgets::ScrollbarContainer::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
ui::widgets::ScrollbarContainer::handleChildAdded(Widget& /*child*/)
{
    // We control widget addition/removal; callback not evaluated
}

void
ui::widgets::ScrollbarContainer::handleChildRemove(Widget& /*child*/)
{
    // We control widget addition/removal; callback not evaluated
}

void
ui::widgets::ScrollbarContainer::handlePositionChange()
{
    doLayout();
}

void
ui::widgets::ScrollbarContainer::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{
    // We control position changes; callback not evaluated.
}

ui::layout::Info
ui::widgets::ScrollbarContainer::getLayoutInfo() const
{
    ui::layout::Info widgetInfo = m_widget.getLayoutInfo();
    ui::layout::Info scrollInfo = m_scrollbar.getLayoutInfo();

    return ui::layout::Info(widgetInfo.getMinSize().extendRight(scrollInfo.getMinSize()),
                            widgetInfo.getPreferredSize().extendRight(scrollInfo.getPreferredSize()),
                            widgetInfo.getGrowthBehaviour());
}

bool
ui::widgets::ScrollbarContainer::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

bool
ui::widgets::ScrollbarContainer::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    return defaultHandleMouse(pt, pressedButtons);
}

void
ui::widgets::ScrollbarContainer::onChange()
{
    doLayout();
}

void
ui::widgets::ScrollbarContainer::doLayout()
{
    bool needScrollbar = m_widget.getTotalSize() > m_widget.getPageSize();
    bool needRedraw = false;

    // Add/remove widgets
    if (needScrollbar > m_hasScrollbar) {
        // Must add scrollbar
        addChild(m_scrollbar, 0);
        needRedraw = true;
    }
    if (needScrollbar < m_hasScrollbar) {
        // Must remove scrollbar
        removeChild(m_scrollbar);
        needRedraw = true;
    }
    m_hasScrollbar = needScrollbar;

    // Layout
    gfx::Rectangle area = getExtent();
    if (m_hasScrollbar) {
        gfx::Rectangle scrollArea = area.splitRightX(m_scrollbar.getLayoutInfo().getPreferredSize().getX());
        if (scrollArea != m_scrollbar.getExtent()) {
            m_scrollbar.setExtent(scrollArea);
            needRedraw = true;
        }
    }
    if (area != m_widget.getExtent()) {
        m_widget.setExtent(area);
        needRedraw = true;
    }

    // Redraw
    if (needRedraw) {
        requestRedraw();
    }
}
