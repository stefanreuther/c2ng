/**
  *  \file ui/layoutablegroup.cpp
  */

#include "ui/layoutablegroup.hpp"

// /** Basic layoutable container

//     This widget provides the basis for a layoutable container. It has an UILayoutManager
//     responsible for actual layout, and implements UIBaseComplexWidget's methods using
//     the layout manager.

//     The actual child widget layout area is provided by a descendant's adjustSize()
//     method to allow for additional frames or other decoration by the descendant.

//     UILayoutableWidget provides an add() method to add child widgets. */

// /** Constructor.
//     \param layout Layout manager. Needs to live at least as long as the widget.
//     \param id     Widget id */
ui::LayoutableGroup::LayoutableGroup(ui::layout::Manager& mgr) throw()
    : Widget(),
      m_manager(mgr)
{ }

// void
// ui::LayoutableGroup::handleStateChange(State st, bool enable)
// {
//     // FIXME: what to do?
//     (void) st;
//     (void) enable;
// }

void
ui::LayoutableGroup::requestChildRedraw(Widget& /*child*/, const gfx::Rectangle& area)
{
    requestRedraw(area);
}

void
ui::LayoutableGroup::handleChildAdded(Widget& /*child*/)
{
    // User must call pack().
}

void
ui::LayoutableGroup::handleChildRemove(Widget& /*child*/)
{
    // User must call pack().
}

void
ui::LayoutableGroup::handlePositionChange(gfx::Rectangle& /*oldPosition*/)
{
    doLayout();
}

void
ui::LayoutableGroup::handleChildPositionChange(Widget& /*child*/, gfx::Rectangle& /*oldPosition*/)
{
    // We don't expect children to change their position.
}

ui::layout::Info
ui::LayoutableGroup::getLayoutInfo() const
{
    // ex UILayoutableWidget::getLayoutInfo
    layout::Info info = m_manager.getLayoutInfo(*this);
    return layout::Info(transformPoint(info.getMinSize(), InnerToOuter),
                        transformPoint(info.getPreferredSize(), InnerToOuter),
                        info.getGrowthBehaviour());
}

void
ui::LayoutableGroup::add(Widget& child)
{
    // ex UILayoutableWidget::add
    addChild(child, getLastChild());
}

void
ui::LayoutableGroup::pack()
{
    // ex UILayoutableWidget::pack
    layout::Info info = getLayoutInfo();
    setExtent(gfx::Rectangle(getExtent().getTopLeft(), info.getPreferredSize()));

    // setExtent will call handlePositionChange() to adjust the content
}

void
ui::LayoutableGroup::doLayout()
{
    // ex UILayoutableWidget::doLayout
    m_manager.doLayout(*this, transformSize(getExtent(), OuterToInner));
}

gfx::Point
ui::LayoutableGroup::transformPoint(gfx::Point pt, Transformation kind) const
{
    // ex UILayoutableWidget::adjustLayoutSize
    return transformSize(gfx::Rectangle(gfx::Point(0, 0), pt), kind).getSize();
}
