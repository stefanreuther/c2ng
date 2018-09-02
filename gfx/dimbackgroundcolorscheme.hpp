/**
  *  \file gfx/dimbackgroundcolorscheme.hpp
  */
#ifndef C2NG_GFX_DIMBACKGROUNDCOLORSCHEME_HPP
#define C2NG_GFX_DIMBACKGROUNDCOLORSCHEME_HPP

#include "util/skincolor.hpp"
#include "gfx/canvas.hpp"
#include "gfx/colorscheme.hpp"

namespace gfx {

    /** Color scheme: dim background.
        This color scheme forwards another color scheme's colors, but dims its background.

        It works by caching the original background in a pixmap.
        It is therefore intended for pixmap background.
        For a solid-color background, better just override the original color scheme's drawBackground(). */
    class DimBackgroundColorScheme : public ColorScheme<util::SkinColor::Color> {
     public:
        /** Constructor.
            \param parent Parent color scheme providing colors and original background */
        explicit DimBackgroundColorScheme(ColorScheme<util::SkinColor::Color>& parent);

        // ColorScheme
        virtual Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(Canvas& can, const Rectangle& area);
     private:
        ColorScheme<util::SkinColor::Color>& m_parent;
        afl::base::Ptr<Canvas> m_cachedBackground;
        Rectangle m_cachedSize;
    };

}

#endif
