/**
  *  \file u/t_client_picturenamer.cpp
  *  \brief Test for client::PictureNamer
  */

#include "client/picturenamer.hpp"

#include "t_client.hpp"
#include "game/player.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/engine.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/torpedolauncher.hpp"

void
TestClientPictureNamer::testHull()
{
    game::spec::Hull h(105);
    h.setInternalPictureNumber(1002);
    h.setName("NILREM");

    client::PictureNamer testee;
    TS_ASSERT_EQUALS(testee.getHullPicture(h), "ship.1002.105");
}

void
TestClientPictureNamer::testEngine()
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
    TS_ASSERT_EQUALS(testee.getEnginePicture(e), "engine.8.44|50|58|65|72|78|86|93|100");
}

void
TestClientPictureNamer::testBeam()
{
    game::spec::Beam b(7);

    client::PictureNamer testee;
    TS_ASSERT_EQUALS(testee.getBeamPicture(b), "beam.7");
}

void
TestClientPictureNamer::testLauncher()
{
    game::spec::TorpedoLauncher tl(6);

    client::PictureNamer testee;
    TS_ASSERT_EQUALS(testee.getLauncherPicture(tl), "launcher.6");
}

void
TestClientPictureNamer::testPlayer()
{
    game::Player pl(3);

    client::PictureNamer testee;
    TS_ASSERT_EQUALS(testee.getPlayerPicture(pl), "");
}

void
TestClientPictureNamer::testAbility()
{
    client::PictureNamer testee;
    TS_ASSERT_EQUALS(testee.getAbilityPicture("cloak"), "ability.cloak");
    TS_ASSERT_EQUALS(testee.getAbilityPicture(""), "");
}

void
TestClientPictureNamer::testVcrObject()
{
    client::PictureNamer testee;
    TS_ASSERT_EQUALS(testee.getVcrObjectPicture(false, 9), "ship.9");
    TS_ASSERT_EQUALS(testee.getVcrObjectPicture(true, 200), "planet");
}

