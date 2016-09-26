/**
  *  \file ui/widgets/abstractbutton.hpp
  */
#ifndef C2NG_UI_WIDGETS_ABSTRACTBUTTON_HPP
#define C2NG_UI_WIDGETS_ABSTRACTBUTTON_HPP

#include "ui/simplewidget.hpp"
#include "ui/draw.hpp"
#include "afl/base/signal.hpp"

namespace ui { namespace widgets {

// /** \class UIAbstractButton
//     \brief Standard Button

//     This class provides the event handling mechanisms of a standard
//     clickable button (checkbox, radiobutton, ...). The button can be
//     have an associated hot-key if so desired.

//     Buttons are triggered by a keystroke or mouse click.

//     Derived classes must override draw().

//     Buttons fire a command, a stop, or a keystroke. You can attach
//     own handlers to sig_fire. */

    /*
     *  Transition from old methods:
     *     addCommand -> retire, replace by event listener
     *     addStop -> replace by sig_fire.addNewClosure(loop.makeStop(n))
     *     addKey -> 
     */
    class AbstractButton : public SimpleWidget {
     public:
        AbstractButton(util::Key_t key);

        void setFlag(ButtonFlag flag, bool value);
        ButtonFlags_t getFlags() const;

        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

        afl::base::Signal<void(int)> sig_fire;
        afl::base::Signal<void(int, util::Key_t)> sig_fireKey;

     private:
        util::Key_t m_key;
        util::Key_t m_activeModifiers;

        ButtonFlags_t m_flags;

        void fire(int arg, util::Key_t key);
//  public:
//     enum {
//         bf_Pressed       = st_Last*2,     //!< Button is currently pressed.
//         bf_Highlight     = st_Last*4,     //!< Button is highlighted ("on").
//         bf_Key           = st_Last*8,     //!< This is a "key" button. That is, it generates a keystroke and does not react upon keystrokes.
//         bf_SmallSize     = st_Last*16,    //!< Use "small" font. FIXME: this belongs into UIButton.
//         bf_PressOverride = st_Last*32,    //!< Button should appear depressed. FIXME: this belongs into UIButton.
//         bf_LeftJust      = st_Last*64     //!< Left-justify text. FIXME: this belongs into UIButton.
//     };

//     /* Constructors */
//     UIAbstractButton();
//     UIAbstractButton(UIKey key);
//     UIAbstractButton(int id, UIKey key);
//     ~UIAbstractButton();

//     /* Button handlers */
//     UIAbstractButton& addCommand();
//     UIAbstractButton& addStop();
//     UIAbstractButton& addKey();

//     template<typename T> UIAbstractButton& addCall(T* object, void (T::*function)(int))
//         { sig_fire.add(object, function); return *this; }
//     template<typename T> UIAbstractButton& addCall(T* object, void (T::*function)())
//         { sig_fire.add(object, function); return *this; }

//     /* Event handlers */
//     bool handleEvent(const UIEvent& e, bool second_pass);
//     bool handleCommand(const UICommand& cmd, UIBaseWidget& sender);
//     void onStateChange(int astate, bool enable);

//     UIKey getKey() const;
//     void setKey(UIKey);

// };

// inline UIKey
// UIAbstractButton::getKey() const
// {
//     return key;
// }

// inline void
// UIAbstractButton::setKey(UIKey key)
// {
//     this->key = key;
// }

// #endif
    };
    

} }

#endif
