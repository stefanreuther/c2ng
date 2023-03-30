/**
  *  \file u/t_game_interface_ufocontext.cpp
  *  \brief Test for game::interface::UfoContext
  */

#include "game/interface/ufocontext.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/session.hpp"
#include "interpreter/test/contextverifier.hpp"

/** Verify types. */
void
TestGameInterfaceUfoContext::testTypes()
{
    // Create a session with a turn
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    game::Turn& turn = session.getGame()->currentTurn();

    // Add an Ufo
    game::map::Ufo* ufo = turn.universe().ufos().addUfo(51, 1, 2);
    TS_ASSERT(ufo != 0);
    ufo->setWarpFactor(2);
    ufo->setHeading(135);
    ufo->setPlanetRange(200);
    ufo->setShipRange(150);
    ufo->setInfo1("USS Rosswell");
    ufo->setInfo2("New Mexico");
    ufo->postprocess(42, session.getGame()->mapConfiguration());

    TS_ASSERT_EQUALS(turn.universe().ufos().getObjectByIndex(1), ufo);

    // Create a context
    game::interface::UfoContext testee(1, turn, session);
    interpreter::test::ContextVerifier v(testee, "testTypes");
    v.verifyTypes();
    v.verifyBasics();
    v.verifyNotSerializable();

    // Verify some values
    v.verifyInteger("ID", 51);
    v.verifyInteger("HEADING$", 135);
    v.verifyString("HEADING", "SE");
    v.verifyString("INFO1", "USS Rosswell");
    v.verifyInteger("COLOR.EGA", 2);
    v.verifyInteger("COLOR", 12);
}

