/**
  *  \file test/gfx/canvastest.cpp
  *  \brief Test for gfx::Canvas
  */

#include "gfx/canvas.hpp"
#include "afl/test/testrunner.hpp"

/* Interface test */
AFL_TEST_NOARG("gfx.Canvas")
{
    using gfx::Point;
    using gfx::Rectangle;
    using gfx::Color_t;
    using gfx::LinePattern_t;
    using gfx::Alpha_t;
    using gfx::FillPattern;
    using gfx::ColorQuad_t;
    class Tester : public gfx::Canvas {
     public:
        virtual void drawHLine(const Point& /*pt*/, int /*npix*/, Color_t /*color*/, LinePattern_t /*pat*/, Alpha_t /*alpha*/)
            { }
        virtual void drawVLine(const Point& /*pt*/, int /*npix*/, Color_t /*color*/, LinePattern_t /*pat*/, Alpha_t /*alpha*/)
            { }
        virtual void drawPixel(const Point& /*pt*/, Color_t /*color*/, Alpha_t /*alpha*/)
            { }
        virtual void drawPixels(const Point& /*pt*/, afl::base::Memory<const Color_t> /*colors*/, Alpha_t /*alpha*/)
            { }
        virtual void drawBar(Rectangle /*rect*/, Color_t /*color*/, Color_t /*bg*/, const FillPattern& /*pat*/, Alpha_t /*alpha*/)
            { }
        virtual void blit(const Point& /*pt*/, Canvas& /*src*/, Rectangle /*rect*/)
            { }
        virtual void blitPattern(Rectangle /*rect*/, const Point& /*pt*/, int /*bytesPerLine*/, const uint8_t* /*data*/, Color_t /*color*/, Color_t /*bg*/, Alpha_t /*alpha*/)
            { }
        virtual Rectangle computeClipRect(Rectangle /*r*/)
            { return Rectangle(); }
        virtual void getPixels(Point /*pt*/, afl::base::Memory<Color_t> /*colors*/)
            { }
        virtual Point getSize()
            { return Point(); }
        virtual int getBitsPerPixel()
            { return 0; }
        virtual bool isVisible(Rectangle /*r*/)
            { return false; }
        virtual bool isClipped(Rectangle /*r*/)
            { return false; }
        virtual void setPalette(Color_t /*start*/, afl::base::Memory<const ColorQuad_t> /*colorDefinitions*/, afl::base::Memory<Color_t> /*colorHandles*/)
            { }
        virtual void decodeColors(afl::base::Memory<const Color_t> /*colorHandles*/, afl::base::Memory<ColorQuad_t> /*colorDefinitions*/)
            { }
        virtual void encodeColors(afl::base::Memory<const ColorQuad_t> /*colorDefinitions*/, afl::base::Memory<Color_t> /*colorHandles*/)
            { }
        virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig)
            { return orig; }
    };
    Tester t;
}
