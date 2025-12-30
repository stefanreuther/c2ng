/**
  *  \file test/ui/reshack/linetooltest.cpp
  *  \brief Test for ui::reshack::LineTool
  */

#include "ui/reshack/linetool.hpp"

#include "afl/test/testrunner.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/base/ref.hpp"
#include "gfx/palettizedpixmap.hpp"

AFL_TEST("ui.reshack.LineTool", a)
{
    afl::string::NullTranslator tx;
    ui::reshack::LineTool testee(tx);

    // Basics
    a.checkDifferent("getName must not be null", testee.getName(), "");
    a.check("needsPreview must be true", testee.needsPreview());
    a.check("isUsable must be true", testee.isUsable());

    // Test a cycle
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(7, 5);
    pix->pixels().fill('.');
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();

    gfx::BaseContext ctx(*can);
    ctx.setRawColor('x');

    testee.click(ctx, gfx::Point(2, 1), 'o');
    pix->pixels().fill('.');                  // needsPreview() protocol
    testee.drag(ctx, gfx::Point(5, 1));
    pix->pixels().fill('.');                  // needsPreview() protocol
    testee.release(ctx, gfx::Point(5, 4));

    // Verify image
    a.checkEqual("row  0", afl::string::fromBytes(pix->row( 0)), ".......");
    a.checkEqual("row  1", afl::string::fromBytes(pix->row( 1)), "..x....");
    a.checkEqual("row  2", afl::string::fromBytes(pix->row( 2)), "...x...");
    a.checkEqual("row  3", afl::string::fromBytes(pix->row( 3)), "....x..");
    a.checkEqual("row  4", afl::string::fromBytes(pix->row( 4)), ".....x.");
}
