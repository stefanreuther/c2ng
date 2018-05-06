/**
  *  \file gfx/sdl2/nullpresenter.hpp
  */
#ifndef C2NG_GFX_SDL2_NULLPRESENTER_HPP
#define C2NG_GFX_SDL2_NULLPRESENTER_HPP

#include "gfx/sdl2/presenter.hpp"

namespace gfx { namespace sdl2 {

    class NullPresenter : public Presenter {
     public:
        NullPresenter();
        ~NullPresenter();
        virtual void present(SDL_Surface* sfc, const Rectangle& region);
    };

} }

#endif
