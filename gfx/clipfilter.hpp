/**
  *  \file gfx/clipfilter.hpp
  */
#ifndef C2NG_GFX_CLIPFILTER_HPP
#define C2NG_GFX_CLIPFILTER_HPP

#include "gfx/filter.hpp"

namespace gfx {

// /** \class GfxClipRect
//     \brief Clip at a Rectangle

//     This class provides the back-end for drawing with clipping
//     at a rectangle.

//     It does not contain a public constructor or methods to modify the
//     clip rectangle; derived classes must implement these when needed. */
    class ClipFilter : public Filter {
     public:
        ClipFilter(Canvas& parent, const Rectangle& r);

        // Canvas:
        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawPixel(const Point& pt, Color_t color, Alpha_t alpha);
        virtual void drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha);
        virtual void drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha);
        virtual void blit(const Point& pt, Canvas& src, Rectangle rect);
        virtual void blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha);
        virtual Rectangle computeClipRect(Rectangle r);
        virtual bool isVisible(Rectangle r);
        virtual bool isClipped(Rectangle r);

     private:
        Rectangle m_rectangle;
    };

}

#endif
