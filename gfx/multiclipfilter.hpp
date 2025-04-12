/**
  *  \file gfx/multiclipfilter.hpp
  */
#ifndef C2NG_GFX_MULTICLIPFILTER_HPP
#define C2NG_GFX_MULTICLIPFILTER_HPP

#include "gfx/filter.hpp"
#include "gfx/rectangleset.hpp"

namespace gfx {

    class MultiClipFilter : public Filter {
     public:
        MultiClipFilter(Canvas& parent);
        ~MultiClipFilter();

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

        // MultiClipFilter:
        void add(const Rectangle& r);
        void remove(const Rectangle& r);
        void clipRegionAtRectangle(const Rectangle& r);
        void clear();
        bool empty() const;

     private:
        RectangleSet m_set;
    };

}

#endif
