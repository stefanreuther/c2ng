/**
  *  \file gfx/dimbackgroundcolorscheme.cpp
  */

#include "gfx/dimbackgroundcolorscheme.hpp"
#include "gfx/rgbapixmap.hpp"
#include "gfx/complex.hpp"
#include "gfx/basecontext.hpp"

namespace {
    // FIXME: make configurable?
    const int TRANSPARENCY = 180;
}

gfx::DimBackgroundColorScheme::DimBackgroundColorScheme(ColorScheme<util::SkinColor::Color>& parent)
    : m_parent(parent),
      m_cachedBackground(),
      m_cachedSize()
{ }

gfx::Color_t
gfx::DimBackgroundColorScheme::getColor(util::SkinColor::Color index)
{
    return m_parent.getColor(index);
}

void
gfx::DimBackgroundColorScheme::drawBackground(Canvas& can, const Rectangle& area)
{
    // We cannot draw beyond (0,0)
    Rectangle adjustedArea(area);
    if (adjustedArea.getLeftX() < 0) {
        adjustedArea.consumeX(-adjustedArea.getLeftX());
    }
    if (adjustedArea.getTopY() < 0) {
        adjustedArea.consumeY(-adjustedArea.getTopY());
    }

    // Check cached pixmap
    if (m_cachedBackground.get() == 0 || !m_cachedSize.contains(adjustedArea)) {
        // We have not cached anything, or the request asks for pixels outside our cache.
        // Create a new pixmap.
        // The origin must always be (0,0), so include that point.
        // FIXME: this is inefficient if this is only a small area far from (0,0).
        m_cachedSize.include(adjustedArea);
        m_cachedSize.include(gfx::Point(0, 0));
        afl::base::Ref<RGBAPixmap> pix(RGBAPixmap::create(m_cachedSize.getWidth(), m_cachedSize.getHeight()));
        m_cachedBackground = pix->makeCanvas().asPtr();

        // Draw original background.
        m_parent.drawBackground(*m_cachedBackground, m_cachedSize);

        // Draw a transparent black bar atop.
        m_cachedBackground->drawBar(m_cachedSize, COLORQUAD_FROM_RGBA(0, 0, 0, OPAQUE_ALPHA), TRANSPARENT_COLOR, FillPattern::SOLID, TRANSPARENCY);
    }

    // Draw
    blitTiledAnchored(BaseContext(can), area, *m_cachedBackground, m_cachedSize.getTopLeft(), 0);
}
