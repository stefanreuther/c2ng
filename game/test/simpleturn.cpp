/**
  *  \file game/test/simpleturn.cpp
  *  \brief Class game::test::SimpleTurn
  */

#include "game/test/simpleturn.hpp"
#include "afl/base/countof.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/configuration.hpp"

using game::map::Ship;
using game::map::Planet;

game::test::SimpleTurn::SimpleTurn()
    : m_turn(),
      m_interface(),
      m_config(),
      m_mapConfiguration(),
      m_shipList(),
      m_version(game::HostVersion::PHost, MKVERSION(3,5,0)),
      m_position(2000, 2000),
      m_hullNr(17)
{
    m_config.setDefaultValues();
}

game::map::Ship&
game::test::SimpleTurn::addShip(Id_t shipId, int owner, game::map::Object::Playability playability)
{
    // Make sure there is a hull, so querying the ship's hull properties works.
    if (m_shipList.hulls().get(m_hullNr) == 0) {
        game::spec::Hull* pHull = m_shipList.hulls().create(m_hullNr);
        afl::except::checkAssertion(pHull != 0, "invalid hull");
        pHull->setMass(1);
        pHull->setMaxCargo(100);
        pHull->setMaxFuel(100);
    }

    // Create ship
    Ship* pShip = universe().ships().create(shipId);
    afl::except::checkAssertion(pShip != 0, "invalid ship");

    // - seed the ship to make it visible
    game::map::ShipData sd;
    sd.x = m_position.getX();
    sd.y = m_position.getY();
    sd.owner = owner;
    pShip->addCurrentShipData(sd, game::PlayerSet_t(owner));
    pShip->internalCheck(game::PlayerSet_t(owner), m_turn.getTurnNumber());
    pShip->setPlayability(playability);

    // - set some nice properties
    pShip->setHull(m_hullNr);
    static const Element::Type elems[] = { Element::Neutronium, Element::Tritanium, Element::Duranium,
                                           Element::Molybdenum, Element::Supplies, Element::Colonists,
                                           Element::Money };
    for (size_t i = 0; i < countof(elems); ++i) {
        pShip->setCargo(elems[i], 10);
        pShip->setTransporterCargo(Ship::TransferTransporter, elems[i], 0);
        pShip->setTransporterCargo(Ship::UnloadTransporter, elems[i], 0);
    }
    pShip->setTransporterTargetId(Ship::TransferTransporter, 0);
    pShip->setTransporterTargetId(Ship::UnloadTransporter, 0);

    return *pShip;
}

game::map::Planet&
game::test::SimpleTurn::addPlanet(Id_t planetId, int owner, game::map::Object::Playability playability)
{
    // Create planet
    Planet* pPlanet = universe().planets().create(planetId);
    afl::except::checkAssertion(pPlanet != 0, "invalid planet");

    pPlanet->setPosition(m_position);

    game::map::PlanetData pd;
    pd.owner = owner;
    pd.minedNeutronium = 1000;
    pd.minedTritanium = 1000;
    pd.minedDuranium = 1000;
    pd.minedMolybdenum = 1000;
    pd.colonistClans = 1000;
    pd.money = 1000;
    pd.supplies = 1000;
    pPlanet->addCurrentPlanetData(pd, game::PlayerSet_t(owner));

    afl::string::NullTranslator tx;
    afl::sys::Log log;
    pPlanet->internalCheck(game::map::Configuration(), game::PlayerSet_t(owner), m_turn.getTurnNumber(), tx, log);
    pPlanet->setPlayability(playability);

    return *pPlanet;
}

game::map::Planet&
game::test::SimpleTurn::addBase(Id_t planetId, int owner, game::map::Object::Playability playability)
{
    // Create planet
    game::map::Planet& pl = addPlanet(planetId, owner, playability);

    // Add base
    game::map::BaseData d;
    d.engineStorage.set(9, 0);
    d.beamStorage.set(10, 0);
    d.launcherStorage.set(10, 0);
    d.torpedoStorage.set(10, 0);
    d.hullStorage.set(20, 0);
    pl.addCurrentBaseData(d, game::PlayerSet_t(owner));

    // Update m_baseKind
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    pl.internalCheck(game::map::Configuration(), game::PlayerSet_t(owner), m_turn.getTurnNumber(), tx, log);
    return pl;
}
