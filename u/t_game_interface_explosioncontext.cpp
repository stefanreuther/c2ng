/**
  *  \file u/t_game_interface_explosioncontext.cpp
  *  \brief Test for game::interface::ExplosionContext
  */

#include "game/interface/explosioncontext.hpp"

#include "t_game_interface.hpp"
#include "game/session.hpp"
#include "game/game.hpp"
#include "u/helper/contextverifier.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/io/nullfilesystem.hpp"

void
TestGameInterfaceExplosionContext::testIt()
{
    // Infrastructure
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());

    // Add an explosion
    game::map::Explosion expl(1, game::map::Point(1000, 1020));
    expl.setShipName("Excalibur");
    expl.setShipId(23);
    session.getGame()->currentTurn().universe().explosions().add(expl);

    // Test it
    game::interface::ExplosionContext testee(1, session, session.getGame()->currentTurn());
    verifyTypes(testee);

    // Verify some values
    verifyInteger(testee, "ID", 1);
    verifyInteger(testee, "ID.SHIP", 23);
    verifyInteger(testee, "LOC.X", 1000);
    verifyInteger(testee, "LOC.Y", 1020);
    verifyString(testee, "TYPE", "Explosion");
    verifyString(testee, "TYPE.SHORT", "E");
    verifyString(testee, "NAME.SHIP", "Excalibur");
    verifyString(testee, "NAME", "Explosion of Excalibur (#23)");
}

