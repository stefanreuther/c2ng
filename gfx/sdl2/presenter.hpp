/**
  *  \file gfx/sdl2/presenter.hpp
  */
#ifndef C2NG_GFX_SDL2_PRESENTER_HPP
#define C2NG_GFX_SDL2_PRESENTER_HPP

#include <SDL_surface.h>
#include "gfx/rectangle.hpp"

namespace gfx { namespace sdl2 {

    class Presenter {
     public:
        virtual ~Presenter()
            { }
        virtual void present(SDL_Surface* sfc, const Rectangle& region) = 0;
    };

} }

#endif
