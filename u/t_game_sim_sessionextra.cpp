/**
  *  \file u/t_game_sim_sessionextra.cpp
  *  \brief Test for game::sim::SessionExtra
  */

#include "game/sim/sessionextra.hpp"

#include "t_game_sim.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test basic connection of game::Session and game::sim::Session.
    A: create a game::Session and obtain its game::sim::Session.
    E: session is returned and has a GameInterface. */
void
TestGameSimSessionExtra::testIt()
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session gs(tx, fs);
    afl::base::Ref<game::sim::Session> ss = game::sim::getSimulatorSession(gs);

    // Session does have a GameInterface, but that does not have a game
    TS_ASSERT(ss->getGameInterface() != 0);
    TS_ASSERT(!ss->getGameInterface()->hasGame());
    TS_ASSERT_EQUALS(ss->getGameInterface()->getMaxShipId(), 0);
    TS_ASSERT_EQUALS(ss->getGameInterface()->getMaxPlanetId(), 0);
}

