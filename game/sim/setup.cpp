/**
  *  \file game/sim/setup.cpp
  *  \brief Class game::sim::Setup
  */

#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"

// Construct empty list.
game::sim::Setup::Setup()
    : m_ships(),
      m_planet(),
      m_structureChanged(false)
{ }

// Copy a setup.
game::sim::Setup::Setup(const Setup& other)
    : m_ships(),
      m_planet(),
      m_structureChanged(false)
{
    // ex GSimState::GSimState
    *this = other;
}

// Destructor.
game::sim::Setup::~Setup()
{ }

// Assign another setup.
game::sim::Setup&
game::sim::Setup::operator=(const Setup& other)
{
    if (&other != this) {
        // Clear
        m_ships.clear();
        m_planet.reset();

        // Copy other
        m_ships.reserve(other.m_ships.size());
        for (Slot_t i = 0, n = other.m_ships.size(); i < n; ++i) {
            m_ships.pushBackNew(new Ship(*other.m_ships[i]));
        }
        if (other.m_planet.get() != 0) {
            m_planet.reset(new Planet(*other.m_planet));
        }
        m_structureChanged = true;
    }
    return *this;
}

// Add planet.
game::sim::Planet*
game::sim::Setup::addPlanet()
{
    // FIXME: PCC2 resets the planet to standard values (and marks it dirty); do we need this?
    if (!m_planet.get()) {
        m_planet.reset(new Planet());
        m_structureChanged = true;
    }
    return m_planet.get();
}

// Check presence of planet.
bool
game::sim::Setup::hasPlanet() const
{
    return m_planet.get() != 0;
}

// Get planet.
const game::sim::Planet*
game::sim::Setup::getPlanet() const
{
    // ex GSimState::getPlanet
    return m_planet.get();
}

// Get planet.
game::sim::Planet*
game::sim::Setup::getPlanet()
{
    // ex GSimState::getPlanet
    return m_planet.get();
}

// Remove planet.
void
game::sim::Setup::removePlanet()
{
    // ex GSimState::removePlanet
    if (m_planet.get() != 0) {
        m_planet.reset();
        m_structureChanged = true;
    }
}

// Add a ship.
game::sim::Ship*
game::sim::Setup::addShip()
{
    // ex GSimState::addShip
    Ship* result = m_ships.pushBackNew(new Ship());
    m_structureChanged = true;
    return result;
}

// Get number of ships.
game::sim::Setup::Slot_t
game::sim::Setup::getNumShips() const
{
    // ex GSimState::getNumShips
    return m_ships.size();
}

// Get ship, given a slot number.
const game::sim::Ship*
game::sim::Setup::getShip(Slot_t slot) const
{
    return const_cast<Setup*>(this)->getShip(slot);
}

// Get ship, given a slot number.
game::sim::Ship*
game::sim::Setup::getShip(Slot_t slot)
{
    // ex GSimState::getShip
    if (slot < m_ships.size()) {
        return m_ships[slot];
    } else {
        return 0;
    }
}

// Remove ship, given a slot number.
void
game::sim::Setup::removeShip(Slot_t slot)
{
    // ex GSimState::removeShip
    if (slot < m_ships.size()) {
        m_ships.erase(m_ships.begin() + slot);
        m_structureChanged = true;
    }
}

// Get number of objects.
game::sim::Setup::Slot_t
game::sim::Setup::getNumObjects() const
{
    size_t result = m_ships.size();
    if (m_planet.get() != 0) {
        ++result;
    }
    return result;
}

// Get object, given a slot number.
const game::sim::Object*
game::sim::Setup::getObject(Slot_t slot) const
{
    return const_cast<Setup*>(this)->getObject(slot);
}

// Find slot, given an object.
bool
game::sim::Setup::findIndex(const Object* obj, Slot_t& result) const
{
    if (obj != 0 && obj == m_planet.get()) {
        result = m_ships.size();
        return true;
    } else if (const Ship* ship = dynamic_cast<const Ship*>(obj)) {
        return findIndex(ship, result);
    } else {
        return false;
    }
}

// Get object, given a slot number.
game::sim::Object*
game::sim::Setup::getObject(Slot_t slot)
{
    size_t numShips = m_ships.size();
    if (slot < numShips) {
        return m_ships[slot];
    } else if (slot == numShips) {
        return m_planet.get();
    } else {
        return 0;
    }
}

// Duplicate a ship.
void
game::sim::Setup::duplicateShip(Slot_t slot, Id_t newId, afl::string::Translator& tx)
{
    // ex GSimState::duplicateShip
    if (slot < m_ships.size()) {
        Ship* sh = m_ships.insertNew(m_ships.begin() + slot + 1, new Ship(*m_ships[slot]));
        sh->setId(newId);
        sh->setDefaultName(tx);
        m_structureChanged = true;
    }
}

// Swap two ships.
void
game::sim::Setup::swapShips(Slot_t a, Slot_t b)
{
    // ex GSimState::swapShips, ccsim.pas:SwapSim (sort-of)
    if (a < m_ships.size() && b < m_ships.size()) {
        m_ships.swapElements(a, b);
        m_structureChanged = true;
    }
}

// Find ship slot, given an object.
bool
game::sim::Setup::findIndex(const Ship* ship, Slot_t& result) const
{
    // ex GSimState::getIndexOf
    for (Slot_t i = 0, n = m_ships.size(); i < n; ++i) {
        if (m_ships[i] == ship) {
            result = i;
            return true;
        }
    }
    return false;
}

// Find ship slot, given an Id.
bool
game::sim::Setup::findShipSlotById(Id_t id, Slot_t& result) const
{
    // ex GSimState::getIndexOf
    for (Slot_t i = 0, n = m_ships.size(); i < n; ++i) {
        if (m_ships[i]->getId() == id) {
            result = i;
            return true;
        }
    }
    return false;
}

// Find ship, given an Id.
game::sim::Ship*
game::sim::Setup::findShipById(Id_t id)
{
    // ex GSimState::getShipById
    Slot_t slot;
    if (findShipSlotById(id, slot)) {
        return getShip(slot);
    } else {
        return 0;
    }
}

// Find unused ship Id.
game::Id_t
game::sim::Setup::findUnusedShipId(Id_t firstToCheck) const
{
    // ex GSimState::getFreeId, ccsim.pas:NewId
    // \change add firstToCheck to bring the "add N ships" operation down from O(n**3)
    // \change no limit to the maximum setup size
    // FIXME: PCC 1.x tries to avoid ships used in the game
    Slot_t tmp;
    Id_t i = firstToCheck;
    while (findShipSlotById(i, tmp) != 0) {
        ++i;
    }
    return i;
}

// Merge from other setup.
void
game::sim::Setup::merge(const Setup& other)
{
    // ex mergeccb.pas:AddShip, AddPlanet
    // Merge ships
    for (size_t i = 0, n = other.getNumShips(); i < n; ++i) {
        if (const Ship* otherShip = other.getShip(i)) {
            Ship* myShip = findShipById(otherShip->getId());
            if (myShip == 0) {
                myShip = addShip();
            }
            if (myShip != 0) {
                *myShip = *otherShip;
                myShip->markDirty();
            }
        }
    }

    // Merge planet
    if (const Planet* otherPlanet = other.getPlanet()) {
        if (Planet* myPlanet = addPlanet()) {
            *myPlanet = *otherPlanet;
            myPlanet->markDirty();
        }
    }
}

// Notify listeners.
void
game::sim::Setup::notifyListeners()
{
    // ex GSimState::doUpdates
    if (m_structureChanged) {
        m_structureChanged = false;
        sig_structureChange.raise();
    }
    if (Planet* p = getPlanet()) {
        if (p->isDirty()) {
            p->markClean();
            sig_planetChange.raise();
        }
    }
    for (size_t i = 0, n = getNumShips(); i < n; ++i) {
        if (Ship* sh = getShip(i)) {
            if (sh->isDirty()) {
                sh->markClean();
                sig_shipChange.raise(i);
            }
        }
    }
}

// Set random friendly codes.
void
game::sim::Setup::setRandomFriendlyCodes()
{
    // ex GSimState::assignRandomFCodes
    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        if (Object* p = getObject(i)) {
            p->setRandomFriendlyCode();
        }
    }
}

// Check whether this setup matches a ship list.
bool
game::sim::Setup::isMatchingShipList(const game::spec::ShipList& shipList) const
{
    // ex GSimState::isMatchingShipList
    for (size_t i = 0, n = getNumShips(); i < n; ++i) {
        if (const Ship* p = getShip(i)) {
            if (!p->isMatchingShipList(shipList)) {
                return false;
            }
        }
    }
    return true;
}
