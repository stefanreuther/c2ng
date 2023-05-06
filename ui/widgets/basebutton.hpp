/**
  *  \file ui/widgets/basebutton.hpp
  *  \brief Class ui::widgets::BaseButton
  */
#ifndef C2NG_UI_WIDGETS_BASEBUTTON_HPP
#define C2NG_UI_WIDGETS_BASEBUTTON_HPP

#include "afl/base/signal.hpp"
#include "ui/draw.hpp"                  // ButtonFlag
#include "ui/icons/icon.hpp"
#include "ui/root.hpp"
#include "ui/simplewidget.hpp"
#include "util/key.hpp"

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
        Buttons can optionally be focusable.

        Appearance of the widget is controlled by an Icon instance.
        Users or derived classes must create and manage that instance and call setIcon() to set it.

        Event receivers can attach events to sig_fire and sig_fireKey that differ in their parameters. */
    class BaseButton : public SimpleWidget {
     public:
        /** Constructor.
            \param root Associated UI root (required for prefix argument handling)
            \param key  Invoking key */
        BaseButton(ui::Root& root, util::Key_t key);

        /** Set appearance.
            \param icon Icon to display. Must be managed by caller and live sufficiently long. */
        void setIcon(ui::icons::Icon& icon);

        /** Set or clear a button flag.
            \param flag Flag to set or clear
            \param value true to set, false to clear */
        void setFlag(ButtonFlag flag, bool value);

        /** Get current flags.
            \return flags */
        ButtonFlags_t getFlags() const;

        // Widget/SimpleWidget methods:
        virtual void draw(gfx::Canvas& can);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange();
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        /** Dispatch key activation to key event consumer.
            If this button is triggered, calls the KeyEventConsumer's handleKey() method.
            \param target Consumer. Lifetime must equal/exceed that of the button. */
        void dispatchKeyTo(gfx::KeyEventConsumer& target);

        /** Dispatch key activation to widget, and focus it.
            If this button is triggered, focus the widget and call its handleKey() method.
            \param target Consumer. Lifetime must equal/exceed that of the button. */
        void dispatchKeyAndFocus(Widget& target);

        /** Get associated key.
            \return key */
        util::Key_t getKey() const;

        /** Set associated key.
            \param key Key */
        void setKey(util::Key_t key);

        /** Make this button focusable.
            If the button is focusable, it will request to be focused when clicked,
            and will react on the SPACE key when focused.
            Buttons are not focusable by default.
            \param flag true=focusable */
        void setIsFocusable(bool flag);

        /** Set growth behaviour.
            \param growth Behaviour; default is Fixed */
        void setGrowthBehaviour(ui::layout::Info::Growth growth);

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

        // Focusable flag
        bool m_focusable;

        // Growth behaviour for layout
        ui::layout::Info::Growth m_growthBehaviour;

        // Appearance
        ui::icons::Icon* m_pIcon;

        void fire(int arg, util::Key_t key);
    };

} }

#endif
