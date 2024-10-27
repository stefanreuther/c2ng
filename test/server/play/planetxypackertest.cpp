/**
  *  \file test/server/play/planetxypackertest.cpp
  *  \brief Test for server::play::PlanetXYPacker
  */

#include <stdexcept>
#include "server/play/planetxypacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using game::Game;
using game::map::Planet;

AFL_TEST("server.play.PlanetXYPacker", a)
{
    const int ID1 = 42;
    const int ID2 = 69;
    const int PLAYER = 7;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Empty root
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr();
    session.setRoot(r);

    // Game
    afl::base::Ptr<Game> g = new Game();
    session.setGame(g);

    // Played planet
    game::map::PlanetData pd;
    pd.owner = PLAYER;

    Planet& pl1 = *g->currentTurn().universe().planets().create(ID1);
    pl1.setPosition(game::map::Point(1030, 2700));
    pl1.setName("Meatball");
    pl1.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
    pl1.setPlayability(game::map::Object::Playable);
    pl1.internalCheck(g->mapConfiguration(), game::PlayerSet_t(PLAYER), 10, tx, session.log());

    // Other planet
    Planet& pl2 = *g->currentTurn().universe().planets().create(ID2);
    pl2.setPosition(game::map::Point(1250, 1800));
    pl2.setName("Baseball");
    pl2.internalCheck(g->mapConfiguration(), game::PlayerSet_t(PLAYER), 10, tx, session.log());

    // Test it!
    server::play::PlanetXYPacker testee(session);
    a.checkEqual("01. name", testee.getName(), "planetxy");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("11", ap[ID1]("X").toInteger(), 1030);
    a.checkEqual("12", ap[ID1]("Y").toInteger(), 2700);
    a.checkEqual("13", ap[ID1]("NAME").toString(), "Meatball");
    a.checkEqual("14", ap[ID1]("OWNER").toInteger(), PLAYER);
    a.checkEqual("15", ap[ID1]("PLAYED").toInteger(), 1);
    a.checkEqual("16", ap[ID1]("BASE").toInteger(), 0);

    a.checkEqual("21", ap[ID2]("X").toInteger(), 1250);
    a.checkEqual("22", ap[ID2]("Y").toInteger(), 1800);
    a.checkEqual("23", ap[ID2]("NAME").toString(), "Baseball");
    a.checkEqual("24", ap[ID2]("OWNER").toInteger(), 0);
    a.checkEqual("25", ap[ID2]("PLAYED").toInteger(), 0);
    a.checkEqual("26", ap[ID2]("BASE").toInteger(), 0);
}

AFL_TEST("server.play.PlanetXYPacker:error:empty", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    AFL_CHECK_THROWS(a, server::play::PlanetXYPacker(session).buildValue(), std::exception);
}
