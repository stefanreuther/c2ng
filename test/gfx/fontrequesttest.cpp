/**
  *  \file test/gfx/fontrequesttest.cpp
  *  \brief Test for gfx::FontRequest
  */

#include "gfx/fontrequest.hpp"
#include "afl/test/testrunner.hpp"

/** Test setter/getter. */
AFL_TEST("gfx.FontRequest:basics", a)
{
    gfx::FontRequest testee;

    // Initial state
    a.checkEqual("01. getSize",   testee.getSize().orElse(-1),   0);
    a.checkEqual("02. getWeight", testee.getWeight().orElse(-1), 0);
    a.checkEqual("03. getSlant",  testee.getSlant().orElse(-1),  0);
    a.checkEqual("04. getStyle",  testee.getStyle().orElse(-1),  0);

    // Set values
    testee.setSize(9);
    testee.setWeight(8);
    testee.setSlant(-3);
    testee.setStyle(7);
    a.checkEqual("11. getSize",   testee.getSize().orElse(-1),   9);
    a.checkEqual("12. getWeight", testee.getWeight().orElse(-1), 8);
    a.checkEqual("13. getSlant",  testee.getSlant().orElse(-1),  -3);
    a.checkEqual("14. getStyle",  testee.getStyle().orElse(-1),  7);

    // Modify values
    testee.addSize(2);
    testee.addWeight(-1);
    a.checkEqual("21. getSize",   testee.getSize().orElse(-1),   11);
    a.checkEqual("22. getWeight", testee.getWeight().orElse(-1), 7);
    a.checkEqual("23. getSlant",  testee.getSlant().orElse(-1),  -3);
    a.checkEqual("24. getStyle",  testee.getStyle().orElse(-1),  7);

    // Set to unknown
    testee.setSize(afl::base::Nothing);
    testee.setWeight(afl::base::Nothing);
    testee.setSlant(afl::base::Nothing);
    testee.setStyle(afl::base::Nothing);
    a.checkEqual("31. getSize",   testee.getSize().orElse(-1),   -1);
    a.checkEqual("32. getWeight", testee.getWeight().orElse(-1), -1);
    a.checkEqual("33. getSlant",  testee.getSlant().orElse(-1),  -1);
    a.checkEqual("34. getStyle",  testee.getStyle().orElse(-1),  -1);

    // Add
    testee.addSize(2);
    testee.addWeight(3);
    a.checkEqual("41. getSize",   testee.getSize().orElse(-1),   2);
    a.checkEqual("42. getWeight", testee.getWeight().orElse(-1), 3);
    a.checkEqual("43. getSlant",  testee.getSlant().orElse(-1),  -1);
    a.checkEqual("44. getStyle",  testee.getStyle().orElse(-1),  -1);
}

/** Test match. */
AFL_TEST("gfx.FontRequest:match", a)
{
    // Default matches default
    a.check("01", gfx::FontRequest().match(gfx::FontRequest()));

    // Configured matches configured
    a.check("11", gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));

    // Mismatch
    a.check("21", !gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7).match(gfx::FontRequest()));
    a.check("22", !gfx::FontRequest().match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));

    // Partial match unknown/known
    a.check("31", gfx::FontRequest().setSize(afl::base::Nothing).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));
    a.check("32", gfx::FontRequest().setSize(1).setWeight(afl::base::Nothing).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));
    a.check("33", gfx::FontRequest().setSize(1).setWeight(9).setSlant(afl::base::Nothing).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));
    a.check("34", gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(afl::base::Nothing)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)));

    // Partial match known/unknown
    a.check("41", gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(afl::base::Nothing).setWeight(9).setSlant(3).setStyle(7)));
    a.check("42", gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(afl::base::Nothing).setSlant(3).setStyle(7)));
    a.check("43", gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(afl::base::Nothing).setStyle(7)));
    a.check("44", gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(7)
              .match(gfx::FontRequest().setSize(1).setWeight(9).setSlant(3).setStyle(afl::base::Nothing)));
}

/** Test comparison. */
AFL_TEST("gfx.FontRequest:compare", a)
{
    a.checkEqual("01", gfx::FontRequest() == gfx::FontRequest(), true);
    a.checkEqual("02", gfx::FontRequest() != gfx::FontRequest(), false);

    a.checkEqual("11", gfx::FontRequest().addSize(1) == gfx::FontRequest().addSize(1), true);
    a.checkEqual("12", gfx::FontRequest().addSize(1) != gfx::FontRequest().addSize(1), false);

    a.checkEqual("21", gfx::FontRequest().addSize(1) == gfx::FontRequest().addWeight(2), false);
    a.checkEqual("22", gfx::FontRequest().addSize(1) != gfx::FontRequest().addWeight(2), true);
}

/** Test parse(). */
AFL_TEST("gfx.FontRequest:parse", a)
{
    // All signatures
    a.checkEqual("01", gfx::FontRequest("+")                   == gfx::FontRequest().addSize(1), true);
    a.checkEqual("02", gfx::FontRequest().parse("+")           == gfx::FontRequest().addSize(1), true);
    a.checkEqual("03", gfx::FontRequest(String_t("+"))         == gfx::FontRequest().addSize(1), true);
    a.checkEqual("04", gfx::FontRequest().parse(String_t("+")) == gfx::FontRequest().addSize(1), true);

    // Combinations
    a.checkEqual("11", gfx::FontRequest("+++-") == gfx::FontRequest().addSize(2),   true);
    a.checkEqual("12", gfx::FontRequest("bbb")  == gfx::FontRequest().addWeight(3), true);

    // Variants
    a.checkEqual("21", gfx::FontRequest("b")  == gfx::FontRequest().addWeight(1),  true);
    a.checkEqual("22", gfx::FontRequest("l")  == gfx::FontRequest().addWeight(-1), true);
    a.checkEqual("23", gfx::FontRequest("f")  == gfx::FontRequest().setStyle(1),   true);
    a.checkEqual("24", gfx::FontRequest("fp") == gfx::FontRequest().setStyle(0),   true);
    a.checkEqual("25", gfx::FontRequest("i")  == gfx::FontRequest().setSlant(1),   true);
    a.checkEqual("26", gfx::FontRequest("iu") == gfx::FontRequest().setSlant(0),   true);
}
