/**
  *  \file gfx/pixmapcanvasimpl.hpp
  */
#ifndef C2NG_GFX_PIXMAPCANVASIMPL_HPP
#define C2NG_GFX_PIXMAPCANVASIMPL_HPP

#include "gfx/canvas.hpp"
#include "afl/base/ref.hpp"
#include "gfx/primitives.hpp"

namespace gfx {

    template<typename PixmapType, typename TraitsType>
    class PixmapCanvasImpl : public Canvas {
     public:
        typedef Primitives<TraitsType> Primitives_t;

        PixmapCanvasImpl(afl::base::Ref<PixmapType> pix)
            : m_pixmap(pix)
            { }

        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
            {
                Rectangle r(pt, Point(npix, 1));
                r.intersect(getSizeRectangle());
                if (r.exists()) {
                    Primitives_t(*m_pixmap).doHLine(r.getLeftX(), r.getTopY(), r.getRightX(), color, pat, alpha);
                }
            }
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha)
            {
                Rectangle r(pt, Point(1, npix));
                r.intersect(getSizeRectangle());
                if (r.exists()) {
                    Primitives_t(*m_pixmap).doVLine(r.getLeftX(), r.getTopY(), r.getBottomY(), color, pat, alpha);
                }
            }
        virtual void drawPixel(const Point& pt, Color_t color, Alpha_t alpha)
            {
                Color_t pix[1] = {color};
                drawPixels(pt, pix, alpha);
            }
        virtual void drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha)
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
        virtual void drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha)
            {
                rect.intersect(getSizeRectangle());
                if (rect.exists()) {
                    Primitives_t(*m_pixmap).doBar(rect, color, bg, pat, alpha);
                }
            }
        virtual void blit(const Point& pt, Canvas& src, Rectangle rect)
            {
                defaultBlit(pt, src, rect);
            }
        virtual void blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha)
            {
                rect.intersect(getSizeRectangle());
                if (rect.exists()) {
                    Primitives_t(*m_pixmap).doBlitPattern(rect, pt, bytesPerLine, data, color, bg, alpha);
                }
            }
        virtual Rectangle computeClipRect(Rectangle r)
            {
                r.intersect(getSizeRectangle());
                return r;
            }
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors)
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
        virtual Point getSize()
            {
                return m_pixmap->getSize();
            }
        // virtual int getBitsPerPixel();
        virtual bool isVisible(Rectangle r)
            {
                return computeClipRect(r).exists();
            }
        virtual bool isClipped(Rectangle r)
            {
                return computeClipRect(r) == r;
            }
        // virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        // virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions);
        // virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        // virtual afl::base::Ptr<Canvas> convertCanvas(afl::base::Ptr<Canvas> orig);

        PixmapType& pixmap()
            { return *m_pixmap; }

     private:
        Rectangle getSizeRectangle() const
            {
                return Rectangle(Point(0, 0), m_pixmap->getSize());
            }

        afl::base::Ref<PixmapType> m_pixmap;
    };
}

#endif
