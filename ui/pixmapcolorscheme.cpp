/**
  *  \file ui/pixmapcolorscheme.cpp
  */

#include "ui/pixmapcolorscheme.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/complex.hpp"
#include "ui/draw.hpp"
#include "ui/skincolorscheme.hpp"

ui::PixmapColorScheme::PixmapColorScheme(Root& root, afl::base::Ref<gfx::Canvas> pixmap)
    : m_root(root),
      m_pixmap(pixmap)
{ }

ui::PixmapColorScheme::~PixmapColorScheme()
{ }

gfx::Color_t
ui::PixmapColorScheme::getColor(util::SkinColor::Color index)
{
    return SkinColorScheme(BLACK_COLOR_SET, m_root.colorScheme()).getColor(index);
}

void
ui::PixmapColorScheme::drawBackground(gfx::Canvas& can, const gfx::Rectangle& area)
{
    gfx::blitTiledAnchored(gfx::BaseContext(can), area, *m_pixmap, gfx::Point(0, 0), 0);
}
