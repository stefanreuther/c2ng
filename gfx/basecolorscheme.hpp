/**
  *  \file gfx/basecolorscheme.hpp
  *  \brief Interface gfx::BaseColorScheme
  */
#ifndef C2NG_GFX_BASECOLORSCHEME_HPP
#define C2NG_GFX_BASECOLORSCHEME_HPP

#include "afl/base/deletable.hpp"

namespace gfx {

    class Canvas;
    class Rectangle;

    /** Interface for a color scheme.
        Color schemes are templated on the type of a color index.
        This is the common base class containing methods that do not use a color index. */
    class BaseColorScheme : public afl::base::Deletable {
     public:
        /** Draw background.
            \param can Canvas to draw on
            \param area Area to fill */
        virtual void drawBackground(Canvas& can, const Rectangle& area) = 0;
    };

}

#endif
