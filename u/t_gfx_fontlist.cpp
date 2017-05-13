/**
  *  \file u/t_gfx_fontlist.cpp
  *  \brief Test for gfx::FontList
  */

#include "gfx/fontlist.hpp"

#include "t_gfx.hpp"
#include "gfx/font.hpp"
#include "gfx/bitmapfont.hpp"

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
void
TestGfxFontList::testEmpty()
{
    gfx::FontList testee;
    TS_ASSERT(testee.findFont(gfx::FontRequest()).get() == 0);
    TS_ASSERT(testee.findFont(makeRequest(1,0,0,0)).get() == 0);
    TS_ASSERT(testee.findFont(makeRequest(0,1,0,0)).get() == 0);
    TS_ASSERT(testee.findFont(makeRequest(0,0,1,0)).get() == 0);
    TS_ASSERT(testee.findFont(makeRequest(0,0,0,1)).get() == 0);
    TS_ASSERT(testee.findFont(makeRequest(1,1,1,1)).get() == 0);
}

/** Test one-element list. */
void
TestGfxFontList::testUnit()
{
    afl::base::Ref<gfx::Font> f = *new gfx::BitmapFont();
    gfx::FontList testee;
    testee.addFont(gfx::FontRequest().setSize(1).setWeight(1).setSlant(1).setStyle(1), f.asPtr());
    
    TS_ASSERT_EQUALS(testee.findFont(gfx::FontRequest()).get(), &*f);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(1,0,0,0)).get(), &*f);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(0,1,0,0)).get(), &*f);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(0,0,1,0)).get(), &*f);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(0,0,0,1)).get(), &*f);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(1,1,1,1)).get(), &*f);
}

/** Test populated list. */
void
TestGfxFontList::testList()
{
    afl::base::Ref<gfx::Font> a = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> b = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> c = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> d = *new gfx::BitmapFont();
    afl::base::Ref<gfx::Font> e = *new gfx::BitmapFont();

    gfx::FontList testee;

    testee.addFont(makeRequest(1,0,0,0), a.asPtr());
    testee.addFont(makeRequest(2,1,0,0), b.asPtr());
    testee.addFont(makeRequest(3,2,1,0), c.asPtr());
    testee.addFont(makeRequest(4,3,2,0), d.asPtr());
    testee.addFont(makeRequest(0,0,0,1), e.asPtr());

    // Exact matches
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(1,0,0,0)).get(), &*a);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(2,1,0,0)).get(), &*b);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(3,2,1,0)).get(), &*c);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(4,3,2,0)).get(), &*d);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(0,0,0,1)).get(), &*e);

    // Inexact matches
    // - slant mismatch
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(1,0,1,0)).get(), &*a);

    // - weight mismatch
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(1,1,0,0)).get(), &*a);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(2,2,0,0)).get(), &*b);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(3,2,3,0)).get(), &*c);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(4,7,7,0)).get(), &*d);

    // - size mismatch
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(5,0,0,1)).get(), &*e);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(4,3,2,1)).get(), &*e);
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(3,2,1,1)).get(), &*e);

    // - no match
    TS_ASSERT_EQUALS(testee.findFont(makeRequest(7,7,7,7)).get(), &*a);
}

