/**
  *  \file u/t_game_interface_ufocontext.cpp
  *  \brief Test for game::interface::UfoContext
  */

#include "game/interface/ufocontext.hpp"

#include "t_game_interface.hpp"
#include "u/helper/contextverifier.hpp"
#include "game/session.hpp"
#include "game/game.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"

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
    ufo->setSpeed(2);
    ufo->setHeading(135);
    ufo->setPlanetRange(200);
    ufo->setShipRange(150);
    ufo->setInfo1("USS Rosswell");
    ufo->setInfo2("New Mexico");
    ufo->postprocess(42);

    TS_ASSERT_EQUALS(turn.universe().ufos().getObjectByIndex(1), ufo);

    // Create a context
    game::interface::UfoContext testee(1, turn, session);
    verifyTypes(testee);

    // Verify some values
    verifyInteger(testee, "ID", 51);
    verifyInteger(testee, "HEADING$", 135);
    verifyString(testee, "HEADING", "SE");
    verifyString(testee, "INFO1", "USS Rosswell");
    verifyInteger(testee, "COLOR.EGA", 2);
    verifyInteger(testee, "COLOR", 12);
}

