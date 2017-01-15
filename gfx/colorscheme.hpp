/**
  *  \file gfx/colorscheme.hpp
  */
#ifndef C2NG_GFX_COLORSCHEME_HPP
#define C2NG_GFX_COLORSCHEME_HPP

#include "gfx/basecolorscheme.hpp"
#include "gfx/types.hpp"

namespace gfx {

    template<typename Index>
    class ColorScheme : public BaseColorScheme {
     public:
        virtual Color_t getColor(Index index) = 0;
    };

}

#endif
