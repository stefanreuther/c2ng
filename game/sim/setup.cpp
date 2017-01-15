/**
  *  \file game/sim/setup.cpp
  */

#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"

game::sim::Setup::Setup()
    : m_ships(),
      m_planet(),
      m_structureChanged(false)
{ }

game::sim::Setup::Setup(const Setup& other)
    : m_ships(),
      m_planet(),
      m_structureChanged(false)
{
    // ex GSimState::GSimState
    *this = other;
}

game::sim::Setup::~Setup()
{ }

game::sim::Setup&
game::sim::Setup::operator=(const Setup& other)
{
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

    return *this;
}

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

bool
game::sim::Setup::hasPlanet() const
{
    return m_planet.get() != 0;
}

const game::sim::Planet*
game::sim::Setup::getPlanet() const
{
    // ex GSimState::getPlanet
    return m_planet.get();
}

game::sim::Planet*
game::sim::Setup::getPlanet()
{
    // ex GSimState::getPlanet
    return m_planet.get();
}

void
game::sim::Setup::removePlanet()
{
    // ex GSimState::removePlanet
    if (m_planet.get() != 0) {
        m_planet.reset();
        m_structureChanged = true;
    }
}

game::sim::Ship*
game::sim::Setup::addShip()
{
    // ex GSimState::addShip
    Ship* result = m_ships.pushBackNew(new Ship());
    m_structureChanged = true;
    return result;
}

game::sim::Setup::Slot_t
game::sim::Setup::getNumShips() const
{
    // ex GSimState::getNumShips
    return m_ships.size();
}

const game::sim::Ship*
game::sim::Setup::getShip(Slot_t slot) const
{
    return const_cast<Setup*>(this)->getShip(slot);
}

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

// /** Remove a ship.
//     \param ix Index of ship to remove, [0,getNumShips()). */
void
game::sim::Setup::removeShip(Slot_t slot)
{
    // ex GSimState::removeShip
    if (slot < m_ships.size()) {
        m_ships.erase(m_ships.begin() + slot);
        m_structureChanged = true;
    }
}

game::sim::Setup::Slot_t
game::sim::Setup::getNumObjects() const
{
    size_t result = m_ships.size();
    if (m_planet.get() != 0) {
        ++result;
    }
    return result;
}

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

// /** Add ship at specific position.
//     \param i ship to duplicate
//     \param id Id number for the duplicate */
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

// /** Swap two ships.
//     \param a,b Indexes of ships to swap */
void
game::sim::Setup::swapShips(Slot_t a, Slot_t b)
{
    // ex GSimState::swapShips
    if (a < m_ships.size() && b < m_ships.size()) {
        m_ships.swapElements(a, b);
        m_structureChanged = true;
    }
}

// /** Get index of a GSimShip object.
//     \param sh Some ship from this GSimState
//     \inv getIndexOf(getShip(x)) == x */
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

// /** Get index of a ship by Id. */
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

// /** Get a free Id number.
//     \return Id number, 0 if none found */
game::Id_t
game::sim::Setup::findUnusedShipId(Id_t firstToCheck) const
{
    // ex GSimState::getFreeId
    // \change add firstToCheck to bring the "add N ships" operation down from O(n**3)
    // \change no limit to the maximum setup size
    Slot_t tmp;
    Id_t i = firstToCheck;
    while (findShipSlotById(i, tmp) != 0) {
        ++i;
    }
    return i;
}

// /** Perform all queued updates. */
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

// /** Assign random friendly codes for all objects that want them.
//     Calls GSimObject::assignRandomFCode() on all objects. */
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

// /** Check whether simulation matches ship list. Checks all ships.
//     \see GSimShip::isMatchingShipList(). */
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
