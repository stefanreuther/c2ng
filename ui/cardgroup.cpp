/**
  *  \file ui/cardgroup.cpp
  *  \brief Class ui::CardGroup
  */

#include "ui/cardgroup.hpp"

namespace {
    gfx::Point maxPoint(gfx::Point a, gfx::Point b)
    {
        return gfx::Point(std::max(a.getX(), b.getX()),
                          std::max(a.getY(), b.getY()));
    }
}


ui::CardGroup::CardGroup() throw()
    : Widget()
{
    sig_handleFocusChange.add(this, (void (CardGroup::*)()) &CardGroup::requestRedraw);
}

ui::CardGroup::~CardGroup()
{
    // ex UICardGroup::~UICardGroup
}

void
ui::CardGroup::add(Widget& w)
{
    addChild(w, getLastChild());
}

void
ui::CardGroup::handleStateChange(State /*st*/, bool /*enable*/)
{ }

// Redraw
void
ui::CardGroup::draw(gfx::Canvas& can)
{
    // ex UICardGroup::drawEverything etc.
    if (Widget* w = getFocusedChild()) {
        w->draw(can);
    }
}

void
ui::CardGroup::requestChildRedraw(Widget& child, const gfx::Rectangle& area)
{
    if (getFocusedChild() == &child) {
        requestRedraw(area);
    }
}

// Add
void
ui::CardGroup::handleChildAdded(Widget& child)
{
    // UICardGroup::onAddRemoveChild
    child.setState(FocusedState, getFocusedChild() == &child);
    child.setExtent(getExtent());
    if (getFocusedChild() == &child) {
        requestRedraw();
    }
}

// Remove
void
ui::CardGroup::handleChildRemove(Widget& /*child*/)
{
    requestRedraw();
}

void
ui::CardGroup::handlePositionChange()
{
    // ex UICardGroup::onResize
    for (Widget* w = getFirstChild(); w != 0; w = w->getNextSibling()) {
        w->setExtent(getExtent());
    }
    requestRedraw();
}

void
ui::CardGroup::handleChildPositionChange(Widget& /*child*/, const gfx::Rectangle& /*oldPosition*/)
{ }

ui::layout::Info
ui::CardGroup::getLayoutInfo() const
{
    // ex UICardGroup::getLayoutInfo
    using ui::layout::Info;
    Info result(gfx::Point(1, 1), gfx::Point(1, 1), Info::GrowBoth);

    for (Widget* w = getFirstChild(); w != 0; w = w->getNextSibling()) {
        Info child = w->getLayoutInfo();
        result = Info(maxPoint(result.getMinSize(), child.getMinSize()),
                      maxPoint(result.getPreferredSize(), child.getPreferredSize()),
                      Info::andGrowthBehaviour(result.getGrowthBehaviour(), child.getGrowthBehaviour()));
    }
    return result;
}

// Event
bool
ui::CardGroup::handleKey(util::Key_t key, int prefix)
{
    // ex UICardGroup::handleEvent
    if (Widget* w = getFocusedChild()) {
        return w->handleKey(key, prefix);
    } else {
        return false;
    }
}

bool
ui::CardGroup::handleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UICardGroup::handleEvent
    if (Widget* w = getFocusedChild()) {
        return w->handleMouse(pt, pressedButtons);
    } else {
        return false;
    }
}
