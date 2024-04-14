/**
  *  \file game/v3/reverter.cpp
  *  \brief Class game::v3::Reverter
  */

#include "game/v3/reverter.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/element.hpp"
#include "game/game.hpp"
#include "game/map/locationreverter.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/playerset.hpp"
#include "game/root.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"
#include "game/v3/undoinformation.hpp"

using game::map::BaseData;
using game::map::Planet;
using game::map::PlanetData;
using game::map::Point;
using game::map::Ship;
using game::map::ShipData;

namespace {
    template<typename T>
    T& mustExist(T* p, const char* what)
    {
        afl::except::checkAssertion(p != 0, what);
        return *p;
    }

    void setTransporter(Ship& ship, Ship::Transporter which, const ShipData::Transfer& old)
    {
        using game::Element;
        ship.setTransporterTargetId(which, old.targetId);
        ship.setTransporterCargo(which, Element::Neutronium, old.neutronium);
        ship.setTransporterCargo(which, Element::Tritanium,  old.tritanium);
        ship.setTransporterCargo(which, Element::Duranium,   old.duranium);
        ship.setTransporterCargo(which, Element::Molybdenum, old.molybdenum);
        ship.setTransporterCargo(which, Element::Colonists,  old.colonists);
        ship.setTransporterCargo(which, Element::Supplies,   old.supplies);
    }

    void setBaseStorage(Planet& planet, game::TechLevel area, const game::map::BaseStorage& storage)
    {
        for (int i = 1, n = storage.size(); i < n; ++i) {
            planet.setBaseStorage(area, i, storage.get(i));
        }
    }
}

/*
 *  Local LocationReverter
 */

class game::v3::Reverter::MyLocationReverter : public game::map::LocationReverter {
 public:
    MyLocationReverter(const Reverter& parent, Point pt);

    virtual game::ref::List getAffectedObjects() const;
    virtual Modes_t getAvailableModes() const;
    virtual void commit(Modes_t modes);

 private:
    void removeCommands(Reference ref);

    const Reverter& m_parent;
    Modes_t m_modes;
    game::ref::List m_list;
    PlayerSet_t m_players;
};


/*
 *  Reverter
 */

game::v3::Reverter::Reverter(Turn& turn, Session& session)
    : m_turn(turn),
      m_session(session)
{ }

afl::base::Optional<int>
game::v3::Reverter::getMinBuildings(int planetId, PlanetaryBuilding building) const
{
    switch (building) {
     case MineBuilding:
        if (const PlanetData* previousData = m_oldPlanetData.get(planetId)) {
            return previousData->numMines;
        }
        break;

     case FactoryBuilding:
        if (const PlanetData* previousData = m_oldPlanetData.get(planetId)) {
            return previousData->numFactories;
        }
        break;

     case DefenseBuilding:
        if (const PlanetData* previousData = m_oldPlanetData.get(planetId)) {
            return previousData->numDefensePosts;
        }
        break;

     case BaseDefenseBuilding:
        if (const BaseData* previousData = m_oldBaseData.get(planetId)) {
            return previousData->numBaseDefensePosts;
        }
        break;
    }
    return afl::base::Nothing;
}

int
game::v3::Reverter::getSuppliesAllowedToBuy(int planetId) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getSuppliesAllowedToBuy();
    } else {
        return 0;
    }
}

afl::base::Optional<int>
game::v3::Reverter::getMinTechLevel(int planetId, TechLevel techLevel) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getMinTechLevel(techLevel);
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<int>
game::v3::Reverter::getMinBaseStorage(int planetId, TechLevel area, int slot) const
{
    if (const BaseData* previousData = m_oldBaseData.get(planetId)) {
        if (const game::map::BaseStorage* previousStorage = game::map::getBaseStorage(*previousData, area)) {
            return previousStorage->get(slot);
        }
    }
    return afl::base::Nothing;
}

int
game::v3::Reverter::getNumTorpedoesAllowedToSell(int planetId, int slot) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getNumTorpedoesAllowedToSell(slot);
    } else {
        return 0;
    }
}

int
game::v3::Reverter::getNumFightersAllowedToSell(int planetId) const
{
    UndoInformation u;
    if (prepareUndoInformation(u, planetId)) {
        return u.getNumFightersAllowedToSell();
    } else {
        return 0;
    }
}

afl::base::Optional<String_t>
game::v3::Reverter::getPreviousShipFriendlyCode(Id_t shipId) const
{
    if (const ShipData* pShip = m_oldShipData.get(shipId)) {
        return pShip->friendlyCode;
    } else {
        return afl::base::Nothing;
    }
}

afl::base::Optional<String_t>
game::v3::Reverter::getPreviousPlanetFriendlyCode(Id_t planetId) const
{
    if (const PlanetData* pPlanet = m_oldPlanetData.get(planetId)) {
        return pPlanet->friendlyCode;
    } else {
        return afl::base::Nothing;
    }
}

bool
game::v3::Reverter::getPreviousShipMission(int shipId, int& m, int& i, int& t) const
{
    if (const ShipData* pShip = m_oldShipData.get(shipId)) {
        return pShip->mission.get(m)
            && pShip->missionInterceptParameter.get(i)
            && pShip->missionTowParameter.get(t);
    } else {
        return false;
    }
}

bool
game::v3::Reverter::getPreviousShipBuildOrder(int planetId, ShipBuildOrder& result) const
{
    if (const BaseData* previousData = m_oldBaseData.get(planetId)) {
        result = previousData->shipBuildOrder;
        return true;
    } else {
        return false;
    }
}

game::map::LocationReverter*
game::v3::Reverter::createLocationReverter(game::map::Point pt) const
{
    return new MyLocationReverter(*this, pt);
}

void
game::v3::Reverter::addShipData(int id, const game::map::ShipData& data)
{
    if (ShipData* p = m_oldShipData.create(id)) {
        *p = data;
    }
}

void
game::v3::Reverter::addPlanetData(int id, const game::map::PlanetData& data)
{
    if (PlanetData* p = m_oldPlanetData.create(id)) {
        *p = data;
    }
}

void
game::v3::Reverter::addBaseData(int id, const game::map::BaseData& data)
{
    if (BaseData* p = m_oldBaseData.create(id)) {
        *p = data;
    }
}

const game::map::ShipData*
game::v3::Reverter::getShipData(int id) const
{
    return m_oldShipData.get(id);
}

const game::map::PlanetData*
game::v3::Reverter::getPlanetData(int id) const
{
    return m_oldPlanetData.get(id);
}

const game::map::BaseData*
game::v3::Reverter::getBaseData(int id) const
{
    return m_oldBaseData.get(id);
}

inline game::map::Universe&
game::v3::Reverter::universe() const
{
    return m_turn.universe();
}

bool
game::v3::Reverter::prepareUndoInformation(UndoInformation& u, int planetId) const
{
    if (m_session.getShipList().get() != 0 && m_session.getRoot().get() != 0) {
        u.set(universe(), *m_session.getShipList(), m_session.getRoot()->hostConfiguration(), *this, planetId);
        return true;
    } else {
        return false;
    }
}

inline
game::v3::Reverter::MyLocationReverter::MyLocationReverter(const Reverter& parent, Point pt)
    : m_parent(parent),
      m_modes(),
      m_list(),
      m_players()
{
    // ex GReset::addLocation

    // By default, we can revert everything for all found units.
    // If we find a played unit that has no Undo data, we refuse to revert Cargo
    // for everyone else because that might create inconsistencies.
    m_modes += Missions;
    m_modes += Cargo;

    game::map::Universe& univ = m_parent.universe();

    // Planets
    if (Id_t planetId = univ.findPlanetAt(pt)) {
        if (const Planet* planet = univ.playedPlanets().getObjectByIndex(planetId)) {
            int planetOwner;
            if (m_parent.getPlanetData(planetId) != 0
                && planet->getOwner().get(planetOwner)
                && (!planet->hasBase() || m_parent.getBaseData(planetId) != 0))
            {
                m_list.add(Reference(Reference::Planet, planetId));
                m_players += planetOwner;
            } else {
                m_modes -= Cargo;
            }
        }
    }

    // Ships
    game::map::PlayedShipType& ships = univ.playedShips();
    for (Id_t shipId = ships.findNextIndex(0); shipId != 0; shipId = ships.findNextIndex(shipId)) {
        Ship* ship = ships.getObjectByIndex(shipId);
        Point shipPosition;
        int shipOwner;
        if (ship != 0 && ship->getPosition().get(shipPosition) && m_parent.getShipData(shipId) != 0 && ship->getOwner().get(shipOwner)) {
            if (shipPosition == pt) {
                m_list.add(Reference(Reference::Ship, shipId));
                m_players += shipOwner;
            }
        } else {
            m_modes -= Cargo;
        }
    }
}

game::ref::List
game::v3::Reverter::MyLocationReverter::getAffectedObjects() const
{
    return m_list;
}

game::map::LocationReverter::Modes_t
game::v3::Reverter::MyLocationReverter::getAvailableModes() const
{
    return m_modes;
}

void
game::v3::Reverter::MyLocationReverter::commit(Modes_t modes)
{
    // ex GReset::commit()

    // Sanitize
    modes &= m_modes;

    // Do it
    game::map::Universe& univ = m_parent.universe();
    for (size_t i = 0, n = m_list.size(); i < n; ++i) {
        Reference r = m_list[i];
        if (r.getType() == Reference::Ship) {
            Ship& ship = mustExist(univ.ships().get(r.getId()), "missing current ship");
            const ShipData& oldShip = mustExist(m_parent.getShipData(r.getId()), "missing old ship");
            // Ship record:
            // Keep:    Id, Player               4 bytes
            // Mission: Waypoint, Speed, FC      9 bytes
            // Keep:    Location, Equipment     16 bytes
            // Cargo:   Ammo                     2 bytes
            // Keep:    Torp launcher            2 bytes
            // Mission: Mission, PE, TowID       6 bytes
            // Keep:    Damage, Crew             4 bytes
            // Cargo:   Colonists                2 bytes
            // Mission: Name                    20 bytes
            // Cargo:   Cargo, Transfers        38 bytes
            // Mission: Intercept ID             2 bytes
            // Cargo:   Money                    2 bytes
            //                              -> 107 bytes

            if (modes.contains(Missions)) {
                // Waypoint, Speed, FC
                ship.setWaypoint(Point(oldShip.x.orElse(0) + oldShip.waypointDX.orElse(0), oldShip.y.orElse(0) + oldShip.waypointDY.orElse(0)));
                ship.setWarpFactor(oldShip.warpFactor);
                ship.setFriendlyCode(oldShip.friendlyCode);

                // Mission, PE, TowID + IntID
                ship.setMission(oldShip.mission, oldShip.missionInterceptParameter, oldShip.missionTowParameter);
                ship.setPrimaryEnemy(oldShip.primaryEnemy);

                // Name
                if (const String_t* name = oldShip.name.get()) {
                    ship.setName(*name);
                }

                // Commands
                removeCommands(r);
            }

            if (modes.contains(Cargo)) {
                // Transfers
                setTransporter(ship, Ship::UnloadTransporter,   oldShip.unload);
                setTransporter(ship, Ship::TransferTransporter, oldShip.transfer);

                // Remaining cargo
                ship.setCargo(Element::Neutronium, oldShip.neutronium);
                ship.setCargo(Element::Tritanium,  oldShip.tritanium);
                ship.setCargo(Element::Duranium,   oldShip.duranium);
                ship.setCargo(Element::Molybdenum, oldShip.molybdenum);
                ship.setCargo(Element::Supplies,   oldShip.supplies);
                ship.setCargo(Element::Colonists,  oldShip.colonists);
                ship.setCargo(Element::Money,      oldShip.money);
                ship.setAmmo(oldShip.ammo);
            }
        } else if (r.getType() == Reference::Planet) {
            Planet& planet = mustExist(univ.planets().get(r.getId()), "missing current planet");
            const PlanetData& oldPlanet = mustExist(m_parent.getPlanetData(r.getId()), "missing old planet");
            const BaseData* oldBase = m_parent.getBaseData(r.getId());

            // Planet record:
            // Keep:    PID, Player              4 bytes
            // Mission: FC                       3 bytes
            // Cargo:   Structures               6 bytes
            // Cargo:   N/T/D/M, Col, Sup, MC   28 bytes
            // Keep:    Ground N/T/D/M, Density 24 bytes
            // Mission: Taxes                    4 bytes
            // Keep:    Happy, Natives, Temp    14 bytes
            // Cargo:   Base                     2 bytes
            //                               -> 85 bytes

            // Starbase record
            // Keep:    PID, Player              4 bytes
            // Cargo:   Defense                  2 bytes
            // Keep:    Damage                   2 bytes
            // Cargo:   Tech                     8 bytes
            // Cargo:   Engine storage          18 bytes
            // Cargo:   Hull storage            40 bytes
            // Cargo:   Beam, Torp, Tube store  60 bytes
            // Cargo:   Fighter storage          2 bytes
            // Mission: Recycle, Mission         6 bytes
            // Cargo:   Build order             14 bytes
            //                              -> 156 bytes
            if (modes.contains(Missions)) {
                // Planet
                planet.setFriendlyCode(oldPlanet.friendlyCode);
                planet.setColonistTax(oldPlanet.colonistTax);
                planet.setNativeTax(oldPlanet.nativeTax);

                // Base
                if (planet.hasBase()) {
                    const BaseData& b = mustExist(oldBase, "missing old base");
                    planet.setBaseShipyardOrder(b.shipyardAction, b.shipyardId);
                    planet.setBaseMission(b.mission);
                }

                // Commands
                removeCommands(r);
            }

            if (modes.contains(Cargo)) {
                // Structures
                planet.setNumBuildings(MineBuilding,    oldPlanet.numMines);
                planet.setNumBuildings(FactoryBuilding, oldPlanet.numFactories);
                planet.setNumBuildings(DefenseBuilding, oldPlanet.numDefensePosts);

                // Cargo (N/T/D/M/Sup/MC/Col)
                planet.setCargo(Element::Neutronium, oldPlanet.minedNeutronium);
                planet.setCargo(Element::Tritanium,  oldPlanet.minedTritanium);
                planet.setCargo(Element::Duranium,   oldPlanet.minedDuranium);
                planet.setCargo(Element::Molybdenum, oldPlanet.minedMolybdenum);
                planet.setCargo(Element::Supplies,   oldPlanet.supplies);
                planet.setCargo(Element::Colonists,  oldPlanet.colonistClans);
                planet.setCargo(Element::Money,      oldPlanet.money);

                // Base build order
                planet.setBuildBaseFlag(oldPlanet.baseFlag.orElse(0));

                if (planet.hasBase()) {
                    const BaseData& b = mustExist(oldBase, "missing old base 2");

                    // Fighters
                    planet.setCargo(Element::Fighters, b.numFighters);

                    // Torp storage
                    for (int i = 1, n = b.torpedoStorage.size(); i < n; ++i) {
                        planet.setCargo(Element::fromTorpedoType(i), b.torpedoStorage.get(i));
                    }

                    // Ship build order
                    planet.setBaseBuildOrder(b.shipBuildOrder);

                    // Tech
                    for (size_t i = 0; i < NUM_TECH_AREAS; ++i) {
                        planet.setBaseTechLevel(TechLevel(i), b.techLevels[i]);
                    }

                    // Base storage
                    setBaseStorage(planet, HullTech, b.hullStorage);
                    setBaseStorage(planet, EngineTech, b.engineStorage);
                    setBaseStorage(planet, BeamTech, b.beamStorage);
                    setBaseStorage(planet, TorpedoTech, b.launcherStorage);
                }
            }
        } else {
            // cannot happen
        }
    }
}

void
game::v3::Reverter::MyLocationReverter::removeCommands(Reference ref)
{
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (CommandContainer* cc = CommandExtra::get(m_parent.m_turn, i)) {
            cc->removeCommandsByReference(ref);
        }
    }
}
