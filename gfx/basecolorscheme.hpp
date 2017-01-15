/**
  *  \file gfx/basecolorscheme.hpp
  */
#ifndef C2NG_GFX_BASECOLORSCHEME_HPP
#define C2NG_GFX_BASECOLORSCHEME_HPP

#include "afl/base/deletable.hpp"

namespace gfx {

    class Canvas;
    class Rectangle;

    class BaseColorScheme : public afl::base::Deletable {
     public:
        virtual void drawBackground(Canvas& can, const Rectangle& area) = 0;
    };

}

#endif
