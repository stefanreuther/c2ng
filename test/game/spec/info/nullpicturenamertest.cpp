/**
  *  \file test/game/spec/info/nullpicturenamertest.cpp
  *  \brief Test for game::spec::info::NullPictureNamer
  */

#include "game/spec/info/nullpicturenamer.hpp"

#include "afl/test/testrunner.hpp"
#include "game/player.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/torpedolauncher.hpp"

/** Simple smoke test. */
AFL_TEST("game.spec.info.NullPictureNamer", a)
{
    game::spec::info::NullPictureNamer testee;

    a.checkEqual("01. getHullPicture",      testee.getHullPicture(game::spec::Hull(99)), "");
    a.checkEqual("02. getEnginePicture",    testee.getEnginePicture(game::spec::Engine(3)), "");
    a.checkEqual("03. getBeamPicture",      testee.getBeamPicture(game::spec::Beam(4)), "");
    a.checkEqual("04. getLauncherPicture",  testee.getLauncherPicture(game::spec::TorpedoLauncher(5)), "");
    a.checkEqual("05. getAbilityPicture",   testee.getAbilityPicture("cloak", game::spec::info::AbilityFlags_t()), "");
    a.checkEqual("06. getPlayerPicture",    testee.getPlayerPicture(game::Player(7)), "");
    a.checkEqual("07. getVcrObjectPicture", testee.getVcrObjectPicture(false, 7), "");
}
