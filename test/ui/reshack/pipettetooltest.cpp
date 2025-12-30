/**
  *  \file test/ui/reshack/pipettetooltest.cpp
  *  \brief Test for ui::reshack::PipetteTool
  */

#include "ui/reshack/pipettetool.hpp"

#include "afl/base/ref.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "ui/reshack/painter.hpp"

AFL_TEST("ui.reshack.PipetteTool", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::base::Ref<gfx::PalettizedPixmap> pix = gfx::PalettizedPixmap::create(7, 5);
    pix->pixels().fill('.');
    pix->row(2).fill('2');
    afl::base::Ref<gfx::Canvas> can = pix->makeCanvas();

    // Painter
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());
    ui::reshack::Painter p(pix.asPtr(), ui::reshack::Palette::StandardPaletteColor, root);

    // Tool
    ui::reshack::PipetteTool testee(tx, p);

    // Basics
    a.checkDifferent("getName must not be null", testee.getName(), "");
    a.check("needsPreview must be false", !testee.needsPreview());
    a.check("isUsable must be true", testee.isUsable());

    // Test a cycle. Color follows mouse.
    gfx::BaseContext ctx(*can);
    ctx.setRawColor('a');
    testee.click(ctx, gfx::Point(2, 1), 'o');
    a.checkEqual("painter color after click", p.getColor(false), '.');
    testee.drag(ctx, gfx::Point(5, 1));
    testee.release(ctx, gfx::Point(5, 2));
    a.checkEqual("painter color after click", p.getColor(false), '2');
}
