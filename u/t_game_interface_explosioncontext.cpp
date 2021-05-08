/**
  *  \file u/t_game_interface_explosioncontext.cpp
  *  \brief Test for game::interface::ExplosionContext
  */

#include "game/interface/explosioncontext.hpp"

#include "t_game_interface.hpp"
#include "game/session.hpp"
#include "game/game.hpp"
#include "interpreter/test/contextverifier.hpp"
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
    interpreter::test::ContextVerifier v(testee, "testIt");
    v.verifyTypes();

    // Verify some values
    v.verifyInteger("ID", 1);
    v.verifyInteger("ID.SHIP", 23);
    v.verifyInteger("LOC.X", 1000);
    v.verifyInteger("LOC.Y", 1020);
    v.verifyString("TYPE", "Explosion");
    v.verifyString("TYPE.SHORT", "E");
    v.verifyString("NAME.SHIP", "Excalibur");
    v.verifyString("NAME", "Explosion of Excalibur (#23)");
}

