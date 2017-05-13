/**
  *  \file u/t_game_spec_hullfunctionlist.cpp
  */

#include "game/spec/hullfunctionlist.hpp"

#include "u/t_game_spec.hpp"

/** Test simplify(). */
void
TestGameSpecHullFunctionList::testSimplify()
{
    // Simplify, border case
    game::spec::HullFunction oneR(42);
    game::spec::HullFunction oneH(42);
    oneR.setKind(game::spec::HullFunction::AssignedToRace);
    oneH.setKind(game::spec::HullFunction::AssignedToHull);

    game::spec::HullFunctionList hfl;
    hfl.add(oneR);
    hfl.add(oneH);
    TS_ASSERT_EQUALS(hfl.size(), 2U);
    hfl.simplify();

    TS_ASSERT_EQUALS(hfl.size(), 1U);
    TS_ASSERT_EQUALS(hfl[0].getBasicFunctionId(), 42);
}

/** Test sort(). */
void
TestGameSpecHullFunctionList::testSort()
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
    TS_ASSERT_EQUALS(hfl.size(), 8U);

    TS_ASSERT_EQUALS(hfl[0].getBasicFunctionId(), 4);
    TS_ASSERT_EQUALS(hfl[0].getPlayers(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(hfl[0].getLevels(),  game::ExperienceLevelSet_t(2));
    TS_ASSERT_EQUALS(hfl[0].getKind(),    HullFunction::AssignedToShip);

    TS_ASSERT_EQUALS(hfl[1].getBasicFunctionId(), 4);
    TS_ASSERT_EQUALS(hfl[1].getPlayers(), game::PlayerSet_t(1) + 2);
    TS_ASSERT_EQUALS(hfl[1].getLevels(),  game::ExperienceLevelSet_t(2));
    TS_ASSERT_EQUALS(hfl[1].getKind(),    HullFunction::AssignedToShip);

    TS_ASSERT_EQUALS(hfl[2].getBasicFunctionId(), 4);
    TS_ASSERT_EQUALS(hfl[2].getPlayers(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(hfl[2].getLevels(),  game::ExperienceLevelSet_t(2));
    TS_ASSERT_EQUALS(hfl[2].getKind(),    HullFunction::AssignedToHull);

    TS_ASSERT_EQUALS(hfl[3].getBasicFunctionId(), 10);
    TS_ASSERT_EQUALS(hfl[3].getPlayers(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(hfl[3].getLevels(),  game::ExperienceLevelSet_t(2));
    TS_ASSERT_EQUALS(hfl[3].getKind(),    HullFunction::AssignedToHull);

    TS_ASSERT_EQUALS(hfl[4].getBasicFunctionId(), 4);
    TS_ASSERT_EQUALS(hfl[4].getPlayers(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(hfl[4].getLevels(),  game::ExperienceLevelSet_t(2) + 3);
    TS_ASSERT_EQUALS(hfl[4].getKind(),    HullFunction::AssignedToShip);

    TS_ASSERT_EQUALS(hfl[5].getBasicFunctionId(), 3);
    TS_ASSERT_EQUALS(hfl[5].getPlayers(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(hfl[5].getLevels(),  game::ExperienceLevelSet_t(3));
    TS_ASSERT_EQUALS(hfl[5].getKind(),    HullFunction::AssignedToHull);

    TS_ASSERT_EQUALS(hfl[6].getBasicFunctionId(), 2);
    TS_ASSERT_EQUALS(hfl[6].getPlayers(), game::PlayerSet_t(2));
    TS_ASSERT_EQUALS(hfl[6].getKind(),    HullFunction::AssignedToHull);

    TS_ASSERT_EQUALS(hfl[7].getBasicFunctionId(), 1);
    TS_ASSERT_EQUALS(hfl[7].getKind(),    HullFunction::AssignedToRace);
}

/** Simple final test. */
void
TestGameSpecHullFunctionList::testIt()
{
    // Test initial state
    game::spec::HullFunctionList testee;
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());

    // Add
    testee.add(game::spec::HullFunction(1));
    testee.add(game::spec::HullFunction(3));
    testee.add(game::spec::HullFunction(5));

    // Test
    TS_ASSERT_EQUALS(testee.size(), 3U);
    TS_ASSERT_DIFFERS(testee.begin(), testee.end());

    // Clear
    testee.clear();
    TS_ASSERT_EQUALS(testee.size(), 0U);
}

/** Sort levels. */
void
TestGameSpecHullFunctionList::testSortLevels()
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
    TS_ASSERT_EQUALS(testee.size(), 5U);
    TS_ASSERT_EQUALS(testee[0].getLevels(), game::ExperienceLevelSet_t() + 1);
    TS_ASSERT_EQUALS(testee[1].getLevels(), game::ExperienceLevelSet_t() + 1 + 2);
    TS_ASSERT_EQUALS(testee[2].getLevels(), game::ExperienceLevelSet_t() + 1 + 2 + 3);
    TS_ASSERT_EQUALS(testee[3].getLevels(), game::ExperienceLevelSet_t() + 1 + 2 + 3 + 4);
    TS_ASSERT_EQUALS(testee[4].getLevels(), game::ExperienceLevelSet_t() + 1 + 2 + 3 + 4 + 5);

    // Verify content using iterator interface
    size_t n = 0;
    for (game::spec::HullFunctionList::Iterator_t it = testee.begin(); it != testee.end(); ++it) {
        TS_ASSERT_EQUALS(it->getBasicFunctionId(), 7);
        ++n;
    }
    TS_ASSERT_EQUALS(n, 5U);
}

/** Test simplify() on an empty list. */
void
TestGameSpecHullFunctionList::testSimplifyEmpty()
{
    game::spec::HullFunctionList testee;
    testee.simplify();
    TS_ASSERT_EQUALS(testee.size(), 0U);
}

/** Test simplify() on a one-element list. */
void
TestGameSpecHullFunctionList::testSimplifySingle()
{
    game::spec::HullFunctionList testee;
    testee.add(game::spec::HullFunction(99));
    testee.simplify();
    TS_ASSERT_EQUALS(testee.size(), 1U);
}

/** Test simplify() that merges assignments. */
void
TestGameSpecHullFunctionList::testSimplifyMerge()
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
    TS_ASSERT_EQUALS(testee.size(), 3U);

    // Sort
    testee.simplify();

    // Verify
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT_EQUALS(testee[0].getPlayers(), game::PlayerSet_t() + 1 + 2);
    TS_ASSERT_EQUALS(testee[1].getPlayers(), game::PlayerSet_t() + 1 + 2);
}

/** Test simplify() with a racial ability. */
void
TestGameSpecHullFunctionList::testSimplifyRace()
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
    TS_ASSERT_EQUALS(testee.size(), 1U);
    TS_ASSERT_EQUALS(testee[0].getKind(), game::spec::HullFunction::AssignedToHull);
    TS_ASSERT_EQUALS(testee[0].getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(testee[0].getBasicFunctionId(), 33);
}

/** Test simplify() with a racial ability, other case. */
void
TestGameSpecHullFunctionList::testSimplifyRace2()
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
    TS_ASSERT_EQUALS(testee.size(), 1U);
    TS_ASSERT_EQUALS(testee[0].getKind(), game::spec::HullFunction::AssignedToRace);
    TS_ASSERT_EQUALS(testee[0].getPlayers(), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(testee[0].getBasicFunctionId(), 33);
}

/** Test simplify() with a racial ability which is not hit. */
void
TestGameSpecHullFunctionList::testSimplifyNotRace()
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
    TS_ASSERT_EQUALS(testee.size(), 2U);
}

/** Test simplify(), general case. */
void
TestGameSpecHullFunctionList::testSimplifyGeneral()
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
    TS_ASSERT_EQUALS(testee.size(), 5U);
}

/** Test removal of null assigments. */
void
TestGameSpecHullFunctionList::testSimplifyNullAssignment()
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
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT_EQUALS(testee[0].getBasicFunctionId(), 55);
    TS_ASSERT_EQUALS(testee[1].getBasicFunctionId(), 57);
}
