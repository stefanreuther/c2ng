/**
  *  \file ui/widget.cpp
  *  \brief Base class ui::Widgets
  */

#include <cassert>
#include "ui/widget.hpp"
#include "afl/except/assertionfailedexception.hpp"

// Constructor.
ui::Widget::Widget() throw()
    : m_parent(0),
      m_nextSibling(0),
      m_previousSibling(0),
      m_firstChild(0),
      m_lastChild(0),
      m_focusedChild(0),
      m_activeChild(0),
      m_colorScheme(0),
      m_states(),
      m_extent()
{ }

// Destructor.
ui::Widget::~Widget()
{
    // ex UIBaseWidget::~UIBaseWidget

    // Drop all states.
    // Parent will try to remove states from us as well; we must make sure this does not yield any virtual callbacks.
    // We're dying, so we're not interested in callbacks anyway.
    m_states.clear();

    // Disconnect from parent to avoid that they call us
    if (Widget* parent = m_parent) {
        // Remove links.
        bool needNewFocus = false;
        if (parent->m_activeChild == this) {
            parent->m_activeChild = 0;
        }
        if (parent->m_focusedChild == this) {
            parent->m_focusedChild = 0;
            needNewFocus = true;
        }

        // Remove from parent.
        // This will reset our m_parent link which is why we keep working with our local copy.
        parent->removeChild(*this);

        // If we had focus, find new widget with focus.
        // Normally, removeChild() will do that, but we have previously cut our links to avoid callbacks,
        // so we now have to do it manually.
        if (needNewFocus) {
            Widget* p = parent->m_firstChild;
            while (p != 0 && p->hasState(DisabledState)) {
                p = p->m_nextSibling;
            }
            parent->setFocusedChild(p);
        }
    }

    // Cut internal links
    m_activeChild = 0;
    m_focusedChild = 0;

    // Remove all children
    while (m_firstChild != 0) {
        // Remove from list
        Widget* p = m_firstChild;
        m_firstChild = p->m_nextSibling;

        // Cut all links
        p->m_parent = 0;
        p->m_nextSibling = 0;
        p->m_previousSibling = 0;

        // Remove states
        p->setState(FocusedState, false);
        p->setState(ActiveState, false);
    }
    m_lastChild = 0;
}

// Set state.
void
ui::Widget::setState(State st, bool enable)
{
    // ex UIBaseWidget::setState, UIBaseWidget::onStateChange
    bool raise = false;
    if (enable) {
        if (!m_states.contains(st)) {
            m_states += st;
            raise = true;
        }
    } else {
        if (m_states.contains(st)) {
            m_states -= st;
            raise = true;
        }
    }

    if (raise) {
        // FIXME: (do we need to) port this?
    // // propagate selection loss
    // if ((bits & st_Selected) && !enabled) {
    //     if (selected_child != 0) {
    //         selected_child->setState(st_Selected, false);
    //         selected_child = 0;
    //     }
    // }

        // propagate focus gain/loss
        if (st == FocusedState) {
            if (m_focusedChild != 0) {
                m_focusedChild->setState(FocusedState, enable);
            }
        }
        handleStateChange(st, enable);
        sig_handleStateChange.raise(*this, st, enable);
    }
}


/*
 *  Redraw
 *
 *  Widget requests redraw of a rectangle.
 *  Parent must eventually call draw().
 *  - CardGroup: pass on request if widget is focused
 *  - ComplexWidget: pass on request
 *  - Root: remember request for execution
 */

// Request redraw of the given area.
void
ui::Widget::requestRedraw(const gfx::Rectangle& area)
{
    if (m_parent != 0) {
        m_parent->requestChildRedraw(*this, area);
    }
    sig_handleRedraw.raise(*this, area);
}

// Request redraw of this widget.
void
ui::Widget::requestRedraw()
{
    requestRedraw(getExtent());
}


/*
 *  Focus
 *
 *  Invariant: FocusState <=> m_parent->m_focusedChild==this
 */

// Request this widget to be focused (and all siblings be not focused).
void
ui::Widget::requestFocus()
{
    // ex UIBaseWidget::setFocusedWidget
    if (m_parent != 0) {
        // We have a parent. Request focus for that, then for us.
        m_parent->requestFocus();
        m_parent->setFocusedChild(this);
    } else {
        // We have no parent. Remember that we want focus.
        setState(FocusedState, true);
    }
}

// Set focused child.
void
ui::Widget::setFocusedChild(Widget* w)
{
    if (w != m_focusedChild) {
        if (m_focusedChild != 0) {
            m_focusedChild->setState(FocusedState, false);
        }
        m_focusedChild = w;
        if (m_focusedChild != 0) {
            m_focusedChild->setState(FocusedState, true);
        }
        sig_handleFocusChange.raise();
    }
}

// Get focus state.
ui::Widget::Focus
ui::Widget::getFocusState() const
{
    // ex UIBaseWidget::getFocusState() const
    if (hasState(FocusedState)) {
        return PrimaryFocus;
    } else {
        /* Go up in widget hierarchy.
           - when we reach a modal window, we're background focus (means: there is another window that has focus)
           - when we reach the top, we don't have focus
           - when we reach a window whose focused child is modal, we have background focus (means: we're a nonmodal window, and a modal one is in front)
           - when we reach another window whose focused child differs from us, we don't have focus (regular nesting) */
        const Widget* p = this;
        while (1) {
            if (p->hasState(ModalState)) {
                return BackgroundFocus;
            }

            Widget* parent = p->getParent();
            if (parent == 0) {
                return NoFocus;
            }
            if (parent->getFocusedChild() != 0 && parent->getFocusedChild()->hasState(ModalState)) {
                return BackgroundFocus;
            }
            if (parent->getFocusedChild() != p || parent->hasState(FocusedState)) {
                return NoFocus;
            }
            p = parent;
        }
    }
}


// Request this widget to be the active widget (and all siblings be not active).
void
ui::Widget::requestActive()
{
    // ex UIBaseWidget::setSelectedWidget
    if (m_parent != 0) {
        m_parent->requestActive();
        m_parent->setActiveChild(this);
    }
}

// Set active widget.
void
ui::Widget::setActiveChild(Widget* w)
{
    // ex UIBaseWidget::setSelectedWidget
    if (w != m_activeChild) {
        if (m_activeChild != 0) {
            m_activeChild->setState(ActiveState, false);
        }
        m_activeChild = w;
        if (m_activeChild != 0) {
            m_activeChild->setState(ActiveState, true);
        }
    }
}

// Request this widget to be not active anymore.
void
ui::Widget::dropActive()
{
    // ex UIBaseWidget::clearSelectedWidget
    if (m_parent != 0) {
        if (m_parent->getActiveChild() == this) {
            m_parent->setActiveChild(0);
        }
    }
}



/*
 *  Adding children
 *
 *  - CardGroup: set exposed state
 *  - ComplexWidget: set exposed state
 *  - Root: set exposed state
 */


// Add child widget.
void
ui::Widget::addChild(Widget& child, Widget* addAfter)
{
    // ex UIBaseWidget::addChildWidget
    assert(child.m_parent == 0);

    // First, add into our widget chain
    if (m_firstChild == 0) {
        // We have no children
        assert(addAfter == 0);
        assert(m_lastChild == 0);
        child.m_previousSibling = child.m_nextSibling = 0;
        m_firstChild = m_lastChild = &child;
    } else if (addAfter == 0) {
        // Add at front
        assert(m_lastChild != 0);
        child.m_previousSibling = 0;
        child.m_nextSibling = m_firstChild;
        m_firstChild->m_previousSibling = &child;
        m_firstChild = &child;
    } else {
        // Add at end
        assert(addAfter->m_parent == this);
        child.m_previousSibling = addAfter;
        child.m_nextSibling = addAfter->m_nextSibling;
        addAfter->m_nextSibling = &child;
        if (child.m_nextSibling != 0) {
            // There still is a next sibling, so update its back pointer
            child.m_nextSibling->m_previousSibling = &child;
        } else {
            // No next sibling, so we're the new last
            m_lastChild = &child;
        }
    }

    // Second, tell it its new parent
    child.m_parent = this;

    // Fourth, propagate focus
    if (m_focusedChild == 0 && !child.hasState(DisabledState)) {
        m_focusedChild = &child;
        child.setState(FocusedState, hasState(FocusedState));
    } else {
        child.setState(FocusedState, false);
    }

    // Sixths, propagate exposedness
    handleChildAdded(child);
}

// Remove child widget.
void
ui::Widget::removeChild(Widget& child)
{
    // ex UIBaseWidget::removeChildWidget
    assert(child.m_parent == this);

    // Notify descendant
    handleChildRemove(child);

    // Remove prev/next links
    if (child.m_previousSibling != 0) {
        child.m_previousSibling->m_nextSibling = child.m_nextSibling;
    }
    if (child.m_nextSibling != 0) {
        child.m_nextSibling->m_previousSibling = child.m_previousSibling;
    }
    if (m_firstChild == &child) {
        m_firstChild = child.m_nextSibling;
    }
    if (m_lastChild == &child) {
        m_lastChild = child.m_previousSibling;
    }
    child.m_previousSibling = child.m_nextSibling = 0;
    child.m_parent = 0;

    // Remove focus
    if (m_activeChild == &child) {
        // Widget was active, so nobody is active now.
        setActiveChild(0);
    }
    if (m_focusedChild == &child) {
        // Widget was focused, so pick a new focused widget.
        // This behaviour picks the first one, which is the right behaviour for top-level windows.
        Widget* p = m_firstChild;
        while (p != 0 && p->hasState(DisabledState)) {
            p = p->m_nextSibling;
        }
        setFocusedChild(p);
    }

    // Take away other state
    child.setState(FocusedState, false);
}

// Set widget extent (position and size).
void
ui::Widget::setExtent(const gfx::Rectangle& extent)
{
    // ex UIBaseWidget::setExtent
    gfx::Rectangle oldPosition = m_extent;
    if (oldPosition != extent) {
        m_extent = extent;
        handlePositionChange(oldPosition);
        if (m_parent != 0) {
            m_parent->handleChildPositionChange(*this, oldPosition);
        }
    }
}

// Set color scheme.
void
ui::Widget::setColorScheme(gfx::ColorScheme<util::SkinColor::Color>& scheme)
{
    // ex UIBaseWidget::setSkin, sort-of
    m_colorScheme = &scheme;
}

// Get color scheme.
gfx::ColorScheme<util::SkinColor::Color>&
ui::Widget::getColorScheme() const
{
    if (m_colorScheme != 0) {
        return *m_colorScheme;
    } else {
        afl::except::checkAssertion(m_parent != 0, "no parent", "<Widget::getColorScheme>");
        return m_parent->getColorScheme();
    }
}

// Default key handler.
bool
ui::Widget::defaultHandleKey(util::Key_t key, int prefix)
{
    // ex UIBaseComplexWidget::handleEvent, sort-of
    if (Widget* w = getFocusedChild()) {
        if (w->handleKey(key, prefix)) {
            return true;
        }
    }
    for (Widget* w = getFirstChild(); w != 0; w = w->getNextSibling()) {
        // Focused child has already been processed above; do not process it again.
        if (w != getFocusedChild()) {
            if (w->handleKey(key, prefix)) {
                return true;
            }
        }
        if (w->hasState(ModalState)) {
            break;
        }
    }
    return false;
}

// Default mouse handler.
bool
ui::Widget::defaultHandleMouse(gfx::Point pt, MouseButtons_t pressedButtons)
{
    // ex UIBaseComplexWidget::handleEvent, sort-of
    if (Widget* w = getActiveChild()) {
        if (w->handleMouse(pt, pressedButtons)) {
            return true;
        }
    }
    for (Widget* w = getFirstChild(); w != 0; w = w->getNextSibling()) {
        // Active child has already been processed above; do not process it again.
        if (w != getActiveChild()) {
            if (w->handleMouse(pt, pressedButtons)) {
                return true;
            }
        }
        if (w->hasState(ModalState)) {
            break;
        }
    }
    return false;
}

// Default child drawing.
void
ui::Widget::defaultDrawChildren(gfx::Canvas& can)
{
    // ex UIBaseComplexWidget::drawEverything, sort-of
    for (Widget* w = getFirstChild(); w != 0; w = w->getNextSibling()) {
        w->draw(can);
    }
}
