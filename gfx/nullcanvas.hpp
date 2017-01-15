/**
  *  \file gfx/nullcanvas.hpp
  */
#ifndef C2NG_GFX_NULLCANVAS_HPP
#define C2NG_GFX_NULLCANVAS_HPP

#include "gfx/canvas.hpp"

namespace gfx {

    /** A graphics sink.
        A null canvas ignores all requests. This can be used to 'silence' output. */
    class NullCanvas : public Canvas {
     public:
        NullCanvas()
            { }

        ~NullCanvas();

        virtual void drawHLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawVLine(const Point& pt, int npix, Color_t color, LinePattern_t pat, Alpha_t alpha);
        virtual void drawPixel(const Point& pt, Color_t color, Alpha_t alpha);
        virtual void drawPixels(const Point& pt, afl::base::Memory<const Color_t> colors, Alpha_t alpha);
        virtual void drawBar(Rectangle rect, Color_t color, Color_t bg, const FillPattern& pat, Alpha_t alpha);
        virtual void blit(const Point& pt, Canvas& src, Rectangle rect);
        virtual void blitPattern(Rectangle rect, const Point& pt, int bytesPerLine, const uint8_t* data, Color_t color, Color_t bg, Alpha_t alpha);
        virtual Rectangle computeClipRect(Rectangle r);
        virtual void getPixels(Point pt, afl::base::Memory<Color_t> colors);
        virtual Point getSize();
        virtual int getBitsPerPixel();
        virtual bool isVisible(Rectangle r);
        virtual bool isClipped(Rectangle r);
        virtual void setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual void decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions);
        virtual void encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles);
        virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig);
    };

}

#endif
