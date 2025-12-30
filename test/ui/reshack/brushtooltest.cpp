/**
  *  \file test/ui/reshack/brushtooltest.cpp
  *  \brief Test for ui::reshack::BrushTool
  */

#include "ui/reshack/brushtool.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/base/ref.hpp"
#include "gfx/palettizedpixmap.hpp"

AFL_TEST("ui.reshack.BrushTool", a)
{
    afl::string::NullTranslator tx;
    ui::reshack::BrushTool testee(tx);

    // Basics
    a.checkDifferent("getName must not be null", testee.getName(), "");
    a.check("needsPreview must be false", !testee.needsPreview());
    a.check("isUsable must be true", testee.isUsable());

    // Test a cycle
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(20, 15);
    pix->pixels().fill('.');
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();

    gfx::BaseContext ctx(*can);
    ctx.setRawColor('x');

    testee.click(ctx, gfx::Point(3, 4), 'o');
    testee.drag(ctx, gfx::Point(10, 6));
    testee.release(ctx, gfx::Point(10, 8));

    // Verify image
    a.checkEqual("row  0", afl::string::fromBytes(pix->row( 0)), ".xxxxx..............");
    a.checkEqual("row  1", afl::string::fromBytes(pix->row( 1)), "xxxxxxx.............");
    a.checkEqual("row  2", afl::string::fromBytes(pix->row( 2)), "xxxxxxxxxxxxx.......");
    a.checkEqual("row  3", afl::string::fromBytes(pix->row( 3)), "xxxxxxxxxxxxxx......");
    a.checkEqual("row  4", afl::string::fromBytes(pix->row( 4)), "xxxxxxxxxxxxxxx.....");
    a.checkEqual("row  5", afl::string::fromBytes(pix->row( 5)), "xxxxxxxxxxxxxxx.....");
    a.checkEqual("row  6", afl::string::fromBytes(pix->row( 6)), "xxxxxxxxxxxxxxx.....");
    a.checkEqual("row  7", afl::string::fromBytes(pix->row( 7)), "xxxxxxxxxxxxxxx.....");
    a.checkEqual("row  8", afl::string::fromBytes(pix->row( 8)), ".xxxxxxxxxxxxxx.....");
    a.checkEqual("row  9", afl::string::fromBytes(pix->row( 9)), "......xxxxxxxxx.....");
    a.checkEqual("row 10", afl::string::fromBytes(pix->row(10)), "......xxxxxxxxx.....");
    a.checkEqual("row 11", afl::string::fromBytes(pix->row(11)), ".......xxxxxxx......");
    a.checkEqual("row 12", afl::string::fromBytes(pix->row(12)), "........xxxxx.......");
    a.checkEqual("row 13", afl::string::fromBytes(pix->row(13)), "....................");
    a.checkEqual("row 14", afl::string::fromBytes(pix->row(14)), "....................");
}

