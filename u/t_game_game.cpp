/**
  *  \file u/t_game_game.cpp
  *  \brief Test for game::Game
  */

#include "game/game.hpp"

#include "t_game.hpp"
#include "game/alliance/container.hpp"
#include "game/alliance/level.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/turn.hpp"
#include "game/parser/messageinformation.hpp"

using game::config::HostConfiguration;

/** Test smart pointers.
    A: pass Game object from smart through dumb pointer.
    E: code executes correctly, no reference-count confusion */
void
TestGameGame::testRef()
{
    // Create a game and place in smart pointer
    afl::base::Ptr<game::Game> sp = new game::Game();
    game::Game* dp = sp.get();

    // Create smart pointer from dumb one
    {
        afl::base::Ptr<game::Game> sp2 = dp;
    }

    // If the pointers didn't work, this will access unallocated memory.
    dp->notifyListeners();
}

/** Test initialisation.
    A: create empty Game
    E: expected initial values */
void
TestGameGame::testInit()
{
    game::Game t;
    TS_ASSERT_EQUALS(t.getViewpointPlayer(), 0);
    TS_ASSERT_EQUALS(t.getViewpointTurnNumber(), 0);
    TS_ASSERT_EQUALS(t.getViewpointTurn().get(), &t.currentTurn());
}

/** Test subobjects.
    A: create Game. Access sub-objects through mutable and const path
    E: both paths produce the same object */
void
TestGameGame::testSubobjects()
{
    game::Game t;
    const game::Game& ct = t;

    TS_ASSERT_EQUALS(&t.currentTurn(), &ct.currentTurn());
    TS_ASSERT_EQUALS(&t.previousTurns(), &ct.previousTurns());
    TS_ASSERT_EQUALS(&t.planetScores(), &ct.planetScores());
    TS_ASSERT_EQUALS(&t.shipScores(), &ct.shipScores());
    TS_ASSERT_EQUALS(&t.teamSettings(), &ct.teamSettings());
    TS_ASSERT_EQUALS(&t.scores(), &ct.scores());
    TS_ASSERT_EQUALS(&t.messageConfiguration(), &ct.messageConfiguration());

    TS_ASSERT_DIFFERS(&t.planetScores(), &t.shipScores());
}

/** Test viewpoint turn access.
    A: create a game; add history turn
    E: current and history turn correctly settable as viewpoint turn */
void
TestGameGame::testViewpointTurn()
{
    // Set turn number. Must immediately be reflected as getViewpointTurnNumber().
    game::Game t;
    t.currentTurn().setTurnNumber(12);
    TS_ASSERT_EQUALS(t.getViewpointTurnNumber(), 12);
    TS_ASSERT_EQUALS(t.getViewpointTurn().get(), &t.currentTurn());

    // Add a history turn. Must be retrievable.
    afl::base::Ref<game::Turn> ht = *new game::Turn();
    ht->setTurnNumber(7);
    t.previousTurns().create(7)->handleLoadSucceeded(ht);

    TS_ASSERT_THROWS_NOTHING(t.setViewpointTurnNumber(7));
    TS_ASSERT_EQUALS(t.getViewpointTurnNumber(), 7);
    TS_ASSERT_EQUALS(t.getViewpointTurn().get(), &*ht);
}

/** Test alliance synchronisation.
    A: create a Game; configure alliances.
    E: synchronizeTeamsFromAlliances() correctly updates teams from our alliance offers */
void
TestGameGame::testAlliances()
{
    using game::alliance::Container;
    using game::alliance::Level;
    using game::alliance::Offer;

    // I am player 3
    game::Game t;
    t.setViewpointPlayer(3);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(3), 3);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(5), 5);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(6), 6);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(7), 7);

    // Add alliance levels, player 5 offers, we offer back; player 6 offers; we offer to 7
    Container& allies = t.currentTurn().alliances();
    allies.addLevel(Level("name", "id", Level::Flags_t(Level::IsOffer)));
    allies.getMutableOffer(0)->theirOffer.set(5, Offer::Yes);
    allies.getMutableOffer(0)->theirOffer.set(6, Offer::Yes);
    allies.getMutableOffer(0)->newOffer.set(5, Offer::Yes);
    allies.getMutableOffer(0)->newOffer.set(7, Offer::Yes);

    // Test
    t.synchronizeTeamsFromAlliances();
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(3), 3);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(5), 3);  // Changed!
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(6), 6);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(7), 3);  // Changed!

    // Retract our offer to 5
    allies.getMutableOffer(0)->newOffer.set(5, Offer::No);
    t.synchronizeTeamsFromAlliances();
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(3), 3);
    TS_ASSERT_EQUALS(t.teamSettings().getPlayerTeam(5), 5);  // Changed
}

/** Test alliance messages.
    A: create Game, configure alliance. Call addMessageInformation() with an alliance record.
    E: alliance settings correctly updated. */
void
TestGameGame::testMessageAlliance()
{
    using game::alliance::Container;
    using game::alliance::Level;
    using game::alliance::Offer;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    Container& allies = testee.currentTurn().alliances();
    allies.addLevel(Level("name", "id", Level::Flags_t()));

    // Add some alliance reports
    game::parser::MessageInformation info(game::parser::MessageInformation::Alliance, 0, 42);
    {
        Offer o;
        o.theirOffer.set(4, Offer::Conditional);
        o.oldOffer.set(4, Offer::Yes);
        info.addAllianceValue("id", o);
    }
    {
        Offer o;
        o.theirOffer.set(5, Offer::Yes);
        info.addAllianceValue("id", o);
    }
    {
        Offer o;
        o.theirOffer.set(6, Offer::Yes);
        info.addAllianceValue("other", o);
    }
    HostConfiguration config;
    testee.addMessageInformation(info, config, afl::base::Nothing);

    // Verify
    TS_ASSERT_EQUALS(allies.getOffer(0)->theirOffer.get(4), Offer::Conditional);
    TS_ASSERT_EQUALS(allies.getOffer(0)->oldOffer.get(4), Offer::Yes);
    TS_ASSERT_EQUALS(allies.getOffer(0)->theirOffer.get(5), Offer::Yes);
    TS_ASSERT_EQUALS(allies.getOffer(0)->theirOffer.get(6), Offer::Unknown);
}

/** Test configuration messages.
    A: create Game, configure alliance. Call addMessageInformation() with configuration information.
    E: configuration correctly updated. */
void
TestGameGame::testMessageConfig()
{
    HostConfiguration config;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::Configuration, 0, 42);
    info.addConfigurationValue("raceminingRate", "5,6,7,8");       // Array of integer
    info.addConfigurationValue("planetshavetubes", "1");           // Boolean, numeric
    info.addConfigurationValue("CrystalSinTempBehavior", "Yes");   // Boolean, name
    info.addConfigurationValue("whatever", "?");                   // unknown option
    info.addConfigurationValue("MaxColTempSlope", "?");            // Integer, bogus value
    info.addConfigurationValue("MaxPlanetaryIncome", "777");       // Integer

    TS_ASSERT_THROWS_NOTHING(testee.addMessageInformation(info, config, afl::base::Nothing));

    TS_ASSERT_EQUALS(config[HostConfiguration::RaceMiningRate](1), 5);
    TS_ASSERT_EQUALS(config[HostConfiguration::RaceMiningRate](4), 8);
    TS_ASSERT_EQUALS(config[HostConfiguration::RaceMiningRate](11), 8);
    TS_ASSERT_EQUALS(config[HostConfiguration::PlanetsHaveTubes](), 1);
    TS_ASSERT_EQUALS(config[HostConfiguration::CrystalSinTempBehavior](), 1);
    TS_ASSERT_EQUALS(config[HostConfiguration::MaxColTempSlope](), 1000);     // unchanged default
    TS_ASSERT_EQUALS(config[HostConfiguration::MaxPlanetaryIncome](1), 777);
}

/** Test message linking.
    A: create Game, add ships and planets. Call addMessageInformation() with message numbers.
    E: message numbers added to units */
void
TestGameGame::testMessageLink()
{
    HostConfiguration config;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::map::Planet* pl = testee.currentTurn().universe().planets().create(99);
    game::map::Ship* sh = testee.currentTurn().universe().ships().create(77);

    // Add planet information
    game::parser::MessageInformation i1(game::parser::MessageInformation::Planet, 99, 42);
    i1.addValue(game::parser::ms_FriendlyCode, "ppp");
    TS_ASSERT_THROWS_NOTHING(testee.addMessageInformation(i1, config, 3));

    // Add ship information
    game::parser::MessageInformation i2(game::parser::MessageInformation::Ship, 77, 42);
    i2.addValue(game::parser::ms_FriendlyCode, "sss");
    TS_ASSERT_THROWS_NOTHING(testee.addMessageInformation(i2, config, 4));

    // Verify
    TS_ASSERT_EQUALS(pl->getFriendlyCode().orElse(""), "ppp");
    TS_ASSERT_EQUALS(pl->messages().get().size(), 1U);
    TS_ASSERT_EQUALS(pl->messages().get()[0], 3U);

    TS_ASSERT_EQUALS(sh->getFriendlyCode().orElse(""), "sss");
    TS_ASSERT_EQUALS(sh->messages().get().size(), 1U);
    TS_ASSERT_EQUALS(sh->messages().get()[0], 4U);
}

