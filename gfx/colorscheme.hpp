/**
  *  \file gfx/colorscheme.hpp
  */
#ifndef C2NG_GFX_COLORSCHEME_HPP
#define C2NG_GFX_COLORSCHEME_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "gfx/types.hpp"

namespace gfx {

    class Context;
    class Rectangle;

    class ColorScheme : public afl::base::Deletable {
     public:
        virtual ~ColorScheme();

        virtual Color_t getColor(uint32_t index) = 0;
        virtual void drawBackground(Context& ctx, const Rectangle& area) = 0;
    };

}

#endif
