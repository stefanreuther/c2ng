/**
  *  \file test/game/spec/hullfunctionlisttest.cpp
  *  \brief Test for game::spec::HullFunctionList
  */

#include "game/spec/hullfunctionlist.hpp"
#include "afl/test/testrunner.hpp"

/** Test simplify(). */
AFL_TEST("game.spec.HullFunctionList:simplify:hull+race", a)
{
    // Simplify, border case
    game::spec::HullFunction oneR(42);
    game::spec::HullFunction oneH(42);
    oneR.setKind(game::spec::HullFunction::AssignedToRace);
    oneH.setKind(game::spec::HullFunction::AssignedToHull);

    game::spec::HullFunctionList hfl;
    hfl.add(oneR);
    hfl.add(oneH);
    a.checkEqual("01. size", hfl.size(), 2U);
    hfl.simplify();

    a.checkEqual("11. size", hfl.size(), 1U);
    a.checkEqual("12. getBasicFunctionId", hfl[0].getBasicFunctionId(), 42);
}

/** Test sort(). */
AFL_TEST("game.spec.HullFunctionList:sort", a)
{
    using game::spec::HullFunction;

    game::spec::HullFunctionList hfl;

    // Add some hull functions


    // AssignedToShip/Hull
    //   for player
    //     lower level
    //       basic function
    //         AssignedToShip
    //           player
    //         AssignedToHull
    //     higher levels
    //   not for player
    // AssignedToRace

    {
        HullFunction f(1);
        f.setKind(HullFunction::AssignedToRace);
        hfl.add(f);
    }
    {
        HullFunction f(2);
        f.setKind(HullFunction::AssignedToHull);
        f.setPlayers(game::PlayerSet_t(2));
        hfl.add(f);
    }
    {
        HullFunction f(3);
        f.setKind(HullFunction::AssignedToHull);
        f.setPlayers(game::PlayerSet_t(1));
        f.setLevels(game::ExperienceLevelSet_t(3));
        hfl.add(f);
    }
    {
        HullFunction f(10);
        f.setKind(HullFunction::AssignedToHull);
        f.setPlayers(game::PlayerSet_t(1));
        f.setLevels(game::ExperienceLevelSet_t(2));
        hfl.add(f);
    }
    {
        HullFunction f(4);
        f.setKind(HullFunction::AssignedToHull);
        f.setPlayers(game::PlayerSet_t(1));
        f.setLevels(game::ExperienceLevelSet_t(2));
        hfl.add(f);
    }
    {
        HullFunction f(4);
        f.setKind(HullFunction::AssignedToShip);
        f.setPlayers(game::PlayerSet_t(1));
        f.setLevels(game::ExperienceLevelSet_t(2) + 3);
        hfl.add(f);
    }
    {
        HullFunction f(4);
        f.setKind(HullFunction::AssignedToShip);
        f.setPlayers(game::PlayerSet_t(1) + 2);
        f.setLevels(game::ExperienceLevelSet_t(2));
        hfl.add(f);
    }
    {
        HullFunction f(4);
        f.setKind(HullFunction::AssignedToShip);
        f.setPlayers(game::PlayerSet_t(1));
        f.setLevels(game::ExperienceLevelSet_t(2));
        hfl.add(f);
    }

    // Sort
    hfl.sortForNewShip(game::PlayerSet_t(1));
    a.checkEqual("01. size", hfl.size(), 8U);

    a.checkEqual("11", hfl[0].getBasicFunctionId(), 4);
    a.checkEqual("12", hfl[0].getPlayers(), game::PlayerSet_t(1));
    a.checkEqual("13", hfl[0].getLevels(),  game::ExperienceLevelSet_t(2));
    a.checkEqual("14", hfl[0].getKind(),    HullFunction::AssignedToShip);

    a.checkEqual("21", hfl[1].getBasicFunctionId(), 4);
    a.checkEqual("22", hfl[1].getPlayers(), game::PlayerSet_t(1) + 2);
    a.checkEqual("23", hfl[1].getLevels(),  game::ExperienceLevelSet_t(2));
    a.checkEqual("24", hfl[1].getKind(),    HullFunction::AssignedToShip);

    a.checkEqual("31", hfl[2].getBasicFunctionId(), 4);
    a.checkEqual("32", hfl[2].getPlayers(), game::PlayerSet_t(1));
    a.checkEqual("33", hfl[2].getLevels(),  game::ExperienceLevelSet_t(2));
    a.checkEqual("34", hfl[2].getKind(),    HullFunction::AssignedToHull);

    a.checkEqual("41", hfl[3].getBasicFunctionId(), 10);
    a.checkEqual("42", hfl[3].getPlayers(), game::PlayerSet_t(1));
    a.checkEqual("43", hfl[3].getLevels(),  game::ExperienceLevelSet_t(2));
    a.checkEqual("44", hfl[3].getKind(),    HullFunction::AssignedToHull);

    a.checkEqual("51", hfl[4].getBasicFunctionId(), 4);
    a.checkEqual("52", hfl[4].getPlayers(), game::PlayerSet_t(1));
    a.checkEqual("53", hfl[4].getLevels(),  game::ExperienceLevelSet_t(2) + 3);
    a.checkEqual("54", hfl[4].getKind(),    HullFunction::AssignedToShip);

    a.checkEqual("61", hfl[5].getBasicFunctionId(), 3);
    a.checkEqual("62", hfl[5].getPlayers(), game::PlayerSet_t(1));
    a.checkEqual("63", hfl[5].getLevels(),  game::ExperienceLevelSet_t(3));
    a.checkEqual("64", hfl[5].getKind(),    HullFunction::AssignedToHull);

    a.checkEqual("71", hfl[6].getBasicFunctionId(), 2);
    a.checkEqual("72", hfl[6].getPlayers(), game::PlayerSet_t(2));
    a.checkEqual("73", hfl[6].getKind(),    HullFunction::AssignedToHull);

    a.checkEqual("81", hfl[7].getBasicFunctionId(), 1);
    a.checkEqual("82", hfl[7].getKind(),    HullFunction::AssignedToRace);
}

/** Simple final test. */
AFL_TEST("game.spec.HullFunctionList:basics", a)
{
    // Test initial state
    game::spec::HullFunctionList testee;
    a.checkEqual("01. size", testee.size(), 0U);
    a.check("02. iterator", testee.begin() == testee.end());

    // Add
    testee.add(game::spec::HullFunction(1));
    testee.add(game::spec::HullFunction(3));
    testee.add(game::spec::HullFunction(5));

    // Test
    a.checkEqual("11. size", testee.size(), 3U);
    a.check("12. iterator", testee.begin() != testee.end());

    // Clear
    testee.clear();
    a.checkEqual("21. size", testee.size(), 0U);
}

/** Sort levels. */
AFL_TEST("game.spec.HullFunctionList:sort:levels", a)
{
    // Build a set
    game::spec::HullFunctionList testee;
    testee.add(game::spec::HullFunction(7, game::ExperienceLevelSet_t() + 1));
    testee.add(game::spec::HullFunction(7, game::ExperienceLevelSet_t() + 1 + 2 + 3 + 4));
    testee.add(game::spec::HullFunction(7, game::ExperienceLevelSet_t() + 1 + 2));
    testee.add(game::spec::HullFunction(7, game::ExperienceLevelSet_t() + 1 + 2 + 3));
    testee.add(game::spec::HullFunction(7, game::ExperienceLevelSet_t() + 1 + 2 + 3 + 4 + 5));
    testee.sortForNewShip(game::PlayerSet_t(1));

    // Verify
    a.checkEqual("01. size", testee.size(), 5U);
    a.checkEqual("02", testee[0].getLevels(), game::ExperienceLevelSet_t() + 1);
    a.checkEqual("03", testee[1].getLevels(), game::ExperienceLevelSet_t() + 1 + 2);
    a.checkEqual("04", testee[2].getLevels(), game::ExperienceLevelSet_t() + 1 + 2 + 3);
    a.checkEqual("05", testee[3].getLevels(), game::ExperienceLevelSet_t() + 1 + 2 + 3 + 4);
    a.checkEqual("06", testee[4].getLevels(), game::ExperienceLevelSet_t() + 1 + 2 + 3 + 4 + 5);

    // Verify content using iterator interface
    size_t n = 0;
    for (game::spec::HullFunctionList::Iterator_t it = testee.begin(); it != testee.end(); ++it) {
        a.checkEqual("11. getBasicFunctionId", it->getBasicFunctionId(), 7);
        ++n;
    }
    a.checkEqual("12. count", n, 5U);
}

/** Test simplify() on an empty list. */
AFL_TEST("game.spec.HullFunctionList:simplify:empty", a)
{
    game::spec::HullFunctionList testee;
    testee.simplify();
    a.checkEqual("01. size", testee.size(), 0U);
}

/** Test simplify() on a one-element list. */
AFL_TEST("game.spec.HullFunctionList:simplify:single", a)
{
    game::spec::HullFunctionList testee;
    testee.add(game::spec::HullFunction(99));
    testee.simplify();
    a.checkEqual("01. size", testee.size(), 1U);
}

/** Test simplify() that merges assignments. */
AFL_TEST("game.spec.HullFunctionList:simplify:merge", a)
{
    game::spec::HullFunctionList testee;

    // Prepare
    {
        game::spec::HullFunction hf(42);
        hf.setPlayers(game::PlayerSet_t(1));
        testee.add(hf);
    }
    {
        game::spec::HullFunction hf(43);
        hf.setPlayers(game::PlayerSet_t() + 1 + 2);
        testee.add(hf);
    }
    {
        game::spec::HullFunction hf(42);
        hf.setPlayers(game::PlayerSet_t(2));
        testee.add(hf);
    }
    a.checkEqual("01. size", testee.size(), 3U);

    // Sort
    testee.simplify();

    // Verify
    a.checkEqual("11. size", testee.size(), 2U);
    a.checkEqual("12", testee[0].getPlayers(), game::PlayerSet_t() + 1 + 2);
    a.checkEqual("13", testee[1].getPlayers(), game::PlayerSet_t() + 1 + 2);
}

/** Test simplify() with a racial ability. */
AFL_TEST("game.spec.HullFunctionList:simplify:racial-ability", a)
{
    game::spec::HullFunctionList testee;

    // Racial ability for some races
    {
        game::spec::HullFunction ra(33);
        ra.setKind(game::spec::HullFunction::AssignedToRace);
        ra.setPlayers(game::PlayerSet_t() + 3 + 5);
        testee.add(ra);
    }

    // Hull function for everyone
    {
        game::spec::HullFunction hf(33);
        hf.setKind(game::spec::HullFunction::AssignedToHull);
        hf.setPlayers(game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
        testee.add(hf);
    }

    // Sort
    testee.simplify();

    // Verify. The hull function remains.
    a.checkEqual("01. size", testee.size(), 1U);
    a.checkEqual("02", testee[0].getKind(), game::spec::HullFunction::AssignedToHull);
    a.checkEqual("03", testee[0].getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    a.checkEqual("04", testee[0].getBasicFunctionId(), 33);
}

/** Test simplify() with a racial ability, other case. */
AFL_TEST("game.spec.HullFunctionList:simplify:racial-ability:other", a)
{
    game::spec::HullFunctionList testee;

    // Racial ability for everyone
    {
        game::spec::HullFunction ra(33);
        ra.setKind(game::spec::HullFunction::AssignedToRace);
        ra.setPlayers(game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
        testee.add(ra);
    }

    // Hull function for some races
    {
        game::spec::HullFunction hf(33);
        hf.setKind(game::spec::HullFunction::AssignedToHull);
        hf.setPlayers(game::PlayerSet_t() + 4 + 7);
        testee.add(hf);
    }

    // Sort
    testee.simplify();

    // Verify. The ability remains.
    a.checkEqual("01. size", testee.size(), 1U);
    a.checkEqual("02", testee[0].getKind(), game::spec::HullFunction::AssignedToRace);
    a.checkEqual("03", testee[0].getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    a.checkEqual("04", testee[0].getBasicFunctionId(), 33);
}

/** Test simplify() with a racial ability which is not hit. */
AFL_TEST("game.spec.HullFunctionList:simplify:racial-ability:mismatch", a)
{
    game::spec::HullFunctionList testee;

    // Racial ability for everyone
    {
        game::spec::HullFunction ra(33);
        ra.setKind(game::spec::HullFunction::AssignedToRace);
        ra.setPlayers(game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
        testee.add(ra);
    }

    // Ship function for some races
    {
        game::spec::HullFunction hf(33);
        hf.setKind(game::spec::HullFunction::AssignedToShip);
        hf.setPlayers(game::PlayerSet_t() + 4 + 7);
        testee.add(hf);
    }

    // Sort
    testee.simplify();

    // Verify. Both remain.
    a.checkEqual("01. size", testee.size(), 2U);
}

/** Test simplify(), general case. */
AFL_TEST("game.spec.HullFunctionList:simplify:general", a)
{
    game::spec::HullFunctionList testee;

    // Racial ability for some races
    {
        game::spec::HullFunction ra(33);
        ra.setKind(game::spec::HullFunction::AssignedToRace);
        ra.setPlayers(game::PlayerSet_t() + 3 + 5);
        testee.add(ra);
    }

    // Ship function for everyone
    {
        game::spec::HullFunction sf(33);
        sf.setKind(game::spec::HullFunction::AssignedToShip);
        sf.setPlayers(game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
        testee.add(sf);
    }

    // Non-exhaustive hull function
    {
        game::spec::HullFunction hf(33);
        hf.setKind(game::spec::HullFunction::AssignedToHull);
        hf.setPlayers(game::PlayerSet_t() + 5 + 7);
        testee.add(hf);
    }

    // Something else
    {
        game::spec::HullFunction hf(44);
        hf.setKind(game::spec::HullFunction::AssignedToHull);
        hf.setPlayers(game::PlayerSet_t() + 1 + 7);
        hf.setLevels(game::ExperienceLevelSet_t() + 1 + 2 + 3);
        testee.add(hf);
    }

    // Something else
    {
        game::spec::HullFunction ra(44);
        ra.setKind(game::spec::HullFunction::AssignedToRace);
        ra.setPlayers(game::PlayerSet_t(1));
        testee.add(ra);
    }

    // Simplify should not change the number of assignments
    testee.simplify();
    a.checkEqual("01. size", testee.size(), 5U);
}

/** Test removal of null assigments. */
AFL_TEST("game.spec.HullFunctionList:simplify:null-assignment", a)
{
    game::spec::HullFunctionList testee;

    // Three elements
    {
        game::spec::HullFunction a(55);
        a.setKind(game::spec::HullFunction::AssignedToHull);
        a.setPlayers(game::PlayerSet_t(1));
        testee.add(a);
    }
    {
        game::spec::HullFunction b(56);
        b.setKind(game::spec::HullFunction::AssignedToHull);
        b.setPlayers(game::PlayerSet_t());
        testee.add(b);
    }
    {
        game::spec::HullFunction c(57);
        c.setKind(game::spec::HullFunction::AssignedToHull);
        c.setPlayers(game::PlayerSet_t(9));
        testee.add(c);
    }

    // Test
    testee.simplify();

    // Verify
    a.checkEqual("01. size", testee.size(), 2U);
    a.checkEqual("02", testee[0].getBasicFunctionId(), 55);
    a.checkEqual("03", testee[1].getBasicFunctionId(), 57);
}
