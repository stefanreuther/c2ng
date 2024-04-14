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
#include "game/map/explosion.hpp"
#include "game/map/ionstorm.hpp"
#include "game/map/ufo.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/turn.hpp"
#include "util/atomtable.hpp"

using game::HostVersion;
using game::config::HostConfiguration;
using game::score::TurnScore;
using game::score::TurnScoreList;
using util::AtomTable;

namespace {
    struct MessageEnvironment {
        HostConfiguration config;
        HostVersion host;
        AtomTable atomTable;
        afl::sys::Log log;
        afl::string::NullTranslator tx;
    };
}


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
    a.checkEqual("08. expressionLists",      &t.expressionLists(), &ct.expressionLists());

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

/** Test viewpoint turn access, failure
    A: create a game; add history turn
    E: setViewpointTurnNumber() with invalid turn numbers is ignored. */
AFL_TEST("game.Game:viewpoint-turn:failure", a)
{
    // Set turn number. Must immediately be reflected as getViewpointTurnNumber().
    game::Game t;
    t.currentTurn().setTurnNumber(12);
    afl::base::Ref<game::Turn> ht = *new game::Turn();
    ht->setTurnNumber(7);
    t.previousTurns().create(7)->handleLoadSucceeded(ht);

    // Success cases
    t.setViewpointTurnNumber(12);
    a.checkEqual("01", t.viewpointTurn().getTurnNumber(), 12);

    t.setViewpointTurnNumber(7);
    a.checkEqual("02", t.viewpointTurn().getTurnNumber(), 7);

    // Error cases
    t.setViewpointTurnNumber(1);
    a.checkEqual("11", t.viewpointTurn().getTurnNumber(), 7);

    t.setViewpointTurnNumber(13);
    a.checkEqual("12", t.viewpointTurn().getTurnNumber(), 7);
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
AFL_TEST("game.Game:addMessageInformation:Alliance", a)
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

    MessageEnvironment env;
    testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log);

    // Verify
    a.checkEqual("01", allies.getOffer(0)->theirOffer.get(4), Offer::Conditional);
    a.checkEqual("02", allies.getOffer(0)->oldOffer.get(4), Offer::Yes);
    a.checkEqual("03", allies.getOffer(0)->theirOffer.get(5), Offer::Yes);
    a.checkEqual("04", allies.getOffer(0)->theirOffer.get(6), Offer::Unknown);
}

/** Test configuration messages.
    A: create Game. Call addMessageInformation() with configuration information.
    E: configuration correctly updated. */
AFL_TEST("game.Game:addMessageInformation:Configuration", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::Configuration, 0, 42);
    info.addConfigurationValue("raceminingRate", "5,6,7,8");       // Array of integer
    info.addConfigurationValue("planetshavetubes", "1");           // Boolean, numeric
    info.addConfigurationValue("CrystalSinTempBehavior", "Yes");   // Boolean, name
    info.addConfigurationValue("whatever", "?");                   // unknown option
    info.addConfigurationValue("MaxColTempSlope", "?");            // Integer, bogus value
    info.addConfigurationValue("MaxPlanetaryIncome", "777");       // Integer

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    a.checkEqual("11. config", env.config[HostConfiguration::RaceMiningRate](1), 5);
    a.checkEqual("12. config", env.config[HostConfiguration::RaceMiningRate](4), 8);
    a.checkEqual("13. config", env.config[HostConfiguration::RaceMiningRate](11), 8);
    a.checkEqual("14. config", env.config[HostConfiguration::PlanetsHaveTubes](), 1);
    a.checkEqual("15. config", env.config[HostConfiguration::CrystalSinTempBehavior](), 1);
    a.checkEqual("16. config", env.config[HostConfiguration::MaxColTempSlope](), 1000);     // unchanged default
    a.checkEqual("17. config", env.config[HostConfiguration::MaxPlanetaryIncome](1), 777);
}

/** Test Explosion info.
    A: create Game. Call addMessageInformation() with Explosion information.
    E: Ufo representing explosion added. */
AFL_TEST("game.Game:addMessageInformation:Explosion", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::Explosion, 0, 42);
    info.addValue(game::parser::mi_X, 1900);
    info.addValue(game::parser::mi_Y, 1700);
    info.addValue(game::parser::ms_Name, "USS Bang");

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    game::map::Explosion* ex = testee.currentTurn().universe().explosions().getObjectByIndex(1);
    a.checkNonNull("11. ex", ex);
    a.checkEqual("12. pos",  ex->getPosition().orElse(game::map::Point()), game::map::Point(1900, 1700));
    a.checkEqual("13. name", ex->getShipName(), "USS Bang");
}

/** Test ion storm info.
    A: create Game. Call addMessageInformation() with current ion storm.
    E: ion storm updated. */
AFL_TEST("game.Game:addMessageInformation:IonStorm", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);
    testee.currentTurn().universe().ionStorms().create(5);

    game::parser::MessageInformation info(game::parser::MessageInformation::IonStorm, 5, 42);
    info.addValue(game::parser::mi_X, 2400);
    info.addValue(game::parser::mi_Y, 1800);
    info.addValue(game::parser::mi_Radius, 30);
    info.addValue(game::parser::mi_IonVoltage, 50);
    info.addValue(game::parser::mi_Heading, 90);
    info.addValue(game::parser::mi_WarpFactor, 3);

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    game::map::IonStorm* st = testee.currentTurn().universe().ionStorms().get(5);
    a.checkNonNull("11. storm", st);
    a.checkEqual("12. pos",     st->getPosition().orElse(game::map::Point()), game::map::Point(2400, 1800));
    a.checkEqual("13. voltage", st->getVoltage().orElse(0), 50);
}

/** Test ion storm info, outdated.
    A: create Game. Call addMessageInformation() with old ion storm.
    E: ion storm not added. */
AFL_TEST("game.Game:addMessageInformation:IonStorm:old", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);
    testee.currentTurn().universe().ionStorms().create(5);

    game::parser::MessageInformation info(game::parser::MessageInformation::IonStorm, 5, 41);
    info.addValue(game::parser::mi_X, 2400);
    info.addValue(game::parser::mi_Y, 1800);
    info.addValue(game::parser::mi_Radius, 30);
    info.addValue(game::parser::mi_IonVoltage, 50);
    info.addValue(game::parser::mi_Heading, 90);
    info.addValue(game::parser::mi_WarpFactor, 3);

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    game::map::IonStorm* st = testee.currentTurn().universe().ionStorms().get(5);
    a.checkNonNull("11. storm", st);
    a.checkEqual("12. pos",     st->getPosition().isValid(), false);
}

/** Test PlayerScore info.
    A: create Game. Call addMessageInformation() with current score.
    E: score data added. */
AFL_TEST("game.Game:addMessageInformation:PlayerScore", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);
    testee.currentTurn().setTimestamp(game::Timestamp(2015, 12, 1, 5, 30, 42));

    game::parser::MessageInformation info(game::parser::MessageInformation::PlayerScore, 99, 42);
    info.addValue(game::parser::ms_Name, "The Score");
    info.addScoreValue(2, 50);
    info.addScoreValue(7, 90);

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    // Verify score description
    const TurnScoreList& ts = testee.scores();
    const TurnScoreList::Description* pDesc = ts.getDescription(99);
    afl::base::Optional<TurnScoreList::Slot_t> optIndex = ts.getSlot(99);
    a.checkNonNull("11. desc", pDesc);
    a.checkEqual  ("12. name", pDesc->name, "The Score");
    a.check       ("13. index", optIndex.isValid());

    // Verify score content
    const TurnScore* pTurn = ts.getTurn(42);
    a.checkNonNull("21. turn", pTurn);
    a.checkEqual  ("22. player 2", pTurn->get(*optIndex.get(), 2).orElse(0), 50);
    a.checkEqual  ("23. player 7", pTurn->get(*optIndex.get(), 7).orElse(0), 90);
    a.check       ("24. player 1", !pTurn->get(*optIndex.get(), 1).isValid());
    a.checkEqual  ("25. time", pTurn->getTimestamp().getTimestampAsString(), "12-01-201505:30:42");
}

/** Test PlayerScore info, old turn.
    A: create Game. Call addMessageInformation() with old score.
    E: score data added. */
AFL_TEST("game.Game:addMessageInformation:PlayerScore:old", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);
    testee.currentTurn().setTimestamp(game::Timestamp(2015, 12, 1, 5, 30, 42));
    testee.scores().addTurn(12, game::Timestamp(2014, 11, 7, 6, 40, 23));

    game::parser::MessageInformation info(game::parser::MessageInformation::PlayerScore, 99, 12);
    info.addValue(game::parser::ms_Name, "The Score");
    info.addScoreValue(2, 50);
    info.addScoreValue(7, 90);

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    // Verify score description
    const TurnScoreList& ts = testee.scores();
    const TurnScoreList::Description* pDesc = ts.getDescription(99);
    afl::base::Optional<TurnScoreList::Slot_t> optIndex = ts.getSlot(99);
    a.checkNonNull("11. desc", pDesc);
    a.checkEqual  ("12. name", pDesc->name, "The Score");
    a.check       ("13. index", optIndex.isValid());

    // Verify score content
    const TurnScore* pTurn = ts.getTurn(12);
    a.checkNonNull("21. turn", pTurn);
    a.checkEqual  ("22. player 2", pTurn->get(*optIndex.get(), 2).orElse(0), 50);
    a.checkEqual  ("23. player 7", pTurn->get(*optIndex.get(), 7).orElse(0), 90);
    a.check       ("24. player 1", !pTurn->get(*optIndex.get(), 1).isValid());
    a.checkEqual  ("25. time", pTurn->getTimestamp().getTimestampAsString(), "11-07-201406:40:23");
}

/** Test Ufo info.
    A: create Game. Call addMessageInformation() with Ufo information.
    E: Ufo added. */
AFL_TEST("game.Game:addMessageInformation:Ufo", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::Ufo, 4000, 42);
    info.addValue(game::parser::mi_X, 2400);
    info.addValue(game::parser::mi_Y, 1800);
    info.addValue(game::parser::mi_Color, 1);
    info.addValue(game::parser::mi_Type, 55);
    info.addValue(game::parser::mi_Radius, 30);
    info.addValue(game::parser::mi_Heading, 90);
    info.addValue(game::parser::mi_WarpFactor, 3);
    info.addValue(game::parser::ms_Name, "Martian");

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    game::map::Ufo* ufo = testee.currentTurn().universe().ufos().getUfoByIndex(1);
    a.checkNonNull("11. ufo", ufo);
    a.checkEqual("12. pos",  ufo->getPosition().orElse(game::map::Point()), game::map::Point(2400, 1800));
    a.checkEqual("13. name", ufo->getName(), "Martian");
}

/** Test Wormhole info.
    A: create Game. Call addMessageInformation() with Wormhole information; call postprocess().
    E: Ufo representing wormhole added. */
AFL_TEST("game.Game:addMessageInformation:Wormhole", a)
{
    MessageEnvironment env;

    game::Game testee;
    testee.currentTurn().setTurnNumber(42);

    game::parser::MessageInformation info(game::parser::MessageInformation::Wormhole, 4000, 42);
    info.addValue(game::parser::mi_X, 1900);
    info.addValue(game::parser::mi_Y, 1700);
    info.addValue(game::parser::mi_Mass, 7000);

    AFL_CHECK_SUCCEEDS(a("01. addMessageInformation"), testee.addMessageInformation(info, env.config, env.host, env.atomTable, afl::base::Nothing, true, env.tx, env.log));

    game::map::Configuration mapConfig;
    testee.currentTurn().universe().ufos().postprocess(42, mapConfig, env.config, env.tx, env.log);

    game::map::Ufo* ufo = testee.currentTurn().universe().ufos().getUfoByIndex(1);
    a.checkNonNull("11. ufo", ufo);
    a.checkEqual("12. pos",  ufo->getPosition().orElse(game::map::Point()), game::map::Point(1900, 1700));
    a.checkEqual("13. id", ufo->getRealId(), 4000);
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

/** Test isGameObject(), planet. */
AFL_TEST("game.Game:isGameObject:planet", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    game::map::Configuration mapConfig;
    game::spec::HullVector_t hulls;
    game::Game testee;
    game::map::Planet& pl = *testee.currentTurn().universe().planets().create(20);
    pl.setPosition(game::map::Point(1000, 1000));
    pl.internalCheck(mapConfig, game::PlayerSet_t(), 10, tx, log);

    // Failure - mismatching Id
    {
        game::vcr::Object p10;
        p10.setIsPlanet(true);
        p10.setId(10);
        a.checkEqual("01", testee.isGameObject(p10, hulls), false);
    }

    // Success - matching Id
    {
        game::vcr::Object p20;
        p20.setIsPlanet(true);
        p20.setId(20);
        a.checkEqual("02", testee.isGameObject(p20, hulls), true);
    }
}

/** Test isGameObject(), ship. */
AFL_TEST("game.Game:isGameObject:ship", a)
{
    game::spec::HullVector_t hulls;
    game::spec::Hull& h = *hulls.create(55);
    h.setMass(50);
    h.setMaxBeams(5);

    game::Game testee;
    game::map::Ship& sh = *testee.currentTurn().universe().ships().create(20);
    sh.addShipXYData(game::map::Point(1000, 1000), 5, 100, game::PlayerSet_t(1));
    sh.setHull(55);
    sh.internalCheck(game::PlayerSet_t(1), 10);

    // Failure - mismatching Id
    {
        game::vcr::Object s10;
        s10.setIsPlanet(false);
        s10.setId(10);
        a.checkEqual("01", testee.isGameObject(s10, hulls), false);
    }

    // Success
    {
        game::vcr::Object s20;
        s20.setIsPlanet(false);
        s20.setId(20);
        s20.setMass(50);
        a.checkEqual("02", testee.isGameObject(s20, hulls), true);
    }

    // Failure - mismatch
    {
        game::vcr::Object s20;
        s20.setIsPlanet(false);
        s20.setId(20);
        s20.setNumBeams(20); // too many
        a.checkEqual("03", testee.isGameObject(s20, hulls), false);
    }
}
