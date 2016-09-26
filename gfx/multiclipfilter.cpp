/**
  *  \file gfx/multiclipfilter.cpp
  */

#include "gfx/multiclipfilter.hpp"

gfx::MultiClipFilter::MultiClipFilter(Canvas& parent)
    : Filter(parent),
      m_set()
{ }

gfx::MultiClipFilter::~MultiClipFilter()
{ }

// Canvas:
void
gfx::MultiClipFilter::drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle r(pt, Point(npix, 1));
        r.intersect(*i);
        if (r.exists()) {
            parent().drawHLine(r.getTopLeft(), r.getWidth(), color, pat, alpha);
        }
    }
}

void
gfx::MultiClipFilter::drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle r(pt, Point(1, npix));
        r.intersect(*i);
        if (r.exists()) {
            parent().drawVLine(r.getTopLeft(), r.getHeight(), color, pat, alpha);
        }
    }
}

void
gfx::MultiClipFilter::drawPixel(const Point& pt, Color_t color, Alpha_t alpha)
{
    if (m_set.contains(pt)) {
        parent().drawPixel(pt, color, alpha);
    }
}

void
gfx::MultiClipFilter::drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle r(pt, Point(colors.size(), 1));
        r.intersect(*i);
        if (r.exists()) {
            parent().drawPixels(r.getTopLeft(), colors.subrange(r.getLeftX() - pt.getX(), r.getWidth()), alpha);
        }
    }
}

void
gfx::MultiClipFilter::drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle r = rect;
        r.intersect(*i);
        if (r.exists()) {
            parent().drawBar(r, color, bg, pat, alpha);
        }
    }
}

void
gfx::MultiClipFilter::blit(const Point& pt, Canvas& src, Rectangle rect)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle rclip = *i, r = rect;
        rclip.moveBy(Point(-pt.getX(), -pt.getY()));
        r.intersect(rclip);
        if (r.exists()) {
            parent().blit(pt, src, r);
        }
    }
}

void
gfx::MultiClipFilter::blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle r = rect;
        r.intersect(*i);
        if (r.exists()) {
            parent().blitPattern(r, pt, bytesPerLine, data, color, bg, alpha);
        }
    }
}

gfx::Rectangle
gfx::MultiClipFilter::computeClipRect(Rectangle r)
{
    // Compute smallest rectangle c such that bar(c) and bar(r) have the same effect.
    Rectangle u;
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle pi = *i;
        pi.intersect(r);
        u.include(pi);
    }
    return u;
}

bool
gfx::MultiClipFilter::isVisible(Rectangle r)
{
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        Rectangle x = *i;
        x.intersect(r);
        if (x.exists()) {
            return true;
        }
    }
    return false;
}

bool
gfx::MultiClipFilter::isClipped(Rectangle r)
{
    // true iff r is only partially visible, that is, we want to check that
    //        r AND rect == r
    // i.e.   r - rect == 0
    RectangleSet set(r);
    for (RectangleSet::Iterator_t i = m_set.begin(); i != m_set.end(); ++i) {
        set.remove(*i);
    }
    return !set.empty();
}


// MultiClipFilter:
/** Add rectangle. Points within \c r will then be visible. */
void
gfx::MultiClipFilter::add(const Rectangle& r)
{
    m_set.add(r);
}

/** Remove rectangle. Points within \c r will then not be accessible any longer. */
void
gfx::MultiClipFilter::remove(const Rectangle& r)
{
    m_set.remove(r);
}

/** Clip region at rectangle. Points outside \c r will no longer be accessible. */
void
gfx::MultiClipFilter::clipRegionAtRectangle(const Rectangle& r)
{
    m_set.intersect(r);
}

void
gfx::MultiClipFilter::clear()
{
    m_set.clear();
}

bool
gfx::MultiClipFilter::empty() const
{
    return m_set.empty();
}
