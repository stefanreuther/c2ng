/**
  *  \file ui/widget.hpp
  *  \brief Base class ui::Widgets
  */
#ifndef C2NG_UI_WIDGET_HPP
#define C2NG_UI_WIDGET_HPP

#include "afl/base/signal.hpp"
#include "afl/bits/smallset.hpp"
#include "gfx/canvas.hpp"
#include "gfx/colorscheme.hpp"
#include "gfx/eventconsumer.hpp"
#include "ui/layout/info.hpp"
#include "util/skincolor.hpp"

namespace ui {

    /** Base class for a user-interface widget.

        Widgets form a hierarchy, that is, every widget can have child widgets.
        Every widget can have only one parent, that is, it cannot be part of multiple widget hierarchies at one time.
        Every widget the user is interacting with has a parent, except for the single root widget (see ui::Root).

        Widgets have an appearance, that is, they can draw themselves on a canvas.
        If a widget has child widgets, it needs to draw the children at appropriate places.

        Widgets consume events.
        If a widget has child widgets, it needs to dispatch events to them appropriately.

        Widgets have an absolute position.
        If a widget is moved, it needs to move its child widgets if appropriate.

        Widgets can have a color scheme or inherit it from their parent.

        Widgets have common states that track keyboard/mouse focus.

        <b>Lifetime</b>

        There are no restrictions on widget lifetime and allocation.
        Widgets do not own their children in a widget tree.
        If a widget dies, it unlinks itself nicely from the tree.
        Widgets can therefor die at any time, in any order. */
    class Widget : public gfx::EventConsumer {
     public:
        /** Widget state. */
        enum State {
            // Removed states:
            // - Exposed. All uses in PCC2 deal with now-illegal display optimisation.
            // - IdleNotify. Used in PCC/PCC2 to simulate a background thread.
            // - WantRedraw/ChildRedraw. Used in PCC2 to control redraw.

            /** Physical (keyboard) input focus.
                The user has actively selected this widget to work with, i.e. placed the cursor on it.
                Only one path (widget and all its parents) can have physical focus in a widget tree. */
            // ex st_Focused
            FocusedState,

            /** Logical (mouse) focus.
                The user is interacting with this widget, i.e. moving a scrollbar.
                The component may not have physical focus, and the mouse may not be over the component.
                Only one path (widget and all its parents) can have logical focus in a widget tree.  */
            // ex st_Selected
            ActiveState,

            /** Widget is disabled. */
            // ex st_Disabled
            DisabledState,

            /** Widget is modal.
                User input does not propagate past this widget. */
            // ex st_Modal
            ModalState
        };

        /** Set of widget states. */
        typedef afl::bits::SmallSet<State> States_t;

        /** Focus type.
            Return value of getFocusState(). */
        enum Focus {
            NoFocus,                ///< This widget doesn't have focus.
            PrimaryFocus,           ///< This widget has the primary focus and is receiving input.
            BackgroundFocus         ///< This widget has focus, but its owner hasn't.
        };

        /** Constructor. */
        Widget() throw();

        /** Destructor.
            If this widget is still part of a widget tree, removes it. */
        ~Widget();

        /** Draw this widget.
            Can be called at any time, with any canvas.
            The widget needs to draw itself and its children.
            \param can Canvas. Not required to live longer than this call. */
        virtual void draw(gfx::Canvas& can) = 0;

        /*
         *  State
         */

        /** Set state.
            You can call this to change the DisabledState or ModalState.
            It is used internally to manage the FocusedState and ActiveState;
            use requestFocus() and requestActive() to changes these from the outside.

            If there is a state change, calls handleStateChange() and sig_handleStateChange().

            \param st State to change
            \param enable true to set this state, false to reset */
        void setState(State st, bool enable);

        /** Check presence of state.
            \param st State to check
            \return state */
        bool hasState(State st) const;

        /** Get set of all states.
            \return set */
        States_t getStates() const;

        /** Handle state change.
            Called by setState() if a state is changed.
            \param st Changed state
            \param enable New state */
        virtual void handleStateChange(State st, bool enable) = 0;

        /** Signal: handle state change.
            Called by setState() if a state is changed.
            \param w *this
            \param st Changed state
            \param enable New state */
        afl::base::Signal<void (Widget&, State, bool)> sig_handleStateChange;

        /*
         *  Redraw
         */

        /** Request redraw of the given area.
            Calls parent's requestChildRedraw(), and sig_handleRedraw.
            \param area Area to redraw */
        void requestRedraw(const gfx::Rectangle& area);

        /** Request redraw of this widget. */
        void requestRedraw();

        /** Request redraw of a child.
            \param child Invoking child
            \param area Area to redraw */
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area) = 0;

        /** Signal: redraw requested.
            \param w *this
            \param area Area to redraw */
        afl::base::Signal<void (Widget&, const gfx::Rectangle&)> sig_handleRedraw;

        /*
         *  Physical Focus Handling
         */

        /** Request this widget to be focused (and all siblings be not focused). */
        void requestFocus();

        /** Get focused child.
            \return focused child; can be null */
        Widget* getFocusedChild() const;

        /** Set focused child.
            \param w Widget to focus. Must be a direct child of this one, or null. */
        void setFocusedChild(Widget* w);

        /** Get focus state.
            This should affect the widget's appearance.
            \return focus state */
        Focus getFocusState() const;

        /** Signal: focus change */
        afl::base::Signal<void ()> sig_handleFocusChange;

        /*
         *  Logical Focus Handling / Activation
         */

        /** Request this widget to be the active widget (and all siblings be not active).
            A well-behaved widget calls this function when the user talks to it,
            and dropActive() when the user no longer talks to it.
            It reacts to loss of the ActiveState.

            This is to prevent things like two buttons being depressed simultaneously
            because one of them did not notice that the user moved the mouse out of it. */
        void requestActive();

        /** Get active child.
            \return active child; can be null */
        Widget* getActiveChild() const;

        /** Set active widget.
            If a different widget is active, makes it inactive.
            \param w Widget. Must be a direct child of this one, or null. */
        void setActiveChild(Widget* w);

        /** Request this widget to be not active anymore. */
        void dropActive();

        /*
         *  Adding Children
         */

        /** Add child widget.
            Note that derived classes usually provide an additional add() method (which eventually calls addChild())
            to support a particular child widget policy.
            \param child Child to add
            \param addAfter Add after this widget; null to add as first child.
            \pre child.getParent()==0 */
        void addChild(Widget& child, Widget* addAfter);

        /** Callback after child has been added.
            \param child Newly-added child; has been fully added into the widget tree */
        virtual void handleChildAdded(Widget& child) = 0;

        /*
         *  Removing children
         */

        /** Remove child widget.
            \param child Child to remove
            \pre child.getParent()==this */
        void removeChild(Widget& child);

        /** Callback after child has been added.
            \param child Child to remove; still present on widget tree */
        virtual void handleChildRemove(Widget& child) = 0;

        /*
         *  Position Change
         */

        /** Set widget extent (position and size).
            \param extent New extent */
        void setExtent(const gfx::Rectangle& extent);

        /** Get widget extent.
            \return extent */
        const gfx::Rectangle& getExtent() const;

        /** Callback: position changed.
            At this time, getExtent() returns the new position. */
        virtual void handlePositionChange() = 0;

        /** Callback: a child has moved.
            At this time, child.getExtent() returns the new position.
            \param child Child
            \param oldPosition Old position */
        virtual void handleChildPositionChange(Widget& child, const gfx::Rectangle& oldPosition) = 0;

        /** Get layout parameters.
            \return parameters */
        virtual ui::layout::Info getLayoutInfo() const = 0;

        /*
         *  Color Scheme
         */

        /** Set color scheme.
            \param scheme New color scheme. Must out-live this widget. */
        void setColorScheme(gfx::ColorScheme<util::SkinColor::Color>& scheme);

        /** Get color scheme.
            \return color scheme (set with setColorScheme(), or parent's). */
        gfx::ColorScheme<util::SkinColor::Color>& getColorScheme() const;

        /*
         *  Links
         */

        /** Get parent.
            \return parent (null if none) */
        Widget* getParent() const;

        /** Get next sibling.
            \return next sibling (null if none, i.e. I am the last one) */
        Widget* getNextSibling() const;

        /** Get previous sibling.
            \return previous sibling (null if none, i.e. I am the first one) */
        Widget* getPreviousSibling() const;

        /** Get first child.
            \return first child (null if none). */
        Widget* getFirstChild() const;

        /** Get last child.
            \return last child (null if none). */
        Widget* getLastChild() const;

     protected:
        /** Default key handler.
            Call this from handleKey() if desired.

            Keys are first given to the focused child.
            If they do not want it, they are given to all other children.
            Processing stops after a modal child.

            \param key Key
            \param prefix Prefix argument

            \retval true Key was handled
            \retval false Key was not handled */
        bool defaultHandleKey(util::Key_t key, int prefix);

        /** Default mouse handler.
            Call this from handleMouse() if desired.

            Mouse events are first given to the active child.
            If they do not want it, they are given to all other children.
            Processing stops after a modal child.

            \param pt Mouse position
            \param pressedButtons buttons

            \retval true Event was handled
            \retval false Event was not handled */
        bool defaultHandleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Default child drawing.
            Call this from draw() if desired.

            All children are drawn, in normal order.
            This is an appropriate implementation for regular containers
            of non-overlapping widgets.

            \param can Canvas */
        void defaultDrawChildren(gfx::Canvas& can);

     private:
        Widget* m_parent;              /**< Parent of this widget. */
        Widget* m_nextSibling;         /**< Next sibling. */
        Widget* m_previousSibling;     /**< Previous sibling. */
        Widget* m_firstChild;          /**< First child. */
        Widget* m_lastChild;           /**< Last child. */
        Widget* m_focusedChild;        /**< Focused child. */
        Widget* m_activeChild;         /**< Selected child. */
        gfx::ColorScheme<util::SkinColor::Color>* m_colorScheme;
        States_t m_states;
        gfx::Rectangle m_extent;
    };

}

// Check presence of state.
inline bool
ui::Widget::hasState(State st) const
{
    return m_states.contains(st);
}

// Get set of all states.
inline ui::Widget::States_t
ui::Widget::getStates() const
{
    return m_states;
}

// Get focused child.
inline ui::Widget*
ui::Widget::getFocusedChild() const
{
    return m_focusedChild;
}

// Get active child.
inline ui::Widget*
ui::Widget::getActiveChild() const
{
    return m_activeChild;
}

// Get widget extent.
inline const gfx::Rectangle&
ui::Widget::getExtent() const
{
    return m_extent;
}

// Get parent.
inline ui::Widget*
ui::Widget::getParent() const
{
    return m_parent;
}

// Get next sibling.
inline ui::Widget*
ui::Widget::getNextSibling() const
{
    return m_nextSibling;
}

// Get previous sibling.
inline ui::Widget*
ui::Widget::getPreviousSibling() const
{
    return m_previousSibling;
}

// Get first child.
inline ui::Widget*
ui::Widget::getFirstChild() const
{
    return m_firstChild;
}

// Get last child.
inline ui::Widget*
ui::Widget::getLastChild() const
{
    return m_lastChild;
}

#endif
