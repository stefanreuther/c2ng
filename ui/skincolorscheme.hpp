/**
  *  \file ui/skincolorscheme.hpp
  */
#ifndef C2NG_UI_SKINCOLORSCHEME_HPP
#define C2NG_UI_SKINCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"
#include "util/skincolor.hpp"

namespace ui {

    class ColorSet;
    class ColorScheme;

    class SkinColorScheme : public gfx::ColorScheme<util::SkinColor::Color> {
     public:
        SkinColorScheme(const ColorSet& colors, ui::ColorScheme& uiColorScheme);
        virtual gfx::Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area);

     private:
        const ColorSet& m_colors;
        ui::ColorScheme& m_uiColorScheme;
    };
}

#endif
