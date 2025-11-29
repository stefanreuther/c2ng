/**
  *  \file test/gfx/fonttest.cpp
  *  \brief Test for gfx::Font
  */

#include "gfx/font.hpp"

#include "afl/functional/stringtable.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/canvas.hpp"
#include "gfx/context.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "gfx/palettizedpixmap.hpp"

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
AFL_TEST("gfx.Font:basics", a)
{
    TestFont t;

    // Litmus test
    a.checkEqual("01. getTextWidth",  t.getTextWidth("four"), 4);
    a.checkEqual("02. getTextHeight", t.getTextHeight("four"), 1);

    // Derived functions
    a.checkEqual("11. getEmWidth",    t.getEmWidth(), 1);
    a.checkEqual("12. getLineHeight", t.getLineHeight(), 1);
    a.checkEqual("13. getCellSize",   t.getCellSize(), gfx::Point(1, 1));

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
    a.checkEqualContent<uint8_t>("12. pixels", pix->pixels(), EXPECTED);
}

/** Test outTextF with a given width. */
AFL_TEST("gfx.Font:outTextF", a)
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
    a.checkEqual("01. getCursor", ctx.getCursor(), gfx::Point(5, 1));

    ctx.setColor(2);
    ctx.setTextAlign(gfx::CenterAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Point(5, 2), 8, "mnopq");
    a.checkEqual("11. getCursor", ctx.getCursor(), gfx::Point(5, 2));

    ctx.setColor(3);
    ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
    outTextF(ctx, gfx::Point(10, 5), 4, String_t("xyz"));
    a.checkEqual("21. getCursor", ctx.getCursor(), gfx::Point(7, 5));

    ctx.setColor(4);
    ctx.setTextAlign(gfx::LeftAlign, gfx::TopAlign);
    outTextF(ctx, gfx::Point(0, 4), 2, "xyzzy");
    a.checkEqual("31. getCursor", ctx.getCursor(), gfx::Point(2, 4));

    static const uint8_t EXPECTED[] = {
        9,9,9,9,9,9,9,9,9,9,
        9,9,1,1,1,0,0,9,9,9,
        9,0,0,2,2,2,2,2,0,9,
        9,9,9,9,9,9,9,9,9,9,
        4,4,9,9,9,9,0,3,3,3,
    };
    a.checkEqualContent<uint8_t>("41. pixels", pix->pixels(), EXPECTED);
}

/** Test outTextF with a given area. */
AFL_TEST("gfx.Font:outTextF:area", a)
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
    a.checkEqual("01. getCursor", ctx.getCursor(), gfx::Point(5, 1));

    ctx.setColor(2);
    ctx.setTextAlign(gfx::CenterAlign, gfx::MiddleAlign);
    outTextF(ctx, gfx::Rectangle(1, 3, 8, 3), "mnopq");
    a.checkEqual("11. getCursor", ctx.getCursor(), gfx::Point(5, 4));

    ctx.setColor(3);
    ctx.setTextAlign(gfx::RightAlign, gfx::BottomAlign);
    outTextF(ctx, gfx::Rectangle(5, 7, 5, 2), String_t("xyz"));
    a.checkEqual("21. getCursor", ctx.getCursor(), gfx::Point(7, 9));

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
    a.checkEqualContent<uint8_t>("31. pixels", pix->pixels(), EXPECTED);
}

/** Test getMaxTextWidth. */
AFL_TEST("gfx.Font:getMaxTextWidth", a)
{
    String_t lines[] = { "a", "bcd", "e" };
    TestFont t;

    a.checkEqual("getMaxTextWidth", t.getMaxTextWidth(afl::functional::createStringTable(lines)), 3);
}

/** Test getMaxTextWidth, empty list. */
AFL_TEST("gfx.Font:getMaxTextWidth:empty", a)
{
    class Empty : public afl::functional::Mapping<int,String_t> {
     public:
        bool getFirstKey(int&) const
            { return false; }
        bool getNextKey(int&) const
            { return false; }
        String_t get(int) const
            { return ""; }
    };

    TestFont t;
    Empty e;

    a.checkEqual("getMaxTextWidth", t.getMaxTextWidth(e), 0);
}
