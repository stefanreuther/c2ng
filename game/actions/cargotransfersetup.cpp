/**
  *  \file game/actions/cargotransfersetup.cpp
  *  \brief Class game::actions::CargoTransferSetup
  */

#include <algorithm>
#include "game/actions/cargotransfersetup.hpp"
#include "afl/base/memory.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/except/assertionfailedexception.hpp"
#include "game/actions/cargotransfer.hpp"
#include "game/exception.hpp"
#include "game/map/planetstorage.hpp"
#include "game/map/shipstorage.hpp"
#include "game/map/shiptransporter.hpp"
#include "game/map/beamupshiptransfer.hpp"
#include "game/map/beamupplanettransfer.hpp"

using afl::base::Memory;
using game::map::Object;
using game::map::Planet;
using game::map::Ship;

namespace {
    Ship& getShip(game::map::Universe& univ, int id)
    {
        Ship* pShip = univ.ships().get(id);
        afl::except::checkAssertion(pShip != 0, "invalid ship");
        return *pShip;
    }

    Planet& getPlanet(game::map::Universe& univ, int id)
    {
        Planet* pPlanet = univ.planets().get(id);
        afl::except::checkAssertion(pPlanet != 0, "invalid planet");
        return *pPlanet;
    }

    bool isConflictingTransfer(const game::map::Universe& univ, int shipId, int requiredTarget)
    {
        Ship* pShip = univ.ships().get(shipId);
        return pShip != 0
            && pShip->isTransporterActive(Ship::TransferTransporter)
            && pShip->getTransporterTargetId(Ship::TransferTransporter).orElse(0) != requiredTarget;
    }
}


// Default constructor.
game::actions::CargoTransferSetup::CargoTransferSetup()
{
    Memory<Action>(m_actions).fill(Invalid);
    Memory<Id_t>(m_ids).fill(0);
}

game::actions::CargoTransferSetup::CargoTransferSetup(Action leftAction, int leftId, Action rightAction, int rightId)
{
    m_actions[Left]  = leftAction;
    m_actions[Right] = rightAction;
    m_ids[Left]      = leftId;
    m_ids[Right]     = rightId;
    m_ids[Proxy]     = 0;
}

// Construct from a planet and ship.
game::actions::CargoTransferSetup
game::actions::CargoTransferSetup::fromPlanetShip(const game::map::Universe& univ, int planetId, int shipId)
{
    // Validate ship
    // Must have known position and owner.
    const Ship* pShip = univ.ships().get(shipId);
    game::map::Point shipPos;
    int shipOwner;
    if (pShip == 0 || !pShip->getPosition(shipPos) || !pShip->getOwner(shipOwner)) {
        return CargoTransferSetup();
    }

    // Validate planet
    const Planet* pPlanet = univ.planets().get(planetId);
    game::map::Point planetPos;
    int planetOwner;
    if (pPlanet == 0 || !pPlanet->getPosition(planetPos) || !pPlanet->getOwner(planetOwner)) {
        return CargoTransferSetup();
    }

    // Match
    if (shipPos != planetPos) {
        return CargoTransferSetup();
    }

    // Build result
    if (pShip->isPlayable(Object::Playable)) {
        if (pPlanet->isPlayable(Object::Playable) && shipOwner == planetOwner) {
            // Totally client-side transfer.
            return CargoTransferSetup(UsePlanetStorage, planetId,
                                      UseShipStorage,   shipId);
        } else {
            // We own the ship, but not the planet. Standard unload case.
            return CargoTransferSetup(UseOtherUnload,   planetId,
                                      UseShipStorage,   shipId);
        }
    } else {
        if (pPlanet->isPlayable(Object::Playable)) {
            // We own the planet, but not the ship. This requires a proxy.
            return CargoTransferSetup(UsePlanetStorage, planetId,
                                      UseProxyTransfer, shipId);
        } else {
            // We own neither.
            // In theory, this could be done using a double-proxy (one ship with two transfers, or two ships).
            // However, since ship transfers start empty, there wouldn't be anything to transfer
            // unless they had already used the proxy to transfer stuff.
            // This makes this too unimportant to support.
            return CargoTransferSetup();
        }
    }
}

// Construct from two ships.
game::actions::CargoTransferSetup
game::actions::CargoTransferSetup::fromShipShip(const game::map::Universe& univ, int leftId, int rightId)
{
    // Ids must not be identical
    if (leftId == rightId) {
        return CargoTransferSetup();
    }

    // Validate left Id
    const Ship* pLeft = univ.ships().get(leftId);
    game::map::Point leftPos;
    int leftOwner;
    if (pLeft == 0 || !pLeft->getPosition(leftPos) || !pLeft->getOwner(leftOwner)) {
        return CargoTransferSetup();
    }

    // Validate right Id
    const Ship* pRight = univ.ships().get(rightId);
    game::map::Point rightPos;
    int rightOwner;
    if (pRight == 0 || !pRight->getPosition(rightPos) || !pRight->getOwner(rightOwner)) {
        return CargoTransferSetup();
    }

    // Match
    if (leftPos != rightPos) {
        return CargoTransferSetup();
    }

    // Build result
    if (pLeft->isPlayable(Object::Playable)) {
        if (pRight->isPlayable(Object::Playable) && leftOwner == rightOwner) {
            // Totally client-side transfer.
            return CargoTransferSetup(UseShipStorage,  leftId,
                                      UseShipStorage,  rightId);
        } else {
            // We own left, but not right.
            return CargoTransferSetup(UseShipStorage,   leftId,
                                      UseOtherTransfer, rightId);
        }
    } else {
        if (pRight->isPlayable(Object::Playable)) {
            // We own right, but not left.
            return CargoTransferSetup(UseOtherTransfer, leftId,
                                      UseShipStorage,   rightId);
        } else {
            // We own neither.
            return CargoTransferSetup();
        }
    }
}

// Construct for jettison.
game::actions::CargoTransferSetup
game::actions::CargoTransferSetup::fromShipJettison(const game::map::Universe& univ, int shipId)
{
    // Validate ship Id: must exist and be playable
    const Ship* pShip = univ.ships().get(shipId);
    game::map::Point shipPos;
    if (pShip == 0 || !pShip->getPosition(shipPos) || !pShip->isPlayable(Object::Playable)) {
        return CargoTransferSetup();
    }

    // Validate position: must be in deep space
    if (univ.findPlanetAt(shipPos) != 0) {
        return CargoTransferSetup();
    }

    // OK
    return CargoTransferSetup(UseShipStorage, shipId, UseOtherUnload, 0);
}

game::actions::CargoTransferSetup
game::actions::CargoTransferSetup::fromShipBeamUp(const game::Turn& turn, int shipId, const game::config::HostConfiguration& config)
{
    // Validate configuration
    if (!config[config.AllowBeamUpMultiple]()) {
        return CargoTransferSetup();
    }

    // Validate ship Id: must exist and be playable
    const game::map::Universe& univ = turn.universe();
    const Ship* pShip = univ.ships().get(shipId);
    game::map::Point shipPos;
    if (pShip == 0 || !pShip->getPosition(shipPos) || !pShip->isPlayable(Object::Playable)) {
        return CargoTransferSetup();
    }

    // Validate position: there must be a planet
    Id_t planetId = univ.findPlanetAt(shipPos);
    if (planetId == 0) {
        return CargoTransferSetup();
    }

    // OK
    return CargoTransferSetup(UseBeamUpShip, shipId, UseBeamUpPlanet, planetId);
}

// Swap sides.
void
game::actions::CargoTransferSetup::swapSides()
{
    std::swap(m_actions[Left], m_actions[Right]);
    std::swap(m_ids[Left], m_ids[Right]);
}

// Get setup status.
game::actions::CargoTransferSetup::Result
game::actions::CargoTransferSetup::getStatus() const
{
    afl::bits::SmallSet<Action> actions;
    actions += m_actions[Left];
    actions += m_actions[Right];
    if (actions.contains(Invalid)) {
        return Impossible;
    } else if (actions.contains(UseProxyTransfer)) {
        if (m_ids[Proxy] == 0) {
            return NeedProxy;
        } else {
            return Ready;
        }
    } else {
        return Ready;
    }
}

// Check validity.
bool
game::actions::CargoTransferSetup::isValid() const
{
    return getStatus() == Ready;
}

// Check valid proxy.
bool
game::actions::CargoTransferSetup::isValidProxy(const game::map::Universe& univ, int shipId) const
{
    if (m_actions[Left] == UseProxyTransfer && m_actions[Right] == UsePlanetStorage) {
        return checkProxyPlanet(univ, shipId, m_ids[Right]);
    } else if (m_actions[Left] == UsePlanetStorage && m_actions[Right] == UseProxyTransfer) {
        return checkProxyPlanet(univ, shipId, m_ids[Left]);
    } else {
        return false;
    }
}

// Set proxy.
bool
game::actions::CargoTransferSetup::setProxy(const game::map::Universe& univ, Id_t shipId)
{
    if (isValidProxy(univ, shipId)) {
        m_ids[Proxy] = shipId;
        return true;
    } else {
        return false;
    }
}

// Check for conflicting transfer.
game::Id_t
game::actions::CargoTransferSetup::getConflictingTransferShipId(const game::map::Universe& univ) const
{
    // A conflict happens if...
    // - X's action is UseOtherTransfer, but other's transfer is active for a ship other than X
    // - X's action is UseProxyTransfer, but proxy's transfer is active for a ship other than X
    // We don't need to handle UseOtherUnload, because there's no choice where you unload, so there cannot legally be a conflicting unload.
    // FIXME: actually, we need to do something about UseOtherUnload for NuHost (!hasParallelShipTransfers()).
    for (size_t i = 0; i < 2; ++i) {
        const Id_t thisId = m_ids[i];
        const Id_t otherId = m_ids[i^1];
        switch (m_actions[i]) {
         case Invalid:
         case UsePlanetStorage:
         case UseShipStorage:
         case UseOtherUnload:
         case UseBeamUpShip:
         case UseBeamUpPlanet:
            break;

         case UseOtherTransfer:
            if (isConflictingTransfer(univ, otherId, thisId)) {
                return otherId;
            }
            break;

         case UseProxyTransfer:
            if (isConflictingTransfer(univ, m_ids[Proxy], thisId)) {
                return m_ids[Proxy];
            }
            break;
        }
    }
    return 0;
}

// Cancel conflicting transfer.
void
game::actions::CargoTransferSetup::cancelConflictingTransfer(game::map::Universe& univ, Id_t shipId)
{
    getShip(univ, shipId).cancelTransporter(Ship::TransferTransporter);
}

// Build CargoTransfer action.
void
game::actions::CargoTransferSetup::build(CargoTransfer& action,
                                         Turn& turn,
                                         const game::config::HostConfiguration& config,
                                         const game::spec::ShipList& shipList,
                                         const game::HostVersion& version,
                                         afl::string::Translator& tx)
{
    game::map::Universe& univ = turn.universe();

    // Deflect call if setup is invalid and user didn't notice.
    if (getStatus() != Ready) {
        throw Exception(Exception::ePerm);
    }

    // Cancel conflicting transfers if user didn't resolve that.
    // (paranoiaCounter to avoid accidentally looping forever. Should not be needed.)
    for (size_t paranoiaCounter = 0; paranoiaCounter < 100; ++paranoiaCounter) {
        const Id_t id = getConflictingTransferShipId(univ);
        if (id == 0) {
            break;
        }
        cancelConflictingTransfer(univ, id);
    }

    // Produce result
    for (size_t i = 0; i < 2; ++i) {
        const Id_t thisId = m_ids[i];
        const Id_t otherId = m_ids[i^1];
        switch (m_actions[i]) {
         case Invalid:
            throw Exception(Exception::ePerm);

         case UsePlanetStorage:
            action.addNew(new game::map::PlanetStorage(getPlanet(univ, thisId), config, tx));
            break;

         case UseShipStorage:
            action.addNew(new game::map::ShipStorage(getShip(univ, thisId), shipList, tx));
            break;

         case UseOtherUnload:
            action.addNew(new game::map::ShipTransporter(getShip(univ, otherId), Ship::UnloadTransporter, thisId, univ, version, tx));
            break;

         case UseOtherTransfer:
            action.addNew(new game::map::ShipTransporter(getShip(univ, otherId), Ship::TransferTransporter, thisId, univ, version, tx));
            break;

         case UseProxyTransfer:
            action.addNew(new game::map::ShipTransporter(getShip(univ, m_ids[Proxy]), Ship::TransferTransporter, thisId, univ, version, tx));
            break;

         case UseBeamUpShip:
            action.addNew(new game::map::BeamUpShipTransfer(getShip(univ, thisId), shipList, turn, config, tx));
            break;

         case UseBeamUpPlanet:
            action.addNew(new game::map::BeamUpPlanetTransfer(getPlanet(univ, thisId), getShip(univ, otherId), turn, config));
            break;
        }
    }
}

bool
game::actions::CargoTransferSetup::checkProxyPlanet(const game::map::Universe& univ, int shipId, int planetId) const
{
    // A ship is a valid proxy for a transfer A->B if A->P can be executed directly.
    // This checks ownership, location, and playability status.
    CargoTransferSetup proxyTransfer = fromPlanetShip(univ, planetId, shipId);
    return proxyTransfer.m_actions[Left] == UsePlanetStorage
        && proxyTransfer.m_actions[Right] == UseShipStorage;
}
