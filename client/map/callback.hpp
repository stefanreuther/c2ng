/**
  *  \file client/map/callback.hpp
  */
#ifndef C2NG_CLIENT_MAP_CALLBACK_HPP
#define C2NG_CLIENT_MAP_CALLBACK_HPP

#include "gfx/rectangle.hpp"

namespace client { namespace map {

    class Overlay;

    class Callback {
     public:
        // FIXME: need this one in the interface? virtual void addOverlay(Overlay& over) = 0;
        virtual void removeOverlay(Overlay& over) = 0;

        virtual void requestRedraw() = 0;
        virtual void requestRedraw(gfx::Rectangle& area) = 0;

     protected:
        virtual ~Callback()
            { }
    };

} }

#endif
