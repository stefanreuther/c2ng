/**
  *  \file test/client/picturenamertest.cpp
  *  \brief Test for client::PictureNamer
  */

#include "client/picturenamer.hpp"

#include "afl/test/testrunner.hpp"
#include "game/player.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/torpedolauncher.hpp"

AFL_TEST("client.PictureNamer:getHullPicture", a)
{
    game::spec::Hull h(105);
    h.setInternalPictureNumber(1002);
    h.setName("NILREM");

    client::PictureNamer testee;
    a.checkEqual("01", testee.getHullPicture(h), "ship.1002.105");
}

AFL_TEST("client.PictureNamer:getEnginePicture", a)
{
    game::spec::Engine e(8);
    e.setName("Improbability Drive");
    e.setFuelFactor(1, 44);
    e.setFuelFactor(2, 200);
    e.setFuelFactor(3, 522);
    e.setFuelFactor(4, 1040);
    e.setFuelFactor(5, 1800);
    e.setFuelFactor(6, 2808);
    e.setFuelFactor(7, 4214);
    e.setFuelFactor(8, 5952);
    e.setFuelFactor(9, 8100);

    client::PictureNamer testee;
    a.checkEqual("01", testee.getEnginePicture(e), "engine.8.44|50|58|65|72|78|86|93|100");
}

AFL_TEST("client.PictureNamer:getBeamPicture", a)
{
    game::spec::Beam b(7);

    client::PictureNamer testee;
    a.checkEqual("01", testee.getBeamPicture(b), "beam.7");
}

AFL_TEST("client.PictureNamer:getLauncherPicture", a)
{
    game::spec::TorpedoLauncher tl(6);

    client::PictureNamer testee;
    a.checkEqual("01", testee.getLauncherPicture(tl), "launcher.6");
}

AFL_TEST("client.PictureNamer:getPlayerPicture", a)
{
    game::Player pl(3);

    client::PictureNamer testee;
    a.checkEqual("01", testee.getPlayerPicture(pl), "");
}

AFL_TEST("client.PictureNamer:getAbilityPicture", a)
{
    client::PictureNamer testee;
    a.checkEqual("01", testee.getAbilityPicture("cloak", game::spec::info::AbilityFlags_t()), "ability.cloak");
    a.checkEqual("02", testee.getAbilityPicture("", game::spec::info::AbilityFlags_t()), "");
}

AFL_TEST("client.PictureNamer:getVcrObjectPicture", a)
{
    client::PictureNamer testee;
    a.checkEqual("01", testee.getVcrObjectPicture(false, 9), "ship.9");
    a.checkEqual("02", testee.getVcrObjectPicture(true, 200), "planet");
}
