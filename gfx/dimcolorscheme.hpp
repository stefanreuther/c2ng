/**
  *  \file gfx/dimcolorscheme.hpp
  */
#ifndef C2NG_GFX_DIMCOLORSCHEME_HPP
#define C2NG_GFX_DIMCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"
#include "util/skincolor.hpp"

namespace gfx {

    class Canvas;

    /** Color scheme: dim foreground.
        This color scheme forwards another color scheme's colors, mixed into the background color.
        This is intented for rendering disabled widgets. */
    class DimColorScheme : public ColorScheme<util::SkinColor::Color> {
     public:
        /** Constructor.
            \param parent Parent color schem providing original colors to use
            \param can Drawing canvas */
        DimColorScheme(gfx::ColorScheme<util::SkinColor::Color>& parent, Canvas& can);

        virtual Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(Canvas& can, const Rectangle& area);

     private:
        ColorScheme<util::SkinColor::Color>& m_parent;
        Canvas& m_canvas;
    };

}

#endif
