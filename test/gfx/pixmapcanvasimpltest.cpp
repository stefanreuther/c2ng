/**
  *  \file test/gfx/pixmapcanvasimpltest.cpp
  *  \brief Test for gfx::PixmapCanvasImpl
  */

#include "gfx/pixmapcanvasimpl.hpp"

#include "afl/base/refcounted.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/types.hpp"

namespace {
    /*
     *  Pixmap type implementation for test
     */
    class PixmapImpl : public afl::base::RefCounted {
        static const int WIDTH = 10;
        static const int HEIGHT = 12;
     public:
        gfx::Point getSize() const
            { return gfx::Point(WIDTH, HEIGHT); }
        int getWidth() const
            { return WIDTH; }
        int getHeight() const
            { return HEIGHT; }
        afl::base::Bytes_t pixels()
            { return afl::base::Bytes_t(m_pixels); }
        afl::base::Bytes_t row(int n)
            { return pixels().subrange(WIDTH*n, WIDTH); }

     private:
        uint8_t m_pixels[WIDTH*HEIGHT];
    };

    /*
     *  Traits type implementation for test
     */
    class TraitsImpl {
     public:
        typedef uint8_t Pixel_t;
        typedef uint8_t Data_t;

        Data_t* get(int x, int y) const
            { return m_pix.row(y).at(x); }
        static inline Pixel_t peek(Data_t* ptr)
            { return *ptr; }
        static inline void poke(Data_t* ptr, Pixel_t val)
            { *ptr = val; }
        Pixel_t mix(Pixel_t a, Pixel_t b, gfx::Alpha_t balpha) const
            { return gfx::mixColorComponent(a, b, balpha); }
        inline Data_t* add(Data_t* ptr, int dx, int dy) const
            { return ptr + m_pix.getWidth()*dy + dx; }

        TraitsImpl(PixmapImpl& pix)
            : m_pix(pix)
            { }

     private:
        PixmapImpl& m_pix;
    };

    /*
     *  Canvas type implementation for test, using pixmap and traits type from above.
     *  This completes the PixmapCanvasImpl to an instantiatable object type.
     */
    class CanvasImpl : public gfx::PixmapCanvasImpl<PixmapImpl, TraitsImpl> {
     public:
        CanvasImpl(afl::base::Ref<PixmapImpl> p)
            : gfx::PixmapCanvasImpl<PixmapImpl, TraitsImpl>(p)
            { }
        virtual int getBitsPerPixel()
            { return 8; }
        virtual void setPalette(gfx::Color_t /*start*/, afl::base::Memory<const gfx::ColorQuad_t> colorDefinitions, afl::base::Memory<gfx::Color_t> colorHandles)
            { encodeColors(colorDefinitions, colorHandles); }
        virtual void decodeColors(afl::base::Memory<const gfx::Color_t> colorHandles, afl::base::Memory<gfx::ColorQuad_t> colorDefinitions)
            {
                while (const gfx::Color_t* color = colorHandles.eat()) {
                    if (gfx::ColorQuad_t* def = colorDefinitions.eat()) {
                        uint8_t c = static_cast<uint8_t>(*color);
                        *def = COLORQUAD_FROM_RGBA(c, c, c, gfx::OPAQUE_ALPHA);
                    }
                }
                colorDefinitions.fill(COLORQUAD_FROM_RGBA(0,0,0,0));
            }
        virtual void encodeColors(afl::base::Memory<const gfx::ColorQuad_t> colorDefinitions, afl::base::Memory<gfx::Color_t> colorHandles)
            {
                while (const gfx::ColorQuad_t* def = colorDefinitions.eat()) {
                    if (gfx::Color_t* color = colorHandles.eat()) {
                        *color = RED_FROM_COLORQUAD(*def);
                    }
                }
                colorHandles.fill(COLORQUAD_FROM_RGBA(0,0,0,0));
            }
        virtual afl::base::Ref<gfx::Canvas> convertCanvas(afl::base::Ref<gfx::Canvas> orig)
            {
                // FIXME: can we do better?
                return orig;
            }
    };
}

/** Simple test. */
AFL_TEST("gfx.PixmapCanvasImpl", a)
{
    // Environment: a pixmap
    afl::base::Ref<PixmapImpl> p(*new PixmapImpl());
    p->pixels().fill(0);

    // Testee
    CanvasImpl testee(p);

    // drawHLine
    {
        testee.drawHLine(gfx::Point(2, 3), 10, 7, 0xE5, gfx::OPAQUE_ALPHA);
        testee.drawHLine(gfx::Point(3, 5), 10, 8, 0xFF, gfx::OPAQUE_ALPHA);
        static const uint8_t EXPECTED[] = {
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
            0,0,7,0,0,7,0,7,7,7,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,8,8,8,8,8,8,8,
        };
        a.checkEqualContent<uint8_t>("01. drawHLine", p->pixels().subrange(0, 6*10), EXPECTED);
    }

    // drawVLine
    {
        testee.drawVLine(gfx::Point(1, 1), 4, 9, 0x55, gfx::OPAQUE_ALPHA);
        testee.drawVLine(gfx::Point(0, 0), 3, 2, 0xFF, gfx::OPAQUE_ALPHA);
        static const uint8_t EXPECTED[] = {
            2,0,0,0,0,0,0,0,0,0,
            2,9,0,0,0,0,0,0,0,0,
            2,0,0,0,0,0,0,0,0,0,
            0,9,7,0,0,7,0,7,7,7,
            0,0,0,0,0,0,0,0,0,0,
            0,0,0,8,8,8,8,8,8,8,
        };
        a.checkEqualContent<uint8_t>("02. drawVLine", p->pixels().subrange(0, 6*10), EXPECTED);
    }

    // drawPixels
    {
        static const gfx::Color_t pixels[] = {1,2,3};
        testee.drawPixels(gfx::Point(6, 6), afl::base::Memory<const gfx::Color_t>::fromSingleObject(6), gfx::OPAQUE_ALPHA);
        testee.drawPixels(gfx::Point(7, 6), afl::base::Memory<const gfx::Color_t>::fromSingleObject(6), 128);
        testee.drawPixels(gfx::Point(6, 7), pixels, gfx::OPAQUE_ALPHA);
        testee.drawPixels(gfx::Point(6, 8), pixels, 128);

        static const uint8_t EXPECTED[] = {
            0,0,0,8,8,8,8,8,8,8,
            0,0,0,0,0,0,6,3,0,0,
            0,0,0,0,0,0,1,2,3,0,
            0,0,0,0,0,0,0,1,1,0,
        };
        a.checkEqualContent<uint8_t>("03. drawPixels", p->pixels().subrange(5*10, 4*10), EXPECTED);
    }

    // drawBar
    {
        testee.drawBar(gfx::Rectangle(0, 0, 1000, 1000), 1, 0, gfx::FillPattern::SOLID,  gfx::OPAQUE_ALPHA);
        testee.drawBar(gfx::Rectangle(1, 1, 3, 4),       5, 6, gfx::FillPattern::GRAY25, gfx::OPAQUE_ALPHA);
        static const uint8_t EXPECTED[] = {
            1,1,1,1,1,1,1,1,1,1,
            1,6,6,6,1,1,1,1,1,1,
            1,5,6,5,1,1,1,1,1,1,
            1,6,6,6,1,1,1,1,1,1,
            1,6,5,6,1,1,1,1,1,1,
            1,1,1,1,1,1,1,1,1,1,
        };
        a.checkEqualContent<uint8_t>("04. drawBar", p->pixels().subrange(0, 6*10), EXPECTED);
    }

    // blitPattern
    {
        static const uint8_t pat[] = { 0x80, 0x40, 0x20, 0x90 };
        testee.blitPattern(gfx::Rectangle(5, 1, 4, 4), gfx::Point(5, 1), 1, pat, 2, 0, gfx::OPAQUE_ALPHA);
        static const uint8_t EXPECTED[] = {
            1,1,1,1,1,1,1,1,1,1,
            1,6,6,6,1,2,0,0,0,1,
            1,5,6,5,1,0,2,0,0,1,
            1,6,6,6,1,0,0,2,0,1,
            1,6,5,6,1,2,0,0,2,1,
            1,1,1,1,1,1,1,1,1,1,
        };
        a.checkEqualContent<uint8_t>("05. blitPattern", p->pixels().subrange(0, 6*10), EXPECTED);
    }

    // getPixels
    {
        gfx::Color_t pix[5];
        testee.getPixels(gfx::Point(1, 2), pix);
        a.checkEqual("01. getPixels", pix[0], 5U);
        a.checkEqual("02. getPixels", pix[1], 6U);
        a.checkEqual("03. getPixels", pix[2], 5U);
        a.checkEqual("04. getPixels", pix[3], 1U);
        a.checkEqual("05. getPixels", pix[4], 0U);
    }

    // computeClipRect etc.
    a.checkEqual("11. computeClipRect", testee.computeClipRect(gfx::Rectangle(0, 0, 1000, 1000)), gfx::Rectangle(0, 0, 10, 12));
    a.check("12. isVisible",            testee.isVisible(gfx::Rectangle(0, 0, 1000, 1000)));
    a.check("13. isVisible",           !testee.isVisible(gfx::Rectangle(100, 100, 2, 2)));
    a.check("14. isClipped",            testee.isClipped(gfx::Rectangle(0, 0, 1000, 1000)));
    a.check("15. isClipped",           !testee.isClipped(gfx::Rectangle(3, 4, 2, 2)));
    a.checkEqual("16. getSize",         testee.getSize(), gfx::Point(10, 12));


    // blit
    {
        afl::base::Ref<PixmapImpl> otherPixmap(*new PixmapImpl());
        otherPixmap->pixels().fill(0);
        CanvasImpl other(otherPixmap);
        other.blit(gfx::Point(-1, 0), testee, gfx::Rectangle(1, 1, 5, 4));

        static const uint8_t EXPECTED[] = {
            0,0,0,0,0,0,0,0,0,0,
            6,6,6,1,2,0,0,0,0,0,
            5,6,5,1,0,0,0,0,0,0,
            6,6,6,1,0,0,0,0,0,0,
            6,5,6,1,2,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,
        };
        a.checkEqualContent<uint8_t>("21. blit", otherPixmap->pixels().subrange(0, 6*10), EXPECTED);
    }
}
