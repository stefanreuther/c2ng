/**
  *  \file test/ui/res/generatedengineprovidertest.cpp
  *  \brief Test for ui::res::GeneratedEngineProvider
  */

#include "ui/res/generatedengineprovider.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/bitmapfont.hpp"
#include "gfx/bitmapglyph.hpp"
#include "ui/colorscheme.hpp"
#include <cstdio>

AFL_TEST("ui.res.GeneratedEngineProvider", a)
{
    // Make an empty font of height 1
    gfx::BitmapFont emptyFont;
    emptyFont.addNewGlyph(' ', new gfx::BitmapGlyph(1, 1));
    a.checkEqual("01. getHeight", emptyFont.getHeight(), 1);

    // Empty translator
    afl::string::NullTranslator tx;

    // Fuel usage vector for Tech 4 engine
    std::vector<int> fuelUsage;
    fuelUsage.push_back(100);
    fuelUsage.push_back(103);
    fuelUsage.push_back(104);
    fuelUsage.push_back(106);
    fuelUsage.push_back(300);
    fuelUsage.push_back(322);
    fuelUsage.push_back(495);
    fuelUsage.push_back(487);
    fuelUsage.push_back(900);

    // Render it
    const int WI = 30, HE = 25;
    afl::base::Ref<gfx::Canvas> result = ui::res::renderEngineDiagram(fuelUsage, gfx::Point(WI, HE), emptyFont, tx);
    a.checkEqual("11. getSize", result->getSize(), gfx::Point(WI, HE));

    // Expected image (color indexes).
    // As of 20200523, renderEngineDiagram will return a palettized image, but this is not contractual.
    // We therefore decode the color indexes before comparing.
    const uint8_t EXPECT[HE][WI] = {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,92, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,92, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,93,92, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0,94,94, 0,94,94, 0,94,94, 0,93,92, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0,94, 0,94,94, 0,94,94, 0,94,94, 0,93,92, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0,95,94, 0,94,94, 0,94,94, 0,94,94, 0,93,92, 0,91,90, 0,89,89, 0,88,88, 0,86,83, 0,81,80 },
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    };

    for (int y = 0; y < HE; ++y) {
        for (int x = 0; x < WI; ++x) {
            char tmp[100];
            std::sprintf(tmp, "(%d,%d)", x, y);

            gfx::Color_t c[1];
            result->getPixels(gfx::Point(x, y), c);

            gfx::ColorQuad_t q[1];
            result->decodeColors(c, q);

            a.checkEqual(tmp, q[0], ui::STANDARD_COLORS[EXPECT[y][x]]);
        }
    }
}
