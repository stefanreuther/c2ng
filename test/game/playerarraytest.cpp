/**
  *  \file test/game/playerarraytest.cpp
  *  \brief Test for game::PlayerArray
  */

#include "game/playerarray.hpp"

#include "afl/string/string.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.PlayerArray:array-access", a)
{
    // ex GamePlayerSetTestSuite::testArray()
    game::PlayerArray<int> n;

    // check indexing
    a.check("01", !n.at(-2));
    a.check("02", !n.at(-1));
    a.check("03", n.at(0));
    a.check("04", n.at(1));
    a.check("05", n.at(10));
    a.check("06", n.at(11));
    a.check("07", n.at(12));
    a.check("08", !n.at(-13));
    a.check("09", !n.at(-14));
    a.check("10", !n.at(1000));  // reconsider when we go MMORPG

    // check initialisation
    n.setAll(0);
    a.checkEqual("11", *n.at(0), 0);
    a.checkEqual("12", *n.at(1), 0);
    a.checkEqual("13", *n.at(2), 0);
    a.checkEqual("14", *n.at(10), 0);
    a.checkEqual("15", *n.at(11), 0);
    a.checkEqual("16", *n.at(12), 0);

    // check initialisation
    n.setAll(42);
    a.checkEqual("21", *n.at(0), 42);
    a.checkEqual("22", *n.at(1), 42);
    a.checkEqual("23", *n.at(2), 42);
    a.checkEqual("24", *n.at(10), 42);
    a.checkEqual("25", *n.at(11), 42);
    a.checkEqual("26", *n.at(12), 42);

    // check assignment
    n.set(2, 8);
    a.checkEqual("31", *n.at(0), 42);
    a.checkEqual("32", *n.at(1), 42);
    a.checkEqual("33", *n.at(2), 8);
    a.checkEqual("34", *n.at(3), 42);
    a.checkEqual("35", *n.at(4), 42);

    // check modify-assignment [not relevant in c2ng]
    *n.at(2) += 7;
    a.checkEqual("41", *n.at(0), 42);
    a.checkEqual("42", *n.at(1), 42);
    a.checkEqual("43", *n.at(2), 15);
    a.checkEqual("44", *n.at(3), 42);
    a.checkEqual("45", *n.at(4), 42);

    // check regular read
    a.checkEqual("51", n.get(0), 42);
    a.checkEqual("52", n.get(1), 42);
    a.checkEqual("53", n.get(2), 15);
    a.checkEqual("54", n.get(3), 42);
    a.checkEqual("55", n.get(4), 42);

    // check out-of-bounds read
    a.checkEqual("61", n.get(-1), 0);
    a.checkEqual("62", n.get(999), 0);

    // check out-of-bounds write
    n.set(999999999, 9);
    n.set(-999999999, 9);
}

/** Test initialisation. */
AFL_TEST("game.PlayerArray:init", a)
{
    using game::PlayerArray;

    a.checkEqual("01", PlayerArray<int>().get(1), 0);
    a.checkEqual("02", PlayerArray<int>(42).get(1), 42);

    a.checkEqual("11", PlayerArray<String_t>().get(1), "");
    a.checkEqual("12", PlayerArray<String_t>("x").get(1), "x");
}

/** Test pointer handling.
    We want to safely receive null pointers when out of range. */
AFL_TEST("game.PlayerArray:get", a)
{
    int ia = 10, ib = 20;
    game::PlayerArray<int*> n;
    n.set(3, &ia);
    n.set(4, &ib);

    a.checkNull("01", n.get(-1));
    a.checkNull("02", n.get(0));
    a.check("03", n.get(3) == &ia);
    a.check("04", n.get(4) == &ib);
    a.checkNull("05", n.get(1000));
}

/** Test comparison. */
AFL_TEST("game.PlayerArray:comparison", a)
{
    game::PlayerArray<int> aa, ab, ac;
    aa.setAll(10);
    ab.setAll(10);
    ac.setAll(20);

    a.checkEqual("01", aa == ab, true);
    a.checkEqual("02", aa != ab, false);

    a.checkEqual("11", aa == ac, false);
    a.checkEqual("12", aa != ac, true);

    aa.set(4, 5);
    a.checkEqual("21", aa == ab, false);
    a.checkEqual("22", aa != ab, true);

    aa.set(4, 10);
    a.checkEqual("31", aa == ab, true);
    a.checkEqual("32", aa != ab, false);
}
