/**
  *  \file u/t_gfx_font.cpp
  *  \brief Test for gfx::Font
  */

#include "gfx/font.hpp"

#include "t_gfx.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/canvas.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "gfx/context.hpp"

#define TS_ASSERT_SAME(got, expected) \
    TS_ASSERT_EQUALS(got.size(), sizeof(expected)); \
    TS_ASSERT_SAME_DATA(got.unsafeData(), expected, sizeof(expected))

namespace {
    class TestFont : public gfx::Font {
     public:
        virtual void outText(gfx::BaseContext& ctx, gfx::Point pt, String_t text)
            { ctx.canvas().drawHLine(pt, int(text.size()), ctx.getRawColor(), 0xFF, ctx.getAlpha()); }
        virtual int getTextWidth(String_t text)
            { return int(text.size()); }
        virtual int getTextHeight(String_t /*text*/)
            { return 1; }
    };
}

/** Simple test. */
void
TestGfxFont::testIt()
{
    TestFont t;

    // Litmus test
    TS_ASSERT_EQUALS(t.getTextWidth("four"), 4);
    TS_ASSERT_EQUALS(t.getTextHeight("four"), 1);

    // Derived functions
    TS_ASSERT_EQUALS(t.getEmWidth(), 1);
    TS_ASSERT_EQUALS(t.getLineHeight(), 1);
    TS_ASSERT_EQUALS(t.getCellSize(), gfx::Point(1, 1));

    // Aligned drawing using outText
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(10, 5);
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();
    gfx::BaseContext ctx(*can);
    ctx.useFont(t);

    ctx.setRawColor(1);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outText(ctx, gfx::Point(2, 1), "abc");

    ctx.setRawColor(2);
    ctx.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
    outText(ctx, gfx::Point(5, 2), "mnopq");

    ctx.setRawColor(3);
    ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
    outText(ctx, gfx::Point(10, 5), String_t("xyz"));

    static const uint8_t EXPECTED[] = {
        0,0,0,0,0,0,0,0,0,0,
        0,0,1,1,1,0,0,0,0,0,
        0,0,0,2,2,2,2,2,0,0,
        0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,3,3,3,
    };
    TS_ASSERT_SAME(pix->pixels(), EXPECTED);
}

/** Test outTextF with a given width. */
void
TestGfxFont::testFitWidth()
{
    TestFont t;

    // Environment
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(10, 5);
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();
    gfx::NullColorScheme<int> cs;
    gfx::Context<int> ctx(*can, cs);
    ctx.useFont(t);
    pix->pixels().fill(9);

    ctx.setSolidBackground();
    ctx.setColor(1);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Point(2, 1), 5, "abc");
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(5, 1));

    ctx.setColor(2);
    ctx.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Point(5, 2), 8, "mnopq");
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(5, 2));

    ctx.setColor(3);
    ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
    outTextF(ctx, gfx::Point(10, 5), 4, String_t("xyz"));
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(7, 5));

    ctx.setColor(4);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Point(0, 4), 2, "xyzzy");
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(2, 4));

    static const uint8_t EXPECTED[] = {
        9,9,9,9,9,9,9,9,9,9,
        9,9,1,1,1,0,0,9,9,9,
        9,0,0,2,2,2,2,2,0,9,
        9,9,9,9,9,9,9,9,9,9,
        4,4,9,9,9,9,0,3,3,3,
    };
    TS_ASSERT_SAME(pix->pixels(), EXPECTED);
}

/** Test outTextF with a given area. */
void
TestGfxFont::testFitArea()
{
    TestFont t;

    // Environment
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(10, 10);
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();
    gfx::NullColorScheme<int> cs;
    gfx::Context<int> ctx(*can, cs);
    ctx.useFont(t);
    pix->pixels().fill(9);

    ctx.setSolidBackground();
    ctx.setColor(1);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Rectangle(2, 1, 4, 2), "abc");
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(5, 1));

    ctx.setColor(2);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    outTextF(ctx, gfx::Rectangle(1, 3, 8, 3), "mnopq");
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(5, 4));

    ctx.setColor(3);
    ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
    outTextF(ctx, gfx::Rectangle(5, 7, 5, 2), String_t("xyz"));
    TS_ASSERT_EQUALS(ctx.getCursor(), gfx::Point(7, 9));

    static const uint8_t EXPECTED[] = {
        9,9,9,9,9,9,9,9,9,9,
        9,9,1,1,1,0,9,9,9,9,
        9,9,0,0,0,0,9,9,9,9,
        9,0,0,0,0,0,0,0,0,9,
        9,0,0,2,2,2,2,2,0,9,
        9,0,0,0,0,0,0,0,0,9,
        9,9,9,9,9,9,9,9,9,9,
        9,9,9,9,9,0,0,0,0,0,
        9,9,9,9,9,0,0,3,3,3,
        9,9,9,9,9,9,9,9,9,9,
    };
    TS_ASSERT_SAME(pix->pixels(), EXPECTED);
}

