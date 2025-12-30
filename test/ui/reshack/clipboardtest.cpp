/**
  *  \file test/ui/reshack/clipboardtest.cpp
  *  \brief Test for ui::reshack::Clipboard
  */

#include "ui/reshack/clipboard.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/basecontext.hpp"
#include "gfx/palettizedpixmap.hpp"

using afl::base::Ref;
using afl::string::NullTranslator;
using gfx::BaseContext;
using gfx::Canvas;
using gfx::PalettizedPixmap;
using ui::reshack::Clipboard;

/* Test basic clipboard operations */
AFL_TEST("ui.reshack.Clipboard:ops", a)
{
    Clipboard testee;

    a.check("01. has no content", !testee.hasContent());

    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(10, 20);
    testee.set(pix.asPtr(), 42);

    a.check("11. has content", testee.hasContent());
    a.checkEqual("12. pixmap", &*pix, testee.getPixmap().get());
    a.checkEqual("13. colorkey", testee.getColorKey(), 42U);
}

/* Test cut&paste operation */
AFL_TEST("ui.reshack.Clipboard:tools", a)
{
    Clipboard testee;
    NullTranslator tx;

    afl::base::ConstBytes_t content = afl::string::toBytes(
        "Lorem ipsum dolor si"
        "t amet, consectetuer"
        "adipiscing elit. Dui"
        "s sem velit, ultrice"
        "s et, fermentum auct"
        "or, rhoncus ut, ligu"
        "la. Phasellus at pur"
        "us sed purus cursus "
        "iaculis. Suspendisse"
        "fermentum. Pellentes");
    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(20, 10);
    pix->pixels().copyFrom(content);

    // Create "copy" tool
    Clipboard::CopyTool copyTool(testee, tx);
    a.checkDifferent("01. copy tool has name", copyTool.getName(), "");
    a.check("02. copy tool needsPreview", copyTool.needsPreview());
    a.check("03. copy tool is usable", copyTool.isUsable());

    // Create "paste" tool
    Clipboard::PasteTool pasteTool(testee, false, tx);
    a.checkDifferent("11. paste tool has name", pasteTool.getName(), "");
    a.check("12. paste tool needsPreview", !pasteTool.needsPreview());
    a.check("13. paste tool is not usable", !pasteTool.isUsable());

    // Perform "copy" sequence
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('X');
    copyTool.click(ctx, gfx::Point(10, 5), ' ');
    pix->pixels().copyFrom(content);                // needsPreview
    copyTool.drag(ctx, gfx::Point(5, 4));
    pix->pixels().copyFrom(content);                // needsPreview
    copyTool.release(ctx, gfx::Point(5, 2));

    // Verify
    a.check("21. clipboard has content", testee.hasContent());
    a.check("22. paste tool now usable", pasteTool.isUsable());
    a.checkEqual("22. clipboard content", afl::string::fromBytes(testee.getPixmap()->pixels()),
                 "scing "
                 " velit"
                 " ferme"
                 "honcus");
    a.checkEqual("23. clipboard colorkey", testee.getColorKey(), 32U);

    // Perform "paste" sequence
    pasteTool.click(ctx, gfx::Point(10, 5), ' ');
    pasteTool.drag(ctx, gfx::Point(11, 5));
    pasteTool.release(ctx, gfx::Point(12, 5));

    // Verify
    a.checkEqual("31. image content", afl::string::fromBytes(pix->pixels()),
                 "Lorem ipsum dolor si"
                 "t amet, consectetuer"
                 "adipiscing elit. Dui"
                 "s sem velit, ultrice"
                 "s et, fermentum auct"
                 "or, rhoncussscingigu"
                 "la. Phasellvvvelitur"
                 "us sed purufffermes "
                 "iaculis. Shhhoncusse"
                 "fermentum. Pellentes");
}

/* Test cut&paste operation, out-of-range coordinates */
AFL_TEST("ui.reshack.Clipboard:tools:out-of-range", a)
{
    Clipboard testee;
    NullTranslator tx;

    Ref<PalettizedPixmap> pix = PalettizedPixmap::create(20, 10);

    // Create "copy" and "paste" tools
    Clipboard::CopyTool copyTool(testee, tx);
    Clipboard::PasteTool pasteTool(testee, false, tx);

    // Perform "copy" sequence
    Ref<Canvas> can = pix->makeCanvas();
    BaseContext ctx(*can);
    ctx.setRawColor('X');
    copyTool.click(ctx, gfx::Point(20, 5), ' ');
    pix->pixels().fill(0);
    copyTool.release(ctx, gfx::Point(30, 2));

    // Verify
    a.check("01. clipboard has no content", !testee.hasContent());
    a.check("02. paste tool not usable", !pasteTool.isUsable());
}
