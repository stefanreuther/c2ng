/**
  *  \file test/game/gametest.cpp
  *  \brief Test for game::Game
  */

#include "game/game.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/alliance/container.hpp"
#include "game/alliance/level.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/turn.hpp"
#include "util/atomtable.hpp"

using game::HostVersion;
using game::config::HostConfiguration;
using util::AtomTable;

/** Test smart pointers.
    A: pass Game object from smart through dumb pointer.
    E: code executes correctly, no reference-count confusion */
AFL_TEST_NOARG("game.Game:ref")
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
AFL_TEST("game.Game:init", a)
{
    game::Game t;
    a.checkEqual("01. getViewpointPlayer",     t.getViewpointPlayer(), 0);
    a.checkEqual("02. getViewpointTurnNumber", t.getViewpointTurnNumber(), 0);
    a.checkEqual("03. viewpointTurn",          &t.viewpointTurn(), &t.currentTurn());
}

/** Test subobjects.
    A: create Game. Access sub-objects through mutable and const path
    E: both paths produce the same object */
AFL_TEST("game.Game:subobjects", a)
{
    game::Game t;
    const game::Game& ct = t;

    a.checkEqual("01. currentTurn",          &t.currentTurn(), &ct.currentTurn());
    a.checkEqual("02. previousTurns",        &t.previousTurns(), &ct.previousTurns());
    a.checkEqual("03. planetScores",         &t.planetScores(), &ct.planetScores());
    a.checkEqual("04. shipScores",           &t.shipScores(), &ct.shipScores());
    a.checkEqual("05. teamSettings",         &t.teamSettings(), &ct.teamSettings());
    a.checkEqual("06. scores",               &t.scores(), &ct.scores());
    a.checkEqual("07. messageConfiguration", &t.messageConfiguration(), &ct.messageConfiguration());

    a.checkDifferent("11. planetScores", &t.planetScores(), &t.shipScores());
}

/** Test viewpoint turn access.
    A: create a game; add history turn
    E: current and history turn correctly settable as viewpoint turn */
AFL_TEST("game.Game:viewpoint-turn", a)
{
    // Set turn number. Must immediately be reflected as getViewpointTurnNumber().
    game::Game t;
    t.currentTurn().setTurnNumber(12);
    a.checkEqual("01. getViewpointTurnNumber", t.getViewpointTurnNumber(), 12);
    a.checkEqual("02. viewpointTurn", &t.viewpointTurn(), &t.currentTurn());

    // Add a history turn. Must be retrievable.
    afl::base::Ref<game::Turn> ht = *new game::Turn();
    ht->setTurnNumber(7);
    t.previousTurns().create(7)->handleLoadSucceeded(ht);

    AFL_CHECK_SUCCEEDS(a("11. setViewpointTurnNumber"), t.setViewpointTurnNumber(7));
    a.checkEqual("12. getViewpointTurnNumber", t.getViewpointTurnNumber(), 7);
    a.checkEqual("13. viewpointTurn", &t.viewpointTurn(), &*ht);
}

/** Test alliance synchronisation.
    A: create a Game; configure alliances.
    E: synchronizeTeamsFromAlliances() correctly updates teams from our alliance offers */
AFL_TEST("game.Game:synchronizeTeamsFromAlliances", a)
{
    using game::alliance::Container;
    using game::alliance::Level;
    using game::alliance::Offer;

    // I am player 3
    game::Game t;
    t.setViewpointPlayer(3);
    a.checkEqual("01", t.teamSettings().getPlayerTeam(3), 3);
    a.checkEqual("02", t.teamSettings().getPlayerTeam(5), 5);
    a.checkEqual("03", t.teamSettings().getPlayerTeam(6), 6);
    a.checkEqual("04", t.teamSettings().getPlayerTeam(7), 7);

    // Add alliance levels, player 5 offers, we offer back; player 6 offers; we offer to 7
    Container& allies = t.currentTurn().alliances();
    allies.addLevel(Level("name", "id", Level::Flags_t(Level::IsOffer)));
    allies.getMutableOffer(0)->theirOffer.set(5, Offer::Yes);
    allies.getMutableOffer(0)->theirOffer.set(6, Offer::Yes);
    allies.getMutableOffer(0)->newOffer.set(5, Offer::Yes);
    allies.getMutableOffer(0)->newOffer.set(7, Offer::Yes);

    // Test
    t.synchronizeTeamsFromAlliances();
    a.checkEqual("11", t.teamSettings().getPlayerTeam(3), 3);
    a.checkEqual("12", t.teamSettings().getPlayerTeam(5), 3);  // Changed!
    a.checkEqual("13", t.teamSettings().getPlayerTeam(6), 6);
    a.checkEqual("14", t.teamSettings().getPlayerTeam(7), 3);  // Changed!

    // Retract our offer to 5
    allies.getMutableOffer(0)->newOffer.set(5, Offer::No);
    t.synchronizeTeamsFromAlliances();
    a.checkEqual("21", t.teamSettings().getPlayerTeam(3), 3);
    a.checkEqual("22", t.teamSettings().getPlayerTeam(5), 5);  // Changed
}

/** Test alliance messages.
    A: create Game, configure alliance. Call addMessageInformation() with an alliance record.
    E: alliance settings correctly updated. */
AFL_TEST("game.Game:addMessageInformation:alliance", a)
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
    HostVersion host;
    AtomTable atomTable;
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    testee.addMessageInformation(info, config, host, atomTable, afl::base::Nothing, true, tx, log);

    // Verify
    a.checkEqual("01", allies.getOffer(0)->theirOffer.get(4), Offer::Conditional);
    a.checkEqual("02", allies.getOffer(0)->oldOffer.get(4), Offer::Yes);
    a.checkEqual("03", allies.getOffer(0)->theirOffer.get(5), Offer::Yes);
    a.checkEqual("04", allies.getOffer(0)->theirOffer.get(6), Offer::Unknown);
}

/** Test configuration messages.
    A: create Game, configure alliance. Call addMessageInformation() with configuration information.
    E: configuration correctly updated. */
AFL_TEST("game.Game:addMessageInformation:config", a)
{
    HostConfiguration config;
    HostVersion host;
    AtomTable atomTable;
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::Configuration, 0, 42);
    info.addConfigurationValue("raceminingRate", "5,6,7,8");       // Array of integer
    info.addConfigurationValue("planetshavetubes", "1");           // Boolean, numeric
    info.addConfigurationValue("CrystalSinTempBehavior", "Yes");   // Boolean, name
    info.addConfigurationValue("whatever", "?");                   // unknown option
    info.addConfigurationValue("MaxColTempSlope", "?");            // Integer, bogus value
    info.addConfigurationValue("MaxPlanetaryIncome", "777");       // Integer

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, config, host, atomTable, afl::base::Nothing, true, tx, log));

    a.checkEqual("11. config", config[HostConfiguration::RaceMiningRate](1), 5);
    a.checkEqual("12. config", config[HostConfiguration::RaceMiningRate](4), 8);
    a.checkEqual("13. config", config[HostConfiguration::RaceMiningRate](11), 8);
    a.checkEqual("14. config", config[HostConfiguration::PlanetsHaveTubes](), 1);
    a.checkEqual("15. config", config[HostConfiguration::CrystalSinTempBehavior](), 1);
    a.checkEqual("16. config", config[HostConfiguration::MaxColTempSlope](), 1000);     // unchanged default
    a.checkEqual("17. config", config[HostConfiguration::MaxPlanetaryIncome](1), 777);
}

/** Test message linking.
    A: create Game, add ships and planets. Call addMessageInformation() with message numbers.
    E: message numbers added to units */
AFL_TEST("game.Game:message-linking", a)
{
    HostConfiguration config;
    HostVersion host;
    AtomTable atomTable;
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::map::Planet* pl = testee.currentTurn().universe().planets().create(99);
    game::map::Ship* sh = testee.currentTurn().universe().ships().create(77);

    // Add planet information
    game::parser::MessageInformation i1(game::parser::MessageInformation::Planet, 99, 42);
    i1.addValue(game::parser::ms_FriendlyCode, "ppp");
    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(i1, config, host, atomTable, 3, true, tx, log));

    // Add ship information
    game::parser::MessageInformation i2(game::parser::MessageInformation::Ship, 77, 42);
    i2.addValue(game::parser::ms_FriendlyCode, "sss");
    AFL_CHECK_SUCCEEDS(a("11. addMessageInformation"), testee.addMessageInformation(i2, config, host, atomTable, 4, true, tx, log));

    // Verify
    a.checkEqual("21. getFriendlyCode", pl->getFriendlyCode().orElse(""), "ppp");
    a.checkEqual("22. messages", pl->messages().get().size(), 1U);
    a.checkEqual("23. messages", pl->messages().get()[0], 3U);

    a.checkEqual("31. getFriendlyCode", sh->getFriendlyCode().orElse(""), "sss");
    a.checkEqual("32. messages", sh->messages().get().size(), 1U);
    a.checkEqual("33. messages", sh->messages().get()[0], 4U);
}

/** Test message containing drawing.
    A: create Game. Call addMessageInformation() with a drawing definition.
    E: drawing exists in currentTurn() */
AFL_TEST("game.Game:message:drawing", a)
{
    HostConfiguration config;
    HostVersion host;
    AtomTable atomTable;
    afl::sys::Log log;
    afl::string::NullTranslator tx;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::MarkerDrawing, 0, 42);
    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 3000);
    info.addValue(game::parser::mi_DrawingShape, 5);
    info.addValue(game::parser::ms_DrawingComment, "hi");
    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, config, host, atomTable, afl::base::Nothing, true, tx, log));

    // Verify
    game::map::DrawingContainer& dc = testee.currentTurn().universe().drawings();
    a.check("11. not empty", dc.begin() != dc.end());
    a.checkEqual("12. X",             (*dc.begin())->getPos().getX(), 2000);
    a.checkEqual("13. Y",             (*dc.begin())->getPos().getY(), 3000);
    a.checkEqual("14. getType",       (*dc.begin())->getType(), game::map::Drawing::MarkerDrawing);
    a.checkEqual("15. getMarkerKind", (*dc.begin())->getMarkerKind(), 5);
    a.checkEqual("16. getComment",    (*dc.begin())->getComment(), "hi");
}
