/**
  *  \file test/game/spec/hullassignmentlisttest.cpp
  *  \brief Test for game::spec::HullAssignmentList
  */

#include "game/spec/hullassignmentlist.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"

/** Simple test. */
AFL_TEST("game.spec.HullAssignmentList:basics", a)
{
    // A configuration
    afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
    game::config::HostConfiguration& config = *rconfig;
    config[config.MapTruehullByPlayerRace].set(false);

    // Configure a testee
    game::spec::HullAssignmentList testee;
    for (int player = 1; player <= 5; ++player) {
        for (int slot = 1; slot <= 10; ++slot) {
            testee.add(player, slot, 100*player+slot);
        }
    }

    // Out-of-range access (all ignored)
    testee.add(1, 1, 0);
    testee.add(1, 0, 1);
    testee.add(0, 1, 1);

    testee.add(1, 1, -1);
    testee.add(1, -1, 1);
    testee.add(-1, 1, 1);

    // Verify access
    a.checkEqual("01. getHullFromIndex", testee.getHullFromIndex(config, 1, 1), 101);
    a.checkEqual("02. getHullFromIndex", testee.getHullFromIndex(config, 2, 2), 202);
    a.checkEqual("03. getHullFromIndex", testee.getHullFromIndex(config, 5, 10), 510);

    a.checkEqual("11. getHullFromIndex", testee.getHullFromIndex(config, 0, 0), 0);
    a.checkEqual("12. getHullFromIndex", testee.getHullFromIndex(config, -1, -1), 0);
    a.checkEqual("13. getHullFromIndex", testee.getHullFromIndex(config, 6, 6), 0);

    a.checkEqual("21. getMaxIndex", testee.getMaxIndex(config, 0), 0);
    a.checkEqual("22. getMaxIndex", testee.getMaxIndex(config, 1), 10);
    a.checkEqual("23. getMaxIndex", testee.getMaxIndex(config, 5), 10);
    a.checkEqual("24. getMaxIndex", testee.getMaxIndex(config, 6), 0);

    a.checkEqual("31. getIndexFromHull", testee.getIndexFromHull(config, 1, 107), 7);
    a.checkEqual("32. getIndexFromHull", testee.getIndexFromHull(config, 1, 111), 0);
    a.checkEqual("33. getIndexFromHull", testee.getIndexFromHull(config, 1, 201), 0);
    a.checkEqual("34. getIndexFromHull", testee.getIndexFromHull(config, 2, 201), 1);

    a.checkEqual("41. getPlayersForHull", testee.getPlayersForHull(config, 107), game::PlayerSet_t(1));
    a.checkEqual("42. getPlayersForHull", testee.getPlayersForHull(config, 201), game::PlayerSet_t(2));
    a.checkEqual("43. getPlayersForHull", testee.getPlayersForHull(config, 501), game::PlayerSet_t(5));
    a.checkEqual("44. getPlayersForHull", testee.getPlayersForHull(config, 999), game::PlayerSet_t());

    // Selective clear
    a.checkEqual("51. getHullFromIndex", testee.getHullFromIndex(config, 3, 5), 305);
    a.checkEqual("52. getIndexFromHull", testee.getIndexFromHull(config, 3, 305), 5);
    testee.clearPlayer(3);
    a.checkEqual("53. getHullFromIndex", testee.getHullFromIndex(config, 3, 5), 0);
    a.checkEqual("54. getIndexFromHull", testee.getIndexFromHull(config, 3, 305), 0);

    // Full clear
    testee.clear();
    a.checkEqual("61. getHullFromIndex", testee.getHullFromIndex(config, 1, 1), 0);
    a.checkEqual("62. getHullFromIndex", testee.getHullFromIndex(config, 2, 2), 0);
    a.checkEqual("63. getHullFromIndex", testee.getHullFromIndex(config, 5, 10), 0);
}

/** Test PlayerRace. */
AFL_TEST("game.spec.HullAssignmentList:PlayerRace", a)
{
    // A configuration
    afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
    game::config::HostConfiguration& config = *rconfig;
    config[config.MapTruehullByPlayerRace].set(false);
    config[config.PlayerRace].set("6,5,4,3,2,1");

    // Configure a testee
    game::spec::HullAssignmentList testee;
    for (int player = 1; player <= 5; ++player) {
        for (int slot = 1; slot <= 10; ++slot) {
            testee.add(player, slot, 100*player+slot);
        }
    }

    // Default
    a.checkEqual("01. getHullFromIndex", testee.getHullFromIndex(config, 1, 5), 105);
    a.checkEqual("02. getHullFromIndex", testee.getHullFromIndex(config, 2, 5), 205);

    // Reconfigure
    config[config.MapTruehullByPlayerRace].set(true);

    // Default
    a.checkEqual("11. getHullFromIndex", testee.getHullFromIndex(config, 1, 5), 0);
    a.checkEqual("12. getHullFromIndex", testee.getHullFromIndex(config, 2, 5), 505);
    a.checkEqual("13. getIndexFromHull", testee.getIndexFromHull(config, 1, 505), 0);
    a.checkEqual("14. getIndexFromHull", testee.getIndexFromHull(config, 2, 505), 5);
    a.checkEqual("15. getMaxIndex", testee.getMaxIndex(config, 1), 0);
    a.checkEqual("16. getMaxIndex", testee.getMaxIndex(config, 2), 10);
}
