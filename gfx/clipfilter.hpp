/**
  *  \file gfx/clipfilter.hpp
  *  \brief Class gfx::ClipFilter
  */
#ifndef C2NG_GFX_CLIPFILTER_HPP
#define C2NG_GFX_CLIPFILTER_HPP

#include "gfx/filter.hpp"

namespace gfx {

    /** Clipping filter.
        This class provides drawing with clipping at a rectangle.
        It will make sure that no drawing operation leaves the given rectangle. */
    class ClipFilter : public Filter {
     public:
        /** Constructor.
            \param parent Underlying canvas
            \param r rectangle */
        ClipFilter(Canvas& parent, const Rectangle& r);

        // Canvas:
        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
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
