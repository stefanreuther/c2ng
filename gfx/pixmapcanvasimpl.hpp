/**
  *  \file gfx/pixmapcanvasimpl.hpp
  *  \brief Template class gfx::PixmapCanvasImpl
  */
#ifndef C2NG_GFX_PIXMAPCANVASIMPL_HPP
#define C2NG_GFX_PIXMAPCANVASIMPL_HPP

#include "gfx/canvas.hpp"
#include "afl/base/ref.hpp"
#include "gfx/primitives.hpp"

namespace gfx {

    /** Implementation of most Canvas methods for a pixmap.
        This is the common implementation for pixmaps.
        The general idea is to create a Pixmap instance using the given type (PixmapType),
        and then create one or more PixmapCanvasImpl instances referring to it.

        \tparam PixmapType Type of pixmap object, stores the pixel data. Requirements:
        - must be derived from afl::base::RefCounted
        - must have a method <tt>gfx::Point getSize()</tt> to return the size as a point
        - must have a method <tt>int getWidth()</tt> to return the width
        - must have a method <tt>int getHeight()</tt> to return the height
        - must have a method <tt>Memory<Pixel_t> pixels()</tt> to return the entire pixel buffer
        - must have a method <tt>Memory<Pixel_t> row(int n)</tt> to return the pixel buffer for one line

        \tparam TraitsType Framebuffer access traits.
        Requirements are the same as for the parameter to class Primitives,
        with the addition of:
        - must have a constructor that takes PixmapType */
    template<typename PixmapType, typename TraitsType>
    class PixmapCanvasImpl : public Canvas {
     public:
        /** Shortcut for Primitives type. */
        typedef Primitives<TraitsType> Primitives_t;

        /** Shortcut for pixel type. */
        typedef typename Primitives_t::Pixel_t Pixel_t;

        /** Constructor.
            \param pix Pixmap */
        explicit PixmapCanvasImpl(afl::base::Ref<PixmapType> pix)
            : m_pixmap(pix)
            { }

        // Canvas:
        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha);
        virtual void drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha);
        virtual void blit(const Point& pt, Canvas& src, Rectangle rect);
        virtual void blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha);
        virtual Rectangle computeClipRect(Rectangle r);
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors);
        virtual Point getSize();
        virtual bool isVisible(Rectangle r);
        virtual bool isClipped(Rectangle r);

        // To implement in child:
        // virtual int getBitsPerPixel();
        // virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        // virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions);
        // virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        // virtual afl::base::Ptr<Canvas> convertCanvas(afl::base::Ptr<Canvas> orig);

        PixmapType& pixmap();

     private:
        Rectangle getSizeRectangle() const;

        afl::base::Ref<PixmapType> m_pixmap;
    };
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    Rectangle r(pt, Point(npix, 1));
    r.intersect(getSizeRectangle());
    if (r.exists()) {
        Primitives_t(*m_pixmap).doHLine(r.getLeftX(), r.getTopY(), r.getRightX(), Pixel_t(color), pat, alpha);
    }
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
{
    Rectangle r(pt, Point(1, npix));
    r.intersect(getSizeRectangle());
    if (r.exists()) {
        Primitives_t(*m_pixmap).doVLine(r.getLeftX(), r.getTopY(), r.getBottomY(), Pixel_t(color), pat, alpha);
    }
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha)
{
    int x = pt.getX();
    int y = pt.getY();
    if (y >= 0 && y < m_pixmap->getHeight()) {
        // Trim left side
        if (x < 0) {
            colors.split(-x);
            x = 0;
        }

        // Trim right side
        if (x < m_pixmap->getWidth()) {
            colors.trim(m_pixmap->getWidth() - x);
            if (!colors.empty()) {
                Primitives_t(*m_pixmap).writePixels(x, y, colors, alpha);
            }
        }
    }
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha)
{
    rect.intersect(getSizeRectangle());
    if (rect.exists()) {
        Primitives_t(*m_pixmap).doBar(rect, color, bg, pat, alpha);
    }
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::blit(const Point& pt, Canvas& src, Rectangle rect)
{
    defaultBlit(pt, src, rect);
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha)
{
    rect.intersect(getSizeRectangle());
    if (rect.exists()) {
        Primitives_t(*m_pixmap).doBlitPattern(rect, pt, bytesPerLine, data, color, bg, alpha);
    }
}

template<typename PixmapType, typename TraitsType>
gfx::Rectangle
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::computeClipRect(Rectangle r)
{
    r.intersect(getSizeRectangle());
    return r;
}

template<typename PixmapType, typename TraitsType>
void
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::getPixels(Point pt, afl::base::Memory<Color_t> colors)
{
    int x = pt.getX();
    int y = pt.getY();
    if (y >= 0 && y < m_pixmap->getHeight()) {
        // Handle initial out-of-range portion
        if (x < 0) {
            colors.split(-x).fill(0);
            x = 0;
        }

        // Read pixels
        if (x < m_pixmap->getWidth()) {
            afl::base::Memory<Color_t> actual = colors.split(m_pixmap->getWidth() - x);
            if (!actual.empty()) {
                Primitives_t(*m_pixmap).readPixels(x, y, actual);
            }
        }

        // Fill remainder
        colors.fill(0);
    } else {
        colors.fill(0);
    }
}

template<typename PixmapType, typename TraitsType>
gfx::Point
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::getSize()
{
    return m_pixmap->getSize();
}

template<typename PixmapType, typename TraitsType>
bool
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::isVisible(Rectangle r)
{
    return defaultIsVisible(r);
}

template<typename PixmapType, typename TraitsType>
bool
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::isClipped(Rectangle r)
{
    return defaultIsClipped(r);
}

template<typename PixmapType, typename TraitsType>
inline PixmapType&
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::pixmap()
{
    return *m_pixmap;
}

template<typename PixmapType, typename TraitsType>
inline gfx::Rectangle
gfx::PixmapCanvasImpl<PixmapType, TraitsType>::getSizeRectangle() const
{
    return Rectangle(Point(0, 0), m_pixmap->getSize());
}

#endif
