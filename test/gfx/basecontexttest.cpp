/**
  *  \file test/gfx/basecontexttest.cpp
  *  \brief Test for gfx::BaseContext
  */

#include "gfx/basecontext.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/defaultfont.hpp"
#include "gfx/font.hpp"
#include "gfx/nullcanvas.hpp"

/** Test getter/setter. */
AFL_TEST("gfx.BaseContext", a)
{
    // Environment
    gfx::NullCanvas ca, cb;
    afl::base::Ref<gfx::Font> f(gfx::createDefaultFont());

    // Testee
    gfx::BaseContext testee(ca);

    // Initial state
    a.checkEqual("01. getRawColor",             testee.getRawColor(), 0U);
    a.checkEqual("02. isTransparentBackground", testee.isTransparentBackground(), true);
    a.checkEqual("03. getLineThickness",        testee.getLineThickness(), 1);
    a.checkEqual("04. getLinePattern",          testee.getLinePattern(), 0xFF);
    a.check     ("05. fillPattern",             testee.fillPattern().isBlack());
    a.checkEqual("06. getAlpha",                testee.getAlpha(), gfx::OPAQUE_ALPHA);
    a.checkEqual("07. getCursor",               testee.getCursor(), gfx::Point(0, 0));
    a.checkEqual("08. getTextAlign",            testee.getTextAlign(), gfx::Point(0, 0));
    a.checkNull ("09. getFont",                 testee.getFont());
    a.checkEqual("10. canvas",                 &testee.canvas(), &ca);

    // Reconfigure
    testee.setRawColor(99);
    testee.setSolidBackground();
    testee.setLineThickness(3);
    testee.setLinePattern(0xCC);
    testee.setFillPattern(gfx::FillPattern::GRAY50);
    testee.setAlpha(120);
    testee.setCursor(gfx::Point(100, 200));
    testee.setTextAlign(gfx::RightAlign, gfx::MiddleAlign);
    testee.useFont(*f);
    testee.useCanvas(cb);

    a.checkEqual("11. getRawColor",             testee.getRawColor(), 99U);
    a.checkEqual("12. isTransparentBackground", testee.isTransparentBackground(), false);
    a.checkEqual("13. getLineThickness",        testee.getLineThickness(), 3);
    a.checkEqual("14. getLinePattern",          testee.getLinePattern(), 0xCC);
    a.checkEqual("15. fillPattern",             testee.fillPattern()[0], gfx::FillPattern::GRAY50[0]);
    a.checkEqual("16. fillPattern",             testee.fillPattern()[1], gfx::FillPattern::GRAY50[1]);
    a.checkEqual("17. fillPattern",             testee.fillPattern()[6], gfx::FillPattern::GRAY50[6]);
    a.checkEqual("18. fillPattern",             testee.fillPattern()[7], gfx::FillPattern::GRAY50[7]);
    a.checkEqual("19. getAlpha",                testee.getAlpha(), 120);
    a.checkEqual("20. getCursor",               testee.getCursor(), gfx::Point(100, 200));
    a.checkEqual("21. getTextAlign",            testee.getTextAlign(), gfx::Point(2, 1));
    a.checkEqual("22. getFont",                 testee.getFont(), &*f);
    a.checkEqual("23. canvas",                 &testee.canvas(), &cb);

    // Background
    testee.setSolidBackground();
    a.check("31. isTransparentBackground", !testee.isTransparentBackground());
    testee.setTransparentBackground();
    a.check("32. isTransparentBackground", testee.isTransparentBackground());

    // Const
    const gfx::BaseContext& bc = testee;
    a.checkEqual("41. getRawColor",      bc.getRawColor(), testee.getRawColor());
    a.checkEqual("42. getLineThickness", bc.getLineThickness(), testee.getLineThickness());
    a.checkEqual("43. fillPattern",     &bc.fillPattern(), &testee.fillPattern());
    a.checkEqual("44. canvas",          &bc.canvas(), &testee.canvas());
    a.checkEqual("45. getFont",          bc.getFont(), testee.getFont());
}
