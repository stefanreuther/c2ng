/**
  *  \file gfx/clipfilter.cpp
  */

#include "gfx/clipfilter.hpp"

// /** Constructor.
//     \param rect      clipping rectangle
//     \param parent    canvas to draw on */
gfx::ClipFilter::ClipFilter(Canvas& parent, const Rectangle& r)
    : Filter(parent),
      m_rectangle(r)
{
    // ex GfxClipRect::GfxClipRect
}

// Canvas:
void
gfx::ClipFilter::drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    // ex GfxClipRect::drawHLine
    Rectangle r(pt, Point(npix, 1));
    r.intersect(m_rectangle);
    if (r.exists()) {
        parent().drawHLine(r.getTopLeft(), r.getWidth(), color, pat, alpha);
    }
}

void
gfx::ClipFilter::drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    // ex GfxClipRect::drawVLine
    Rectangle r(pt, Point(1, npix));
    r.intersect(m_rectangle);
    if (r.exists()) {
        parent().drawVLine(r.getTopLeft(), r.getHeight(), color, pat, alpha);
    }
}

void
gfx::ClipFilter::drawPixel(const Point& pt, Color_t color, Alpha_t alpha)
{
    // ex GfxClipRect::drawPixel
    if (m_rectangle.contains(pt)) {
        parent().drawPixel(pt, color, alpha);
    }
}

void
gfx::ClipFilter::drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha)
{
    Rectangle r(pt, Point(colors.size(), 1));
    r.intersect(m_rectangle);
    if (r.exists()) {
        parent().drawPixels(r.getTopLeft(), colors.subrange(r.getLeftX() - pt.getX(), r.getWidth()), alpha);
    }
}

void
gfx::ClipFilter::drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha)
{
    // ex GfxClipRect::drawBar
    rect.intersect(m_rectangle);
    if (rect.exists()) {
        parent().drawBar(rect, color, bg, pat, alpha);
    }
}

void
gfx::ClipFilter::blit(const Point& pt, Canvas& src, Rectangle rect)
{
    // ex GfxClipRect::blitSurface
    Rectangle rclip = m_rectangle;
    rclip.moveBy(Point(-pt.getX(), -pt.getY()));
    rect.intersect(rclip);
    if (rect.exists()) {
        parent().blit(pt, src, rect);
    }
}

void
gfx::ClipFilter::blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha)
{
    // ex GfxClipRect::blitPattern
    rect.intersect(m_rectangle);
    if (rect.exists()) {
        parent().blitPattern(rect, pt, bytesPerLine, data, color, bg, alpha);
    }
}

gfx::Rectangle
gfx::ClipFilter::computeClipRect(Rectangle r)
{
    // ex GfxClipRect::computeClipRect
    r.intersect(m_rectangle);
    return parent().computeClipRect(r);
}

bool
gfx::ClipFilter::isVisible(Rectangle r)
{
    return computeClipRect(r).exists();
}

bool
gfx::ClipFilter::isClipped(Rectangle r)
{
    return computeClipRect(r) != r;
}

// FIXME: remove
// bool
// GfxClipRect::computeOffset(GfxCanvas& other, GfxPoint& pt)
// {
//     return this == &other
//         || parent.computeOffset(other, pt);
// }
