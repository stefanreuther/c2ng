/**
  *  \file gfx/eventconsumer.hpp
  */
#ifndef C2NG_GFX_EVENTCONSUMER_HPP
#define C2NG_GFX_EVENTCONSUMER_HPP

#include "util/key.hpp"
#include "gfx/point.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/base/deletable.hpp"

namespace gfx {

    class EventConsumer : public afl::base::Deletable {
     public:
        enum MouseButton {
            LeftButton,
            RightButton,
            MiddleButton,
            DoubleClick,
            ShiftKey,
            CtrlKey,
            AltKey,
            MetaKey
        };
        typedef afl::bits::SmallSet<MouseButton> MouseButtons_t;

        /** Handle keypress.
            \param key Key that was pressed
            \param prefix Prefix (repeat) count */
        virtual bool handleKey(util::Key_t key, int prefix) = 0;

        /** Handle mouse movement.
            \param pt Mouse location or movement
            \param buttonState Button state */
        virtual bool handleMouse(Point pt, MouseButtons_t pressedButtons) = 0;
    };

}

#endif
