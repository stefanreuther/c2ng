/**
  *  \file test/ui/icons/spacertest.cpp
  *  \brief Test for ui::icons::Spacer
  */

#include "ui/icons/spacer.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "gfx/palettizedpixmap.hpp"

/** Simple test. */
AFL_TEST("ui.icons.Spacer", a)
{
    gfx::Point pt(3, 5);
    ui::icons::Spacer testee(pt);

    // Verify size
    a.checkEqual("01. getSize", testee.getSize(), pt);

    // Verify (non)drawing
    // - make empty pixmap, canvas
    afl::base::Ref<gfx::PalettizedPixmap> pix(gfx::PalettizedPixmap::create(7, 8));
    pix->pixels().fill(0xAA);
    afl::base::Ref<gfx::Canvas> can(pix->makeCanvas());

    // - context with color scheme
    gfx::NullColorScheme<ui::SkinColor::Color> colorScheme;
    gfx::Context<ui::SkinColor::Color> ctx(*can, colorScheme);
    ctx.setColor(ui::SkinColor::Static);

    // - draw
    testee.draw(ctx, gfx::Rectangle(1, 1, 3, 5), ui::ButtonFlags_t());

    // - verify: all pixels at original values
    a.checkEqual("11. background pixels", pix->pixels().findNot(0xAA), 7U * 8U);
}
