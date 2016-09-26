/**
  *  \file u/t_game_teamsettings.cpp
  *  \brief Test for game::TeamSettings
  */

#include "game/teamsettings.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test initialisation.
    Object must report virgin state. */
void
TestGameTeamSettings::testInit()
{
    game::TeamSettings testee;
    TS_ASSERT(!testee.hasAnyTeams());
    TS_ASSERT(!testee.isNamedTeam(9));
}

/** Test setters and getters. */
void
TestGameTeamSettings::testSet()
{
    afl::string::NullTranslator tx;
    game::TeamSettings testee;

    // Set
    testee.setPlayerTeam(1, 2);
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(8, 4);
    testee.setPlayerTeam(999999999, 2);   // out-of-range, must not crash

    // Must preserve
    TS_ASSERT_EQUALS(testee.getPlayerTeam(1), 2);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(9), 2);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(8), 4);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(999999999), 0);  // out-of-range

    // Accessors
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(2), 3);     // 1, 2, 9
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 2);     // 4, 8

    // Some names
    testee.setTeamName(1, "One");
    TS_ASSERT_EQUALS(testee.getTeamName(1, tx), "One");
    TS_ASSERT_EQUALS(testee.getTeamName(2, tx), "Team 2");
    TS_ASSERT(testee.isNamedTeam(1));
    TS_ASSERT(!testee.isNamedTeam(2));
    TS_ASSERT(!testee.isNamedTeam(0));
    TS_ASSERT(!testee.isNamedTeam(999999999));

    // We now have teams
    TS_ASSERT(testee.hasAnyTeams());
}

/** Test other modifications. */
void
TestGameTeamSettings::testModify()
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Verify counts
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(2), 3);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(3), 2);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 1);

    // Remove player 4. Because team 4 is not available they get 1 as the first free one
    testee.removePlayerTeam(4);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(4), 1);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 1);

    // Add 4 to 4.
    testee.setPlayerTeam(4, 4);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(1), 0);

    // Remove 8 from 4. 1 is still free, but because 8 is also free, this one goes to 8.
    testee.removePlayerTeam(8);
    TS_ASSERT_EQUALS(testee.getPlayerTeam(8), 8);
    TS_ASSERT_EQUALS(testee.getNumTeamMembers(4), 1);
}

/** Test viewpoint functions. */
void
TestGameTeamSettings::testViewpoint()
{
    game::TeamSettings testee;
    testee.setPlayerTeam(1, 2);       // 1,2,9 in team 2
    testee.setPlayerTeam(9, 2);
    testee.setPlayerTeam(4, 3);       // 3,4 in team 3
    testee.setPlayerTeam(8, 4);       // 8 in team 4

    // Start with no viewpoint
    TS_ASSERT_EQUALS(testee.getViewpointPlayer(), 0);

    // Set viewpoint
    testee.setViewpointPlayer(9);
    TS_ASSERT_EQUALS(testee.getViewpointPlayer(), 9);

    // Relations
    TS_ASSERT_EQUALS(testee.getPlayerRelation(1), game::TeamSettings::AlliedPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(2), game::TeamSettings::AlliedPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(3), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(4), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(8), game::TeamSettings::EnemyPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(9), game::TeamSettings::ThisPlayer);
    TS_ASSERT_EQUALS(testee.getPlayerRelation(10), game::TeamSettings::EnemyPlayer);
}
