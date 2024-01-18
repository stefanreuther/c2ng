/**
  *  \file test/ui/icons/colortiletest.cpp
  *  \brief Test for ui::icons::ColorTile
  */

#include "ui/icons/colortile.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullcolorscheme.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

namespace {
    /* Colors.
       NullEngine creates RGBAPixmaps, so Color_t is actually a ColorQuad_t.
       Color 77 is used for testing. */
    const gfx::Color_t EMPTY = COLORQUAD_FROM_RGB( 11, 22, 33);
    const gfx::Color_t BLACK = COLORQUAD_FROM_RGB(  0,  0,  0);
    const gfx::Color_t WHITE = COLORQUAD_FROM_RGB(255,255,255);
    const gfx::Color_t COL77 = COLORQUAD_FROM_RGB(214,214,230);

    /* Test Harness.
       Creates a Root (and associated engine/provider).
       Creates a Canvas and fills it with EMPTY color.
       (Root will also create a Canvas but not publish it.
       Use NullEngine's ability to create as many canvases as we want.) */
    struct TestHarness {
        gfx::NullEngine engine;
        gfx::NullResourceProvider provider;
        ui::Root root;

        afl::base::Ref<gfx::Canvas> canvas;
        gfx::NullColorScheme<ui::SkinColor::Color> colorScheme;
        gfx::Context<ui::SkinColor::Color> ctx;

        TestHarness()
            : engine(),
              provider(),
              root(engine, provider, gfx::WindowParameters()),
              canvas(engine.createWindow(gfx::WindowParameters())),
              colorScheme(),
              ctx(*canvas, colorScheme)
            {
                canvas->drawBar(gfx::Rectangle(0, 0, 1000, 1000), EMPTY, 0, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);
            }
    };
}

/** Test normal/default behaviour.
    A: create test harness. Create ColorTile.
    E: getSize() must report configured size; draw() must produce framed tile. */
AFL_TEST("ui.icons.ColorTile:normal", a)
{
    // Environment
    TestHarness h;

    // Testee; verify size
    ui::icons::ColorTile testee(h.root, gfx::Point(5, 7), 77);
    a.checkEqual("01. size", testee.getSize().getX(), 5);
    a.checkEqual("02. size", testee.getSize().getY(), 7);

    // Verify drawing
    testee.draw(h.ctx, gfx::Rectangle(10, 10, 5, 7), ui::ButtonFlags_t());

    gfx::Color_t pixels[20];
    h.canvas->getPixels(gfx::Point(8, 10), pixels);
    a.checkEqual("11", pixels[0], EMPTY);
    a.checkEqual("12", pixels[1], EMPTY);
    a.checkEqual("13", pixels[2], WHITE);
    a.checkEqual("14", pixels[3], WHITE);
    a.checkEqual("15", pixels[4], WHITE);
    a.checkEqual("16", pixels[5], WHITE);
    a.checkEqual("17", pixels[6], BLACK);
    a.checkEqual("18", pixels[7], EMPTY);

    h.canvas->getPixels(gfx::Point(8, 11), pixels);
    a.checkEqual("21", pixels[0], EMPTY);
    a.checkEqual("22", pixels[1], EMPTY);
    a.checkEqual("23", pixels[2], WHITE);
    a.checkEqual("24", pixels[3], COL77);
    a.checkEqual("25", pixels[4], COL77);
    a.checkEqual("26", pixels[5], COL77);
    a.checkEqual("27", pixels[6], BLACK);
    a.checkEqual("28", pixels[7], EMPTY);
}

/** Test behaviour with no frame.
    A: create test harness. Create ColorTile with frame width 0.
    E: getSize() must report configured size; draw() must produce unframed tile. */
AFL_TEST("ui.icons.ColorTile:no-frame", a)
{
    // Environment
    TestHarness h;

    // Testee; verify size
    ui::icons::ColorTile testee(h.root, gfx::Point(5, 7), 66);
    testee.setColor(77);
    testee.setFrameWidth(0);
    a.checkEqual("01. size", testee.getSize().getX(), 5);
    a.checkEqual("02. size", testee.getSize().getY(), 7);

    // Verify drawing
    testee.draw(h.ctx, gfx::Rectangle(10, 10, 5, 7), ui::ButtonFlags_t());

    gfx::Color_t pixels[20];
    h.canvas->getPixels(gfx::Point(8, 10), pixels);
    a.checkEqual("11", pixels[0], EMPTY);
    a.checkEqual("12", pixels[1], EMPTY);
    a.checkEqual("13", pixels[2], COL77);
    a.checkEqual("14", pixels[3], COL77);
    a.checkEqual("15", pixels[4], COL77);
    a.checkEqual("16", pixels[5], COL77);
    a.checkEqual("17", pixels[6], COL77);
    a.checkEqual("18", pixels[7], EMPTY);

    h.canvas->getPixels(gfx::Point(8, 11), pixels);
    a.checkEqual("21", pixels[0], EMPTY);
    a.checkEqual("22", pixels[1], EMPTY);
    a.checkEqual("23", pixels[2], COL77);
    a.checkEqual("24", pixels[3], COL77);
    a.checkEqual("25", pixels[4], COL77);
    a.checkEqual("26", pixels[5], COL77);
    a.checkEqual("27", pixels[6], COL77);
    a.checkEqual("28", pixels[7], EMPTY);
}

/** Test draw behaviour with different size.
    A: create test harness. Create ColorTile.
    E: draw() must produce framed tile at size passed to draw. */
AFL_TEST("ui.icons.ColorTile:resize", a)
{
    // Environment
    TestHarness h;

    // Testee; verify size
    ui::icons::ColorTile testee(h.root, gfx::Point(5, 7), 77);
    a.checkEqual("01. size", testee.getSize().getX(), 5);
    a.checkEqual("02. size", testee.getSize().getY(), 7);

    // Verify drawing
    testee.draw(h.ctx, gfx::Rectangle(10, 10, 3, 4), ui::ButtonFlags_t());

    gfx::Color_t pixels[20];
    h.canvas->getPixels(gfx::Point(8, 10), pixels);
    a.checkEqual("11", pixels[0], EMPTY);
    a.checkEqual("12", pixels[1], EMPTY);
    a.checkEqual("13", pixels[2], WHITE);
    a.checkEqual("14", pixels[3], WHITE);
    a.checkEqual("15", pixels[4], BLACK);
    a.checkEqual("16", pixels[5], EMPTY);

    h.canvas->getPixels(gfx::Point(8, 11), pixels);
    a.checkEqual("21", pixels[0], EMPTY);
    a.checkEqual("22", pixels[1], EMPTY);
    a.checkEqual("23", pixels[2], WHITE);
    a.checkEqual("24", pixels[3], COL77);
    a.checkEqual("25", pixels[4], BLACK);
    a.checkEqual("26", pixels[5], EMPTY);
}

/** Test default behaviour with changed frame type.
    A: create test harness. Create ColorTile; change frame type.
    E: getSize() must report configured size; draw() must produce framed tile. */
AFL_TEST("ui.icons.ColorTile:frame-type", a)
{
    // Environment
    TestHarness h;

    // Testee; verify size
    ui::icons::ColorTile testee(h.root, gfx::Point(5, 7), 77);
    testee.setFrameType(ui::LoweredFrame);
    a.checkEqual("01. size", testee.getSize().getX(), 5);
    a.checkEqual("02. size", testee.getSize().getY(), 7);

    // Verify drawing
    testee.draw(h.ctx, gfx::Rectangle(10, 10, 5, 7), ui::ButtonFlags_t());

    gfx::Color_t pixels[20];
    h.canvas->getPixels(gfx::Point(8, 10), pixels);
    a.checkEqual("11", pixels[0], EMPTY);
    a.checkEqual("12", pixels[1], EMPTY);
    a.checkEqual("13", pixels[2], BLACK);
    a.checkEqual("14", pixels[3], BLACK);
    a.checkEqual("15", pixels[4], BLACK);
    a.checkEqual("16", pixels[5], BLACK);
    a.checkEqual("17", pixels[6], WHITE);
    a.checkEqual("18", pixels[7], EMPTY);

    h.canvas->getPixels(gfx::Point(8, 11), pixels);
    a.checkEqual("21", pixels[0], EMPTY);
    a.checkEqual("22", pixels[1], EMPTY);
    a.checkEqual("23", pixels[2], BLACK);
    a.checkEqual("24", pixels[3], COL77);
    a.checkEqual("25", pixels[4], COL77);
    a.checkEqual("26", pixels[5], COL77);
    a.checkEqual("27", pixels[6], WHITE);
    a.checkEqual("28", pixels[7], EMPTY);
}
