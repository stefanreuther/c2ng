/**
  *  \file gfx/colorscheme.hpp
  *  \brief Interface gfx::ColorScheme
  */
#ifndef C2NG_GFX_COLORSCHEME_HPP
#define C2NG_GFX_COLORSCHEME_HPP

#include "gfx/basecolorscheme.hpp"
#include "gfx/types.hpp"

namespace gfx {

    /** Interface for a concrete color scheme.
        Color schemes are templated on the type of a color index,
        for example "color in a well-known palette" or "color matching a skin color set".
        A color scheme is specific to a Canvas (or family there-of) and produces canvas colors.
        \tparam Index color index */
    template<typename Index>
    class ColorScheme : public BaseColorScheme {
     public:
        /** Convert color into canvas color.
            \param index Color index
            \return canvas color */
        virtual Color_t getColor(Index index) = 0;
    };

}

#endif
