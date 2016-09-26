/**
  *  \file ui/skincolorscheme.hpp
  */
#ifndef C2NG_UI_SKINCOLORSCHEME_HPP
#define C2NG_UI_SKINCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"

namespace ui {

    class ColorSet;
    class ColorScheme;

    class SkinColorScheme : public gfx::ColorScheme {
     public:
        SkinColorScheme(const ColorSet& colors, ColorScheme& uiColorScheme);
        virtual gfx::Color_t getColor(uint32_t index);
        virtual void drawBackground(gfx::Context& ctx, const gfx::Rectangle& area);

     private:
        const ColorSet& m_colors;
        ColorScheme& m_uiColorScheme;
    };
}

#endif
