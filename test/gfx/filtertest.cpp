/**
  *  \file test/gfx/filtertest.cpp
  *  \brief Test for gfx::Filter
  */

#include "gfx/filter.hpp"

#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    /* Test implementation of Filter (also serves as interface test) */
    class TestFilter : public gfx::Filter, public afl::test::CallReceiver {
     public:
        TestFilter(gfx::Canvas& parent, afl::test::Assert a)
            : Filter(parent), CallReceiver(a)
            { }
        virtual void drawHLine(const gfx::Point& /*pt*/, int /*npix*/, gfx::Color_t /*color*/, gfx::LinePattern_t /*pat*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawHLine"); }
        virtual void drawVLine(const gfx::Point& /*pt*/, int /*npix*/, gfx::Color_t /*color*/, gfx::LinePattern_t /*pat*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawVLine"); }
        virtual void drawPixel(const gfx::Point& /*pt*/, gfx::Color_t /*color*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawPixel"); }
        virtual void drawPixels(const gfx::Point& /*pt*/, afl::base::Memory<const gfx::Color_t> /*colors*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawPixels"); }
        virtual void drawBar(gfx::Rectangle /*rect*/, gfx::Color_t /*color*/, gfx::Color_t /*bg*/, const gfx::FillPattern& /*pat*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawBar"); }
        virtual void blit(const gfx::Point& /*pt*/, gfx::Canvas& /*src*/, gfx::Rectangle /*rect*/)
            { checkCall("blit"); }
        virtual void blitPattern(gfx::Rectangle /*rect*/, const gfx::Point& /*pt*/, int /*bytesPerLine*/, const uint8_t* /*data*/, gfx::Color_t /*color*/, gfx::Color_t /*bg*/, gfx::Alpha_t /*alpha*/)
            { checkCall("blitPattern"); }
        virtual gfx::Rectangle computeClipRect(gfx::Rectangle /*r*/)
            {
                checkCall("computeClipRect");
                return consumeReturnValue<gfx::Rectangle>();
            }
        virtual bool isVisible(gfx::Rectangle /*r*/)
            {
                checkCall("isVisible");
                return consumeReturnValue<bool>();
            }
        virtual bool isClipped(gfx::Rectangle /*r*/)
            {
                checkCall("isClipped");
                return consumeReturnValue<bool>();
            }
    };

    class TestCanvas : public gfx::Canvas, public afl::test::CallReceiver {
     public:
        TestCanvas(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual void drawHLine(const gfx::Point& /*pt*/, int /*npix*/, gfx::Color_t /*color*/, gfx::LinePattern_t /*pat*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawHLine"); }
        virtual void drawVLine(const gfx::Point& /*pt*/, int /*npix*/, gfx::Color_t /*color*/, gfx::LinePattern_t /*pat*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawVLine"); }
        virtual void drawPixel(const gfx::Point& /*pt*/, gfx::Color_t /*color*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawPixel"); }
        virtual void drawPixels(const gfx::Point& /*pt*/, afl::base::Memory<const gfx::Color_t> /*colors*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawPixels"); }
        virtual void drawBar(gfx::Rectangle /*rect*/, gfx::Color_t /*color*/, gfx::Color_t /*bg*/, const gfx::FillPattern& /*pat*/, gfx::Alpha_t /*alpha*/)
            { checkCall("drawBar"); }
        virtual void blit(const gfx::Point& /*pt*/, gfx::Canvas& /*src*/, gfx::Rectangle /*rect*/)
            { checkCall("blit"); }
        virtual void blitPattern(gfx::Rectangle /*rect*/, const gfx::Point& /*pt*/, int /*bytesPerLine*/, const uint8_t* /*data*/, gfx::Color_t /*color*/, gfx::Color_t /*bg*/, gfx::Alpha_t /*alpha*/)
            { checkCall("blitPattern"); }
        virtual gfx::Rectangle computeClipRect(gfx::Rectangle /*r*/)
            {
                checkCall("computeClipRect");
                return consumeReturnValue<gfx::Rectangle>();
            }
        virtual void getPixels(gfx::Point /*pt*/, afl::base::Memory<gfx::Color_t> /*colors*/)
            { checkCall("getPixels"); }
        virtual gfx::Point getSize()
            {
                checkCall("getSize");
                return consumeReturnValue<gfx::Point>();
            }
        virtual int getBitsPerPixel()
            {
                checkCall("getBitsPerPixel");
                return consumeReturnValue<int>();
            }
        virtual bool isVisible(gfx::Rectangle /*r*/)
            {
                checkCall("isVisible");
                return consumeReturnValue<bool>();
            }
        virtual bool isClipped(gfx::Rectangle /*r*/)
            {
                checkCall("isClipped");
                return consumeReturnValue<bool>();
            }
        virtual void setPalette(gfx::Color_t /*start*/, afl::base::Memory<const gfx::ColorQuad_t> /*colorDefinitions*/, afl::base::Memory<gfx::Color_t> /*colorHandles*/)
            { checkCall("setPalette"); }
        virtual void decodeColors(afl::base::Memory<const gfx::Color_t> /*colorHandles*/, afl::base::Memory<gfx::ColorQuad_t> /*colorDefinitions*/)
            { checkCall("decodeColors"); }
        virtual void encodeColors(afl::base::Memory<const gfx::ColorQuad_t> /*colorDefinitions*/, afl::base::Memory<gfx::Color_t> /*colorHandles*/)
            { checkCall("encodeColors"); }
        virtual afl::base::Ref<Canvas> convertCanvas(afl::base::Ref<Canvas> orig)
            { checkCall("convertCanvas"); return orig; }
    };
}

/** Simple test. */
AFL_TEST("gfx.Filter", a)
{
    TestCanvas c(a("TestCanvas"));
    TestFilter t(c, a("TestFilter"));

    // getPixels
    {
        c.expectCall("getPixels");
        gfx::Color_t colors[3];
        t.getPixels(gfx::Point(), colors);
    }

    // getSize
    {
        c.expectCall("getSize");
        c.provideReturnValue(gfx::Point(10, 30));
        a.checkEqual("getSize", t.getSize(), gfx::Point(10, 30));
    }

    // getBitsPerPixel
    {
        c.expectCall("getBitsPerPixel");
        c.provideReturnValue<int>(24);
        a.checkEqual("getBitsPerPixel", t.getBitsPerPixel(), 24);
    }

    // setPalette
    gfx::ColorQuad_t quads[2] = {1,2};
    gfx::Color_t colors[2] = {3,4};
    {
        c.expectCall("setPalette");
        t.setPalette(99, quads, colors);
    }
    {
        c.expectCall("decodeColors");
        t.decodeColors(colors, quads);
    }
    {
        c.expectCall("encodeColors");
        t.encodeColors(quads, colors);
    }

    a.checkEqual("parent", &t.parent(), &c);
    c.checkFinish();
    t.checkFinish();
}
