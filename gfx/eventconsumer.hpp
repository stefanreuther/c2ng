/**
  *  \file gfx/eventconsumer.hpp
  *  \brief Interface gfx::EventConsumer
  */
#ifndef C2NG_GFX_EVENTCONSUMER_HPP
#define C2NG_GFX_EVENTCONSUMER_HPP

#include "afl/bits/smallset.hpp"
#include "gfx/keyeventconsumer.hpp"
#include "gfx/point.hpp"

namespace gfx {

    class EventConsumer : public KeyEventConsumer {
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

        /** Handle mouse movement.
            \param pt Mouse location or movement
            \param buttonState Button state */
        virtual bool handleMouse(Point pt, MouseButtons_t pressedButtons) = 0;
    };

}

#endif
