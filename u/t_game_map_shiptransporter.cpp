/**
  *  \file u/t_game_map_shiptransporter.cpp
  *  \brief Test for game::map::ShipTransporter
  */

#include "game/map/shiptransporter.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/hostversion.hpp"
#include "game/test/simpleturn.hpp"

using game::Element;
using game::HostVersion;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;
using game::map::ShipTransporter;
using game::test::SimpleTurn;

void
TestGameMapShipTransporter::testNames()
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

    TS_ASSERT_EQUALS(ShipTransporter(sourceShip, Ship::UnloadTransporter, 0, h.universe(), host).getName(tx),
                     "Jettison");
    TS_ASSERT_EQUALS(ShipTransporter(sourceShip, Ship::UnloadTransporter, 30, h.universe(), host).getName(tx),
                     "Uranus");
    TS_ASSERT_EQUALS(ShipTransporter(sourceShip, Ship::UnloadTransporter, 99, h.universe(), host).getName(tx),
                     "Planet 99");
    TS_ASSERT_EQUALS(ShipTransporter(sourceShip, Ship::TransferTransporter, 20, h.universe(), host).getName(tx),
                     "Target");
    TS_ASSERT_EQUALS(ShipTransporter(sourceShip, Ship::TransferTransporter, 99, h.universe(), host).getName(tx),
                     "Ship 99");
}

