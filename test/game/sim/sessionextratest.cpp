/**
  *  \file test/game/sim/sessionextratest.cpp
  *  \brief Test for game::sim::SessionExtra
  */

#include "game/sim/sessionextra.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/alliance/container.hpp"
#include "game/alliance/phosthandler.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

/** Test basic connection of game::Session and game::sim::Session.
    A: create a game::Session and obtain its game::sim::Session.
    E: session is returned and has a GameInterface. */
AFL_TEST("game.sim.SessionExtra:basics", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session gs(tx, fs);
    afl::base::Ref<game::sim::Session> ss = game::sim::getSimulatorSession(gs);

    // Session does have a GameInterface, but that does not have a game
    a.checkNonNull("01. getGameInterface", ss->getGameInterface());
    a.check("02. hasGame", !ss->getGameInterface()->hasGame());
    a.checkEqual("03. getMaxShipId", ss->getGameInterface()->getMaxShipId(), 0);
    a.checkEqual("04. getMaxPlanetId", ss->getGameInterface()->getMaxPlanetId(), 0);
}

/** Test alliance handling.
    A: create a game::Session. Add a game with alliances. Obtain game::sim::Session.
    E: session is returned and can generate appropriate settings. */
AFL_TEST("game.sim.SessionExtra:alliances", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session gs(tx, fs);
    gs.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,2,0))).asPtr());

    // Set up a game. For simplicity, re-use PHost infrastructure.
    afl::base::Ptr<game::Game> g = new game::Game();
    game::Turn& t = g->currentTurn();
    gs.setGame(g);
    g->setViewpointPlayer(3);

    // - alliance 3<->4
    g->teamSettings().setPlayerTeam(4, 3);

    // - alliance 3->7
    game::v3::CommandContainer& cc = game::v3::CommandExtra::create(t).create(3);
    cc.addCommand(game::v3::Command::AddDropAlly, 7, "add");
    cc.addCommand(game::v3::Command::ConfigAlly, 7, "+c");

    // - NOT an alliance 3->8 (not combat level)
    cc.addCommand(game::v3::Command::AddDropAlly, 8, "add");
    cc.addCommand(game::v3::Command::ConfigAlly, 8, "+p");

    // - enemy 3->9
    cc.addCommand(game::v3::Command::Enemies, 9, "add");

    // - NOT an enemy 3->10
    cc.addCommand(game::v3::Command::Enemies, 10, "drop");

    // Alliance handler
    t.alliances().addNewHandler(new game::alliance::PHostHandler(t, *gs.getRoot(), 3), tx);
    t.alliances().postprocess();

    // Simulator session
    afl::base::Ref<game::sim::Session> ss = game::sim::getSimulatorSession(gs);
    a.checkNonNull("01. getGameInterface", ss->getGameInterface());
    a.check("02. hasGame", ss->getGameInterface()->hasGame());

    // Verify relations
    game::PlayerBitMatrix aa, ee;
    ss->getGameInterface()->getPlayerRelations(aa, ee);
    a.check("11. alliance", aa.get(3, 4));
    a.check("12. alliance", aa.get(4, 3));

    a.check("21. alliance", aa.get(3, 7));
    a.check("22. alliance", !aa.get(7, 3));

    a.check("31. alliance", !aa.get(3, 8));
    a.check("32. alliance", !aa.get(8, 3));

    a.check("41. enemy", ee.get(3, 9));
    a.check("42. enemy", !ee.get(9, 3));

    a.check("51. enemy", !ee.get(3, 4));
    a.check("52. enemy", !ee.get(4, 3));

    a.check("61. enemy", !ee.get(3, 10));
    a.check("62. enemy", !ee.get(10, 3));
}

/** Test initSimulatorSession().
    A: create session with a specific host version. Call initSimulatorSession().
    E: session configuration must use matching host version */
AFL_TEST("game.sim.SessionExtra:initSimulatorSession", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session gs(tx, fs);
    gs.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,48))).asPtr());
    gs.setGame(new game::Game());

    // Set some defaults
    afl::base::Ref<game::sim::Session> ss = game::sim::getSimulatorSession(gs);
    game::config::HostConfiguration config;
    ss->configuration().setMode(game::sim::Configuration::VcrPHost4, 0, config);

    // Load game defaults
    game::sim::initSimulatorSession(gs);

    // Verify
    a.checkEqual("01. getMode", ss->configuration().getMode(), game::sim::Configuration::VcrHost);
}
