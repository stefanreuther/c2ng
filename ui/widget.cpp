/**
  *  \file ui/widget.cpp
  */

#include <cassert>
#include "ui/widget.hpp"
#include "afl/except/assertionfailedexception.hpp"

ui::Widget::Widget()
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

// /** Set status. Call this to change the status flags. Derived widgets
//     can override onStateChange() to react on status changes. */
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

bool
ui::Widget::hasState(State st) const
{
    return m_states.contains(st);
}

ui::Widget::States_t
ui::Widget::getStates() const
{
    return m_states;
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


void
ui::Widget::requestRedraw(const gfx::Rectangle& area)
{
    if (m_parent != 0) {
        m_parent->requestChildRedraw(*this, area);
    }
    sig_handleRedraw.raise(*this, area);
}

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

// /** Get focus state. */
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


// FIXME: delete. This function has the wrong abstraction.
// It is used by UIFocusIterator.
// /** Get focused widget. When called on a focused group, returns the
//     innermost focused child widget.

//     Note: can't be const because it might return /this/. */
// UIBaseWidget*
// UIBaseWidget::getFocusedWidget()
// {
//     if (hasState(st_Focused))
//         if (focused_child != 0)
//             return focused_child->getFocusedWidget();
//         else
//             return this;
//     else
//         return 0;
// }


// /** Obtain logical focus. You must call this function prior
//     reacting to user actions. A well-behaved widget
//     - calls setSelectedWidget() when the user talks to it. This sets this
//       widget's st_Selected status flag;
//     - calls clearSelectedWidget() when it determines that the user no
//       longer talks to it.
//     - possibly overrides onStateChange() to react when it loses st_Selected.

//     This is to prevent things like two buttons being depressed
//     simultaneously because one of them did not notice that the user
//     moved the mouse out of it. */
void
ui::Widget::requestActive()
{
    // ex UIBaseWidget::setSelectedWidget
    if (m_parent != 0) {
        m_parent->requestActive();
        m_parent->setActiveChild(this);
    }
}

// /** Give logical focus to widget.
//     If w points to a widget, that widget wants to get the input focus.
//     Take it away from the one which currently has it, and give it to
//     the specified widget. Also grab the logical focus for this
//     UIBaseWidgetContainer, to make this process happen recursively.

//     \pre w != 0 => w->getParent() == this */
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

// /** Drop logical focus. If this widget is currently selected, unselect it.
//     FIXME: this currently only works for leaf widgets. */
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


// /** Add widget. Note that bases usually provide an own add() method (e.g. UIWidget::add(),
//     UILayoutableWidget::add()) to support a particular child widget policy. Those methods
//     ultimately call addChildWidget(). You should generally be using those method instead of
//     addChildWidget().

//     \param the_widget widget to add
//     \param add_after add after this widget. 0 means add at front. Otherwise, must
//                      be a child widget of us. */
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

/** Remove child widget. The child widget will then no longer be part of
    this widget. */
void
ui::Widget::removeChild(Widget& child)
{
    // ex UIBaseWidget::removeChildWidget
    assert(child.m_parent == this);

    // Notify descendant. FIXME: I'm not 100% sure whether it's right to notify here.
    // pro: descendant can know where in the list it is
    // con: descendant cannot easily perform immediate redraw, because it does not know
    //   how the new list will look like
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

const gfx::Rectangle&
ui::Widget::getExtent() const
{
    return m_extent;
}

void
ui::Widget::setColorScheme(gfx::ColorScheme& scheme)
{
    // ex UIBaseWidget::setSkin, sort-of
    m_colorScheme = &scheme;
}

gfx::ColorScheme&
ui::Widget::getColorScheme() const
{
    if (m_colorScheme != 0) {
        return *m_colorScheme;
    } else {
        afl::except::checkAssertion(m_parent != 0, "no parent", "<Widget::getColorScheme>");
        return m_parent->getColorScheme();
    }
}

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
        if (w->handleKey(key, prefix)) {
            return true;
        }
        if (w->hasState(ModalState)) {
            break;
        }
    }
    return false;
}

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
        if (w->handleMouse(pt, pressedButtons)) {
            return true;
        }
        if (w->hasState(ModalState)) {
            break;
        }
    }
    return false;
}

void
ui::Widget::defaultDrawChildren(gfx::Canvas& can)
{
    // ex UIBaseComplexWidget::drawEverything, sort-of
    for (Widget* w = getFirstChild(); w != 0; w = w->getNextSibling()) {
        w->draw(can);
    }
}
