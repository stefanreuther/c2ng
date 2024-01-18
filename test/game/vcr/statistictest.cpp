/**
  *  \file test/game/vcr/statistictest.cpp
  *  \brief Test for game::vcr::Statistic
  */

#include "game/vcr/statistic.hpp"

#include "afl/test/testrunner.hpp"
#include "game/vcr/object.hpp"

/** Test Statistic operations.
    A: create a Statistic object
    E: "inquiry" calls report empty content. */
AFL_TEST("game.vcr.Statistic:init", a)
{
    game::vcr::Statistic t;
    a.checkEqual("01. getMinFightersAboard", t.getMinFightersAboard(), 0);
    a.checkEqual("02. getNumTorpedoHits", t.getNumTorpedoHits(), 0);
    a.checkEqual("03. getNumFights", t.getNumFights(), 0);
}

/** Test Statistic operations.
    A: execute a sequence of "record" calls.
    E: "inquiry" calls produce expected results. */
AFL_TEST("game.vcr.Statistic:sequence", a)
{
    game::vcr::Object obj;
    obj.setNumFighters(30);

    // Initialize
    game::vcr::Statistic t;
    t.init(obj, 1);
    a.checkEqual("01. getMinFightersAboard", t.getMinFightersAboard(), 30);
    a.checkEqual("02. getNumTorpedoHits", t.getNumTorpedoHits(), 0);
    a.checkEqual("03. getNumFights", t.getNumFights(), 1);

    // Some action
    t.handleFightersAboard(20);
    t.handleFightersAboard(25);
    t.handleTorpedoHit();
    t.handleTorpedoHit();
    t.handleTorpedoHit();
    a.checkEqual("11. getMinFightersAboard", t.getMinFightersAboard(), 20);
    a.checkEqual("12. getNumTorpedoHits", t.getNumTorpedoHits(), 3);
    a.checkEqual("13. getNumFights", t.getNumFights(), 1);

    // Merge
    game::vcr::Statistic other;
    other.init(obj, 1);
    other.handleTorpedoHit();
    other.handleFightersAboard(12);

    t.merge(other);
    a.checkEqual("21. getMinFightersAboard", t.getMinFightersAboard(), 12);
    a.checkEqual("22. getNumTorpedoHits", t.getNumTorpedoHits(), 4);
    a.checkEqual("23. getNumFights", t.getNumFights(), 2);
}
