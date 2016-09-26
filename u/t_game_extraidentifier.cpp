/**
  *  \file u/t_game_extraidentifier.cpp
  *  \brief Test for game::ExtraIdentifier
  */

#include "game/extraidentifier.hpp"

#include "t_game.hpp"
#include "game/extra.hpp"

namespace {
    class MyExtra : public game::Extra { };
}

/** Simple test. */
void
TestGameExtraIdentifier::testIt()
{
    // The whole point of ExtraIdentifier is to create static instances.
    game::ExtraIdentifier<int,MyExtra> testee;
    (void) testee;
}
