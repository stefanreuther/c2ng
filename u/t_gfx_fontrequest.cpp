/**
  *  \file u/t_gfx_fontrequest.cpp
  *  \brief Test for gfx::FontRequest
  */

#include "gfx/fontrequest.hpp"

#include "t_gfx.hpp"

/** Test setter/getter. */
void
TestGfxFontRequest::testSet()
{
    gfx::FontRequest testee;

    // Initial state
    TS_ASSERT_EQUALS(testee.getSize().orElse(-1),   0);
    TS_ASSERT_EQUALS(testee.getWeight().orElse(-1), 0);
    TS_ASSERT_EQUALS(testee.getSlant().orElse(-1),  0);
    TS_ASSERT_EQUALS(testee.getStyle().orElse(-1),  0);

    // Set values
    testee.setSize(9);
    testee.setWeight(8);
    testee.setSlant(-3);
    testee.setStyle(7);
    TS_ASSERT_EQUALS(testee.getSize().orElse(-1),   9);
    TS_ASSERT_EQUALS(testee.getWeight().orElse(-1), 8);
    TS_ASSERT_EQUALS(testee.getSlant().orElse(-1),  -3);
    TS_ASSERT_EQUALS(testee.getStyle().orElse(-1),  7);

    // Modify values
    testee.addSize(2);
    testee.addWeight(-1);
    TS_ASSERT_EQUALS(testee.getSize().orElse(-1),   11);
    TS_ASSERT_EQUALS(testee.getWeight().orElse(-1), 7);
    TS_ASSERT_EQUALS(testee.getSlant().orElse(-1),  -3);
    TS_ASSERT_EQUALS(testee.getStyle().orElse(-1),  7);

    // Set to unknown
    testee.setSize(afl::base::Nothing);
    testee.setWeight(afl::base::Nothing);
    testee.setSlant(afl::base::Nothing);
    testee.setStyle(afl::base::Nothing);
    TS_ASSERT_EQUALS(testee.getSize().orElse(-1),   -1);
    TS_ASSERT_EQUALS(testee.getWeight().orElse(-1), -1);
    TS_ASSERT_EQUALS(testee.getSlant().orElse(-1),  -1);
    TS_ASSERT_EQUALS(testee.getStyle().orElse(-1),  -1);

    // Add
    testee.addSize(2);
    testee.addWeight(3);
    TS_ASSERT_EQUALS(testee.getSize().orElse(-1),   2);
    TS_ASSERT_EQUALS(testee.getWeight().orElse(-1), 3);
    TS_ASSERT_EQUALS(testee.getSlant().orElse(-1),  -1);
    TS_ASSERT_EQUALS(testee.getStyle().orElse(-1),  -1);
}

/** Test match. */
void
TestGfxFontRequest::testMatch()
{
    // Default matches default
    TS_ASSERT(gfx::FontRequest().match(gfx::FontRequest()));

    // Configured matches configured
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));

    // Mismatch
    TS_ASSERT(!gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7).match(gfx::FontRequest()));
    TS_ASSERT(!gfx::FontRequest().match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));

    // Partial match unknown/known
    TS_ASSERT(gfx::FontRequest().setSize(afl::base::Nothing).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(afl::base::Nothing).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(afl::base::Nothing).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(afl::base::Nothing)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));

    // Partial match known/unknown
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(afl::base::Nothing).setWeight(9).setSlant(3).setStyle(7)));
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(afl::base::Nothing).setSlant(3).setStyle(7)));
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(afl::base::Nothing).setStyle(7)));
    TS_ASSERT(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(afl::base::Nothing)));
}

