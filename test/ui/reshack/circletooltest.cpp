/**
  *  \file test/ui/reshack/circletooltest.cpp
  *  \brief Test for ui::reshack::CircleTool
  */

#include "ui/reshack/circletool.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/base/ref.hpp"
#include "gfx/palettizedpixmap.hpp"

AFL_TEST("ui.reshack.CircleTool", a)
{
    afl::string::NullTranslator tx;
    ui::reshack::CircleTool testee(tx);

    // Basics
    a.checkDifferent("getName must not be null", testee.getName(), "");
    a.check("needsPreview must be true", testee.needsPreview());
    a.check("isUsable must be true", testee.isUsable());

    // Test a cycle
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(20, 15);
    pix->pixels().fill('.');
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();

    gfx::BaseContext ctx(*can);
    ctx.setRawColor('x');

    testee.click(ctx, gfx::Point(3, 4), 'o');
    pix->pixels().fill('.');                  // needsPreview() protocol
    testee.drag(ctx, gfx::Point(10, 6));
    pix->pixels().fill('.');                  // needsPreview() protocol
    testee.release(ctx, gfx::Point(10, 8));

    // Verify image
    a.checkEqual("row  0", afl::string::fromBytes(pix->row( 0)), "..........x.........");
    a.checkEqual("row  1", afl::string::fromBytes(pix->row( 1)), "...........x........");
    a.checkEqual("row  2", afl::string::fromBytes(pix->row( 2)), "...........x........");
    a.checkEqual("row  3", afl::string::fromBytes(pix->row( 3)), "...........x........");
    a.checkEqual("row  4", afl::string::fromBytes(pix->row( 4)), "...........x........");
    a.checkEqual("row  5", afl::string::fromBytes(pix->row( 5)), "...........x........");
    a.checkEqual("row  6", afl::string::fromBytes(pix->row( 6)), "...........x........");
    a.checkEqual("row  7", afl::string::fromBytes(pix->row( 7)), "...........x........");
    a.checkEqual("row  8", afl::string::fromBytes(pix->row( 8)), "..........x.........");
    a.checkEqual("row  9", afl::string::fromBytes(pix->row( 9)), "..........x.........");
    a.checkEqual("row 10", afl::string::fromBytes(pix->row(10)), ".........x..........");
    a.checkEqual("row 11", afl::string::fromBytes(pix->row(11)), ".......xx...........");
    a.checkEqual("row 12", afl::string::fromBytes(pix->row(12)), "xxxxxxx.............");
    a.checkEqual("row 13", afl::string::fromBytes(pix->row(13)), "....................");
    a.checkEqual("row 14", afl::string::fromBytes(pix->row(14)), "....................");
}

