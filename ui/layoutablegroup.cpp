/**
  *  \file ui/layoutablegroup.cpp
  *  \brief Class ui::LayoutableGroup
  */

#include "ui/layoutablegroup.hpp"

ui::LayoutableGroup::LayoutableGroup(const ui::layout::Manager& mgr) throw()
    : Widget(),
      m_manager(mgr)
{ }

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
ui::LayoutableGroup::handlePositionChange()
{
    doLayout();
}

void
ui::LayoutableGroup::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{
    // We don't expect children to change their position.
}

ui::layout::Info
ui::LayoutableGroup::getLayoutInfo() const
{
    // ex UILayoutableWidget::getLayoutInfo
    layout::Info info = m_manager.getLayoutInfo(*this);
    return layout::Info(transformPoint(info.getPreferredSize(), InnerToOuter), info.getGrowthBehaviour());
}

void
ui::LayoutableGroup::add(Widget& child)
{
    // ex UILayoutableWidget::add
    // This method is in LayoutableGroup to clear that adding as last is the norm for LayoutableGroups,
    // to avoid using it in other containers where that is rare.
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
