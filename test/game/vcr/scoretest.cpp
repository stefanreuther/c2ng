/**
  *  \file test/game/vcr/scoretest.cpp
  *  \brief Test for game::vcr::Score
  */

#include "game/vcr/score.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("game.vcr.Score", a)
{
    // Test initialisation
    game::vcr::Score t;
    a.checkEqual("01. getBuildMillipoints", t.getBuildMillipoints().min(), 0);
    a.checkEqual("02. getBuildMillipoints", t.getBuildMillipoints().max(), 0);
    a.checkEqual("03. getExperience",       t.getExperience().min(), 0);
    a.checkEqual("04. getExperience",       t.getExperience().max(), 0);
    a.checkEqual("05. getTonsDestroyed",    t.getTonsDestroyed().min(), 0);
    a.checkEqual("06. getTonsDestroyed",    t.getTonsDestroyed().max(), 0);

    // Add something
    t.addBuildMillipoints(game::vcr::Score::Range_t(111, 222));
    t.addExperience(game::vcr::Score::Range_t(290, 300));
    t.addTonsDestroyed(game::vcr::Score::Range_t(4444, 4445));
    a.checkEqual("11. getBuildMillipoints", t.getBuildMillipoints().min(), 111);
    a.checkEqual("12. getBuildMillipoints", t.getBuildMillipoints().max(), 222);
    a.checkEqual("13. getExperience",       t.getExperience().min(), 290);
    a.checkEqual("14. getExperience",       t.getExperience().max(), 300);
    a.checkEqual("15. getTonsDestroyed",    t.getTonsDestroyed().min(), 4444);
    a.checkEqual("16. getTonsDestroyed",    t.getTonsDestroyed().max(), 4445);

    // Add again
    t.addBuildMillipoints(game::vcr::Score::Range_t(3, 4));
    t.addExperience(game::vcr::Score::Range_t::fromValue(5));
    t.addTonsDestroyed(game::vcr::Score::Range_t::fromValue(6));
    a.checkEqual("21. getBuildMillipoints", t.getBuildMillipoints().min(), 114);
    a.checkEqual("22. getBuildMillipoints", t.getBuildMillipoints().max(), 226);
    a.checkEqual("23. getExperience",       t.getExperience().min(), 295);
    a.checkEqual("24. getExperience",       t.getExperience().max(), 305);
    a.checkEqual("25. getTonsDestroyed",    t.getTonsDestroyed().min(), 4450);
    a.checkEqual("26. getTonsDestroyed",    t.getTonsDestroyed().max(), 4451);
}
