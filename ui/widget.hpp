/**
  *  \file ui/widget.hpp
  */
#ifndef C2NG_UI_WIDGET_HPP
#define C2NG_UI_WIDGET_HPP

#include "gfx/eventconsumer.hpp"
#include "gfx/canvas.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/signal.hpp"
#include "ui/layout/info.hpp"
#include "gfx/colorscheme.hpp"

namespace ui {

    class Widget : public gfx::EventConsumer {
     public:
        enum State {
            // Removed states:
            // - Exposed. All uses in PCC2 deal with now-illegal display optimisation.
            // - IdleNotify. Used in PCC/PCC2 to simulate a background thread.
            // - WantRedraw/ChildRedraw. Used in PCC2 to control redraw.
            FocusedState,  // ex st_Focused     = 2,     ///< Component has physical input focus. The user has actively selected it to work with.
            ActiveState,   // ex st_Selected    = 4,     ///< Component has logical input focus. The user is actually interacting with this component.
            DisabledState, // ex st_Disabled    = 8,     ///< Component cannot be selected.
            ModalState     // ex st_Modal       = 32,    ///< Component is modal.
        };
        typedef afl::bits::SmallSet<State> States_t;

        enum Focus {
            NoFocus,                ///< This widget doesn't have focus.
            PrimaryFocus,           ///< This widget has the primary focus and is receiving input.
            BackgroundFocus         ///< This widget has focus, but its owner hasn't.
        };
        
        Widget();
        ~Widget();

        virtual void draw(gfx::Canvas& can) = 0;

        // State
        void setState(State st, bool enable);
        bool hasState(State st) const;
        States_t getStates() const;
        virtual void handleStateChange(State st, bool enable) = 0;
        afl::base::Signal<void (Widget&, State, bool)> sig_handleStateChange;

        // Redraw
        void requestRedraw(const gfx::Rectangle& area);
        void requestRedraw();
        virtual void requestChildRedraw(Widget& child, const gfx::Rectangle& area) = 0;
        afl::base::Signal<void (Widget&, const gfx::Rectangle&)> sig_handleRedraw;

        // Focus
        void requestFocus();
        Widget* getFocusedChild() const { return m_focusedChild; }
        void setFocusedChild(Widget* w);
        Focus getFocusState() const;
        afl::base::Signal<void ()> sig_handleFocusChange;

        // Activation
        void requestActive();
        void setActiveChild(Widget* w);
        void dropActive();

        // Add
        void addChild(Widget& child, Widget* addAfter);
        virtual void handleChildAdded(Widget& child) = 0;

        // Remove
        void removeChild(Widget& child);
        virtual void handleChildRemove(Widget& child) = 0;

        // Position change
        void setExtent(const gfx::Rectangle& extent);
        const gfx::Rectangle& getExtent() const;
        virtual void handlePositionChange(gfx::Rectangle& oldPosition) = 0;
        virtual void handleChildPositionChange(Widget& child, gfx::Rectangle& oldPosition) = 0;
        virtual ui::layout::Info getLayoutInfo() const = 0;

        // Color scheme
        void setColorScheme(gfx::ColorScheme& scheme);
        gfx::ColorScheme& getColorScheme() const;

        // Links
        Widget* getParent() const { return m_parent; }
        Widget* getNextSibling() const { return m_nextSibling; }
        Widget* getPreviousSibling() const { return m_previousSibling; }
        Widget* getFirstChild() const { return m_firstChild; }
        Widget* getLastChild() const { return m_lastChild; }
        Widget* getActiveChild() const { return m_activeChild; }

     protected:
        bool defaultHandleKey(util::Key_t key, int prefix);
        bool defaultHandleMouse(gfx::Point pt, MouseButtons_t pressedButtons);
        void defaultDrawChildren(gfx::Canvas& can);

     private:
        Widget* m_parent;              /**< Parent of this widget. */
        Widget* m_nextSibling;         /**< Next sibling. */
        Widget* m_previousSibling;     /**< Previous sibling. */
        Widget* m_firstChild;          /**< First child. */
        Widget* m_lastChild;           /**< Last child. */
        Widget* m_focusedChild;        /**< Focused child. */
        Widget* m_activeChild;         /**< Selected child. */
        gfx::ColorScheme* m_colorScheme;
        States_t m_states;
        int m_id;
        gfx::Rectangle m_extent;
    };

}

#endif
