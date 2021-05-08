/**
  *  \file u/t_ui_icons_spacer.cpp
  *  \brief Test for ui::icons::Spacer
  */

#include "ui/icons/spacer.hpp"

#include "t_ui_icons.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/nullcolorscheme.hpp"

/** Simple test. */
void
TestUiIconsSpacer::testIt()
{
    gfx::Point pt(3, 5);
    ui::icons::Spacer testee(pt);

    // Verify size
    TS_ASSERT_EQUALS(testee.getSize(), pt);

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
    TS_ASSERT_EQUALS(pix->pixels().findNot(0xAA), 7U * 8U);
}

