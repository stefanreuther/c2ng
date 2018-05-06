/**
  *  \file ui/pixmapcolorscheme.hpp
  */
#ifndef C2NG_UI_PIXMAPCOLORSCHEME_HPP
#define C2NG_UI_PIXMAPCOLORSCHEME_HPP

#include "gfx/colorscheme.hpp"
#include "util/skincolor.hpp"
#include "gfx/canvas.hpp"
#include "ui/root.hpp"

namespace ui {

    class PixmapColorScheme : public gfx::ColorScheme<util::SkinColor::Color> {
     public:
        PixmapColorScheme(Root& root, afl::base::Ref<gfx::Canvas> pixmap);
        ~PixmapColorScheme();

        virtual gfx::Color_t getColor(util::SkinColor::Color index);
        virtual void drawBackground(gfx::Canvas& can, const gfx::Rectangle& area);

     private:
        Root& m_root;
        afl::base::Ref<gfx::Canvas> m_pixmap;
    };

}

#endif
