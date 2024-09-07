/**
  *  \file test/server/play/shipxypackertest.cpp
  *  \brief Test for server::play::ShipXYPacker
  */

#include "server/play/shipxypacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using game::Game;
using game::PlayerSet_t;
using game::map::Ship;
using game::map::ShipData;
using game::parser::MessageInformation;

namespace {
    const int TURN_NR = 10;
    const int VIEWPOINT_PLAYER = 1;

    void addScannedShip(Game& g, int id, int x, int y, int owner, int mass, String_t name)
    {
        Ship& sh = *g.currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(x, y), owner, mass, PlayerSet_t(VIEWPOINT_PLAYER));
        sh.setName(name);
        sh.internalCheck(PlayerSet_t(VIEWPOINT_PLAYER), TURN_NR);
    }

    void addPlayedShip(Game& g, int id, int x, int y, int owner, String_t name)
    {
        Ship& sh = *g.currentTurn().universe().ships().create(id);
        ShipData sd;
        sd.x = x;
        sd.y = y;
        sd.owner = owner;
        sd.name = name;
        sh.addCurrentShipData(sd, PlayerSet_t(VIEWPOINT_PLAYER));
        sh.internalCheck(PlayerSet_t(VIEWPOINT_PLAYER), TURN_NR);
        sh.setPlayability(game::map::Object::Playable);
    }

    void addGuessedShip(Game& g, int id, int x, int y, int owner, int mass, String_t name)
    {
        Ship& sh = *g.currentTurn().universe().ships().create(id);
        MessageInformation info(MessageInformation::Ship, id, TURN_NR);
        info.addValue(game::parser::mi_X, x);
        info.addValue(game::parser::mi_Y, y);
        info.addValue(game::parser::mi_Mass, mass);
        info.addValue(game::parser::mi_Owner, owner);
        info.addValue(game::parser::ms_Name, name);
        sh.addMessageInformation(info, PlayerSet_t());
        sh.internalCheck(PlayerSet_t(1), TURN_NR);
    }
}

AFL_TEST("server.play.ShipXYPacker", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // We need a root/shiplist, but it can be empty
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());

    // Game containing the ships we show
    afl::base::Ptr<Game> g = new Game();
    addScannedShip(*g, 1, 1000, 1200, 4, 500, "One");
    addPlayedShip(*g, 10, 2000, 1300, 1,      "Two");
    addGuessedShip(*g, 5, 3000, 1400, 4, 200, "Guess");

    session.setGame(g);

    // Test it!
    server::play::ShipXYPacker testee(session);
    a.checkEqual("01. name", testee.getName(), "shipxy");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("11. length", ap.getArraySize(), 11U);

    a.checkEqual("21. x",       ap[1]("X").toInteger(),       1000);
    a.checkEqual("22. y",       ap[1]("Y").toInteger(),       1200);
    a.checkEqual("23. mass",    ap[1]("MASS").toInteger(),    500);
    a.checkEqual("24. owner",   ap[1]("OWNER").toInteger(),   4);
    a.checkEqual("25. name",    ap[1]("NAME").toString(),     "One");
    a.checkEqual("26. played",  ap[1]("PLAYED").toInteger(),  0);
    a.checkEqual("27. guessed", ap[1]("GUESSED").toInteger(), 0);

    a.checkEqual("31. x",       ap[10]("X").toInteger(),       2000);
    a.checkEqual("32. y",       ap[10]("Y").toInteger(),       1300);
    // To set the mass, we would have to define a hull.
    // a.checkEqual("33. mass",    ap[10]("MASS").toInteger(),    600);
    a.checkEqual("34. owner",   ap[10]("OWNER").toInteger(),   1);
    a.checkEqual("35. name",    ap[10]("NAME").toString(),     "Two");
    a.checkEqual("36. played",  ap[10]("PLAYED").toInteger(),  true);
    a.checkEqual("37. guessed", ap[10]("GUESSED").toInteger(), 0);

    a.checkEqual("31. x",       ap[5]("X").toInteger(),       3000);
    a.checkEqual("32. y",       ap[5]("Y").toInteger(),       1400);
    a.checkEqual("33. mass",    ap[5]("MASS").toInteger(),    200);
    a.checkEqual("34. owner",   ap[5]("OWNER").toInteger(),   4);
    a.checkEqual("35. name",    ap[5]("NAME").toString(),     "Guess");
    a.checkEqual("36. played",  ap[5]("PLAYED").toInteger(),  0);
    a.checkEqual("37. guessed", ap[5]("GUESSED").toInteger(), 1);
}
