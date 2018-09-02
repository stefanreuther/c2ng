/**
  *  \file ui/widgets/abstractbutton.hpp
  *  \brief Class ui::widgets::AbstractButton
  */
#ifndef C2NG_UI_WIDGETS_ABSTRACTBUTTON_HPP
#define C2NG_UI_WIDGETS_ABSTRACTBUTTON_HPP

#include "afl/base/signal.hpp"
#include "ui/draw.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"

namespace ui { namespace widgets {

    /*
     *  Transition from old methods:
     *     addCommand -> retire, replace by event listener
     *     addStop -> replace by sig_fire.addNewClosure(loop.makeStop(n))
     *     addKey -> replace by sig_fireKey listener
     *
     *  Transition for flags:
     *     bf_Pressed -> PressedButton
     *     bf_Highlight -> HighlightedButton
     *     bf_Key -> replace by sig_fireKey listener
     *     bf_SmallSize -> Button::setFont
     *     bf_PressOverride -> ?
     *     bf_LeftJust -> ?
     */


    /** Base class for a regular push-button.
        This implements the event handling of a standard push button:
        - a matching keypress produces an event
        - a mouse click within the button produces an event
        Event handling honors prefix arguments.

        Derived classes must implement the missing abstract methods:
        - appearance-related: draw, getLayoutInfo, handlePositionChange
        - handleStateChange: call defaultHandleStateChange
        - handleKey: call defaultHandleKey
        - handleMouse: call defaultHandleMouse

        Event receivers can attach events to sig_fire and sig_fireKey that differ in their parameters. */
    class AbstractButton : public SimpleWidget {
     public:
        /** Constructor.
            \param root Associated UI root (required for prefix argument handling)
            \param key  Invoking key */
        AbstractButton(ui::Root& root, util::Key_t key);

        /** Set or clear a button flag.
            \param flag Flag to set or clear
            \param value true to set, false to clear */
        void setFlag(ButtonFlag flag, bool value);

        /** Get current flags.
            \return flags */
        ButtonFlags_t getFlags() const;

        /** Default implementation for handleStateChange.
            Call this from your handleStateChange implementation.
            \param st State
            \param enable Whether state was added or removed */
        void defaultHandleStateChange(State st, bool enable);

        /** Default implementation for handleKey.
            Call this from your handleKey implementation.
            \param key Key
            \param prefix Prefix argument
            \return true if event was handled */
        bool defaultHandleKey(util::Key_t key, int prefix);

        /** Default implementation for handleMouse.
            Call this from your handleMouse implementation.
            \param pt Mouse pointer location
            \param pressedButtons Button and modifier state
            \return true if event was handled */
        bool defaultHandleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Dispatch key activation to another widget.
            If this button is triggered, calls the other widget's handleKey() method.
            \param target Other widget. Lifetime must equal/exceed that of the button. */
        void dispatchKeyTo(Widget& target);

        /** Get associated key.
            \return key */
        util::Key_t getKey() const;

        /** Set associated key.
            \param key Key */
        void setKey(util::Key_t key);

        /** Get associated root.
            \return root */
        ui::Root& root() const;

        /** Signal: regular activation.
            \param prefix Prefix argument */
        afl::base::Signal<void(int)> sig_fire;

        /** Signal: key activation.
            \param prefix Prefix argument.
            \param key Invoking key. If the button is clicked with a modifier being held, this key will include the modifier. */
        afl::base::Signal<void(int, util::Key_t)> sig_fireKey;

     private:
        // Root
        ui::Root& m_root;

        // Associated key
        util::Key_t m_key;

        // Keyboard modifiers that were active when the mouse was pressed in this button
        util::Key_t m_activeModifiers;

        // Additional flags
        ButtonFlags_t m_flags;

        void fire(int arg, util::Key_t key);
    };

} }

#endif
