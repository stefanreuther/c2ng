/**
  *  \file u/t_ui_colorscheme.cpp
  *  \brief Test for ui::ColorScheme
  */

#include "ui/colorscheme.hpp"

#include "t_ui.hpp"
#include "gfx/rgbapixmap.hpp"
#include "gfx/context.hpp"

/** Test background drawing. */
void
TestUiColorScheme::testBackground()
{
    const size_t N = 20;

    // Initialize
    ui::ColorScheme testee;
    afl::base::Ref<gfx::RGBAPixmap> pix = gfx::RGBAPixmap::create(N, N);
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();
    testee.init(*can);
    pix->pixels().fill(1234578);

    // Draw background
    testee.drawBackground(*can, gfx::Rectangle(0,0,N,N));

    // Verify
    afl::base::Memory<const uint32_t> pixels = pix->pixels();
    TS_ASSERT_EQUALS(pixels.size(), N*N);
    while (const uint32_t* p = pixels.eat()) {
        TS_ASSERT_EQUALS(*p, COLORQUAD_FROM_RGB(0,0,0));
    }
}

/** Test that we can get every color, even out-of-range ones. */
void
TestUiColorScheme::testColor()
{
    // Use a RGBAPixmap to initialize the palette to a 1:1 mapping
    ui::ColorScheme testee;
    afl::base::Ref<gfx::Canvas> can = gfx::RGBAPixmap::create(1, 1)->makeCanvas();
    testee.init(*can);

    // Test we can get everything
    for (uint32_t i = 0; i < ui::Color_Avail; ++i) {
        testee.getColor(uint8_t(i));
    }

    // Out-of-range colors must report black
    for (uint32_t i = ui::Color_Avail; i < 255; ++i) {
        TS_ASSERT_EQUALS(testee.getColor(uint8_t(i)), COLORQUAD_FROM_RGB(0,0,0));
    }
}
