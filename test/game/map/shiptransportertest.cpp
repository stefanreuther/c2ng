/**
  *  \file test/game/map/shiptransportertest.cpp
  *  \brief Test for game::map::ShipTransporter
  */

#include "game/map/shiptransporter.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::HostVersion;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;
using game::map::ShipTransporter;
using game::test::SimpleTurn;

AFL_TEST("game.map.ShipTransporter:getName", a)
{
    SimpleTurn h;
    HostVersion host(HostVersion::PHost, MKVERSION(4,0,0));

    Ship& sourceShip = h.addShip(10, 5, Object::Playable);
    sourceShip.setName("Source");

    Ship& targetShip = h.addShip(20, 7, Object::NotPlayable);
    targetShip.setName("Target");

    Planet& targetPlanet = h.addPlanet(30, 8, Object::NotPlayable);
    targetPlanet.setName("Uranus");

    afl::string::NullTranslator tx;

    a.checkEqual("01", ShipTransporter(sourceShip, Ship::UnloadTransporter,    0, h.universe(), host).getName(tx), "Jettison");
    a.checkEqual("02", ShipTransporter(sourceShip, Ship::UnloadTransporter,   30, h.universe(), host).getName(tx), "Uranus");
    a.checkEqual("03", ShipTransporter(sourceShip, Ship::UnloadTransporter,   99, h.universe(), host).getName(tx), "Planet 99");
    a.checkEqual("04", ShipTransporter(sourceShip, Ship::TransferTransporter, 20, h.universe(), host).getName(tx), "Target");
    a.checkEqual("05", ShipTransporter(sourceShip, Ship::TransferTransporter, 99, h.universe(), host).getName(tx), "Ship 99");

    // Info not currently set
    a.checkEqual("11", ShipTransporter(sourceShip, Ship::UnloadTransporter, 0, h.universe(), host).getInfo1(tx), "");
    a.checkEqual("12", ShipTransporter(sourceShip, Ship::UnloadTransporter, 0, h.universe(), host).getInfo2(tx), "");
}
