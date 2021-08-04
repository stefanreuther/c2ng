/**
  *  \file u/t_game_spec_info_nullpicturenamer.cpp
  *  \brief Test for game::spec::info::NullPictureNamer
  */

#include "game/spec/info/nullpicturenamer.hpp"

#include "t_game_spec_info.hpp"
#include "game/player.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/torpedolauncher.hpp"

/** Simple smoke test. */
void
TestGameSpecInfoNullPictureNamer::testIt()
{
    game::spec::info::NullPictureNamer testee;

    TS_ASSERT_EQUALS(testee.getHullPicture(game::spec::Hull(99)), "");
    TS_ASSERT_EQUALS(testee.getEnginePicture(game::spec::Engine(3)), "");
    TS_ASSERT_EQUALS(testee.getBeamPicture(game::spec::Beam(4)), "");
    TS_ASSERT_EQUALS(testee.getLauncherPicture(game::spec::TorpedoLauncher(5)), "");
    TS_ASSERT_EQUALS(testee.getAbilityPicture("cloak", game::spec::info::AbilityFlags_t()), "");
    TS_ASSERT_EQUALS(testee.getPlayerPicture(game::Player(7)), "");
    TS_ASSERT_EQUALS(testee.getVcrObjectPicture(false, 7), "");
}

