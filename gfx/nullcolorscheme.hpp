/**
  *  \file gfx/nullcolorscheme.hpp
  */
#ifndef C2NG_GFX_NULLCOLORSCHEME_HPP
#define C2NG_GFX_NULLCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"

namespace gfx {

    class NullColorScheme : public ColorScheme {
     public:
        virtual ~NullColorScheme();

        virtual Color_t getColor(uint32_t index);
        virtual void drawBackground(Context& ctx, const Rectangle& area);

        static NullColorScheme instance;
    };

}

#endif
