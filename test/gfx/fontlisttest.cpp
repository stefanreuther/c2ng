/**
  *  \file test/gfx/fontlisttest.cpp
  *  \brief Test for gfx::FontList
  */

#include "gfx/fontlist.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/bitmapfont.hpp"
#include "gfx/font.hpp"

namespace {
    gfx::FontRequest makeRequest(gfx::FontRequest::Value_t size,
                                 gfx::FontRequest::Value_t weight,
                                 gfx::FontRequest::Value_t slant,
                                 gfx::FontRequest::Value_t style)
    {
        return gfx::FontRequest().setSize(size).setWeight(weight).setSlant(slant).setStyle(style);
    }
}

/** Test empty list.
    All requests return 0. */
AFL_TEST("gfx.FontList:empty", a)
{
    gfx::FontList testee;
    a.checkNull("01", testee.findFont(gfx::FontRequest()).get());
    a.checkNull("02", testee.findFont(makeRequest(1,0,0,0)).get());
    a.checkNull("03", testee.findFont(makeRequest(0,1,0,0)).get());
    a.checkNull("04", testee.findFont(makeRequest(0,0,1,0)).get());
    a.checkNull("05", testee.findFont(makeRequest(0,0,0,1)).get());
    a.checkNull("06", testee.findFont(makeRequest(1,1,1,1)).get());
}

/** Test one-element list. */
AFL_TEST("gfx.FontList:unit", a)
{
    afl::base::Ref<gfx::Font> f = *new gfx::BitmapFont();
    gfx::FontList testee;
    testee.addFont(gfx::FontRequest().setSize(1).setWeight(1).setSlant(1).setStyle(1), f.asPtr());

    a.checkEqual("01", testee.findFont(gfx::FontRequest()).get(), &*f);
    a.checkEqual("02", testee.findFont(makeRequest(1,0,0,0)).get(), &*f);
    a.checkEqual("03", testee.findFont(makeRequest(0,1,0,0)).get(), &*f);
    a.checkEqual("04", testee.findFont(makeRequest(0,0,1,0)).get(), &*f);
    a.checkEqual("05", testee.findFont(makeRequest(0,0,0,1)).get(), &*f);
    a.checkEqual("06", testee.findFont(makeRequest(1,1,1,1)).get(), &*f);
}

/** Test populated list. */
AFL_TEST("gfx.FontList:full", a)
{
    afl::base::Ref<gfx::Font> aa = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> bb = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> cc = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> dd = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> ee = *new gfx::BitmapFont();

    gfx::FontList testee;

    testee.addFont(makeRequest(1,0,0,0), aa.asPtr());
    testee.addFont(makeRequest(2,1,0,0), bb.asPtr());
    testee.addFont(makeRequest(3,2,1,0), cc.asPtr());
    testee.addFont(makeRequest(4,3,2,0), dd.asPtr());
    testee.addFont(makeRequest(0,0,0,1), ee.asPtr());

    // Exact matches
    a.checkEqual("01", testee.findFont(makeRequest(1,0,0,0)).get(), &*aa);
    a.checkEqual("02", testee.findFont(makeRequest(2,1,0,0)).get(), &*bb);
    a.checkEqual("03", testee.findFont(makeRequest(3,2,1,0)).get(), &*cc);
    a.checkEqual("04", testee.findFont(makeRequest(4,3,2,0)).get(), &*dd);
    a.checkEqual("05", testee.findFont(makeRequest(0,0,0,1)).get(), &*ee);

    // Inexact matches
    // - slant mismatch
    a.checkEqual("11", testee.findFont(makeRequest(1,0,1,0)).get(), &*aa);

    // - weight mismatch
    a.checkEqual("21", testee.findFont(makeRequest(1,1,0,0)).get(), &*aa);
    a.checkEqual("22", testee.findFont(makeRequest(2,2,0,0)).get(), &*bb);
    a.checkEqual("23", testee.findFont(makeRequest(3,2,3,0)).get(), &*cc);
    a.checkEqual("24", testee.findFont(makeRequest(4,7,7,0)).get(), &*dd);

    // - size mismatch
    a.checkEqual("31", testee.findFont(makeRequest(5,0,0,1)).get(), &*ee);
    a.checkEqual("32", testee.findFont(makeRequest(4,3,2,1)).get(), &*ee);
    a.checkEqual("33", testee.findFont(makeRequest(3,2,1,1)).get(), &*ee);

    // - no match
    a.checkEqual("41", testee.findFont(makeRequest(7,7,7,7)).get(), &*aa);
}
