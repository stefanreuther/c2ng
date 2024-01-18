/**
  *  \file test/game/config/costarrayoptiontest.cpp
  *  \brief Test for game::config::CostArrayOption
  */

#include "game/config/costarrayoption.hpp"
#include "afl/test/testrunner.hpp"

/** Test set(), case 1. */
AFL_TEST("game.config.CostArrayOption:set:1", a)
{
    game::config::CostArrayOption testee;
    a.check("01. isChanged", !testee.isChanged());

    testee.set("T10 D20 M30");
    a.check("11. isChanged", testee.isChanged());
    a.checkEqual("12. tri", testee(1).get(game::spec::Cost::Tritanium), 10);
    a.checkEqual("13. dur", testee(1).get(game::spec::Cost::Duranium), 20);
    a.checkEqual("14. mol", testee(1).get(game::spec::Cost::Molybdenum), 30);

    a.checkEqual("21. tri", testee(10).get(game::spec::Cost::Tritanium), 10);
    a.checkEqual("22. dur", testee(10).get(game::spec::Cost::Duranium), 20);
    a.checkEqual("23. mol", testee(10).get(game::spec::Cost::Molybdenum), 30);

    // out-of-range
    a.checkEqual("31. range", testee(100).get(game::spec::Cost::Tritanium), 10);
    a.checkEqual("32. rage", testee(-1).get(game::spec::Cost::Tritanium), 10);

    a.checkEqual("41. toString", testee.toString(), "T10 D20 M30");
}

/** Test set(), case 2. */
AFL_TEST("game.config.CostArrayOption:set:2", a)
{
    game::config::CostArrayOption testee;
    testee.set("T10,T20,T30,T40,T50");
    a.checkEqual("01. tri", testee(1).get(game::spec::Cost::Tritanium), 10);
    a.checkEqual("02. dur", testee(1).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("03. mol", testee(1).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("11. tri", testee(2).get(game::spec::Cost::Tritanium), 20);
    a.checkEqual("12. dur", testee(2).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("13. mol", testee(2).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("21. tri", testee(5).get(game::spec::Cost::Tritanium), 50);
    a.checkEqual("22. dur", testee(5).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("23. mol", testee(5).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("31. tri", testee(6).get(game::spec::Cost::Tritanium), 50);
    a.checkEqual("32. dur", testee(6).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("33. mol", testee(6).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("41. tri", testee(10).get(game::spec::Cost::Tritanium), 50);
    a.checkEqual("42. dur", testee(10).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("43. mol", testee(10).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("51. toString", testee.toString(), "T10,T20,T30,T40,T50,T50,T50,T50,T50,T50,T50");
}

/** Test set(), case 2. */
AFL_TEST("game.config.CostArrayOption:set:3", a)
{
    game::config::CostArrayOption testee;
    testee.set("T10");
    testee.set(2, game::spec::Cost::fromString("M5"));

    a.checkEqual("01. tri", testee(1).get(game::spec::Cost::Tritanium), 10);
    a.checkEqual("02. dur", testee(1).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("03. mol", testee(1).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("11. tri", testee(2).get(game::spec::Cost::Tritanium), 0);
    a.checkEqual("12. dur", testee(2).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("13. mol", testee(2).get(game::spec::Cost::Molybdenum), 5);

    a.checkEqual("21. tri", testee(3).get(game::spec::Cost::Tritanium), 10);
    a.checkEqual("22. dur", testee(3).get(game::spec::Cost::Duranium), 0);
    a.checkEqual("23. mol", testee(3).get(game::spec::Cost::Molybdenum), 0);

    a.checkEqual("31. toString", testee.toString(), "T10,M5,T10,T10,T10,T10,T10,T10,T10,T10,T10");
}

/** Test formatting, various cases. */
AFL_TEST("game.config.CostArrayOption:toString:1", a)
{
    game::config::CostArrayOption testee;
    testee.set("T10,T20");
    a.checkEqual("01. toString", testee.toString(), "T10,T20,T20,T20,T20,T20,T20,T20,T20,T20,T20");
}

AFL_TEST("game.config.CostArrayOption:toString:2", a)
{
    game::config::CostArrayOption testee;
    testee.set("T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13");
    a.checkEqual("02. toString", testee.toString(), "T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13");
}

AFL_TEST("game.config.CostArrayOption:toString:3", a)
{
    game::config::CostArrayOption testee;
    testee.set("T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T13,T13,T13,T13,T13");
    a.checkEqual("03. toString", testee.toString(), "T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13");
}

AFL_TEST("game.config.CostArrayOption:toString:4", a)
{
    game::config::CostArrayOption testee;
    testee.set("T1,T2,T3,T4,T5,T6,T7,T8,T9,T9,T9,T9,T9,T9");
    a.checkEqual("04. toString", testee.toString(), "T1,T2,T3,T4,T5,T6,T7,T8,T9,T9,T9");
}

AFL_TEST("game.config.CostArrayOption:toString:5", a)
{
    game::config::CostArrayOption testee;
    testee.set("T20,T20,T20,T20,T20,T20,T20,T20,T20,T20,T20");
    a.checkEqual("05. toString", testee.toString(), "T20");
}
