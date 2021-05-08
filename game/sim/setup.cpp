/**
  *  \file game/sim/setup.cpp
  *  \brief Class game::sim::Setup
  */

#include "game/sim/setup.hpp"
#include "game/sim/gameinterface.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"

namespace {
    typedef int CompareFunction_t(const game::sim::Ship&, const game::sim::Ship&);

    class CompareShips {
     public:
        CompareShips(CompareFunction_t* compare)
            : m_compare(compare)
            { }

        bool operator()(const game::sim::Ship& a, const game::sim::Ship& b) const
            {
                int c = m_compare(a, b);
                if (c != 0) {
                    return c < 0;
                } else {
                    return a.getId() < b.getId();
                }
            }

     private:
        CompareFunction_t* m_compare;
    };
}

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

// Add planet from data.
game::sim::Planet*
game::sim::Setup::addPlanet(const Planet& data)
{
    Planet* pl = addPlanet();
    if (pl != 0) {
        *pl = data;
    }
    return pl;
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

// Add a ship from data.
game::sim::Ship*
game::sim::Setup::addShip(const Ship& data)
{
    Ship* sh = findShipById(data.getId());
    if (sh == 0) {
        sh = addShip();
    }
    if (sh != 0) {
        *sh = data;
    }
    return sh;
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
    // ex GSimState::duplicateShip, WSimListWithHandler::addShip (part)
    if (slot < m_ships.size()) {
        Ship* sh = m_ships.insertNew(m_ships.begin() + slot + 1, new Ship(*m_ships[slot]));
        sh->setId(newId);
        sh->setDefaultName(tx);

        // Duplicate never is initially deactivated
        // (applies when a ship is created using [Ins] in the GUI, which duplicates the current ship)
        sh->setFlags(sh->getFlags() & ~Object::fl_Deactivated);

        m_structureChanged = true;
    }
}

// Replicate a ship.
void
game::sim::Setup::replicateShip(Slot_t slot, int count, const GameInterface* gi, afl::string::Translator& tx)
{
    // ex WSimListWithHandler::replicateCurrentShip
    // The most naive implementation of this algorithm is O(n^3) which is definitely too slow.,
    // This implementation is an O(n^2) operation. PCC2 uses a bitset to do it in O(n),
    // but since O(n^2) is good enough for the JavaScript version even in prehistoric browsers,
    // let's keep it simple.
    Id_t id = 0;
    for (int i = 0; i < count; ++i) {
        id = findUnusedShipId(id+1, gi);
        duplicateShip(slot, id, tx);
        ++slot;
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

// Find ship, given an Id (const version).
const game::sim::Ship*
game::sim::Setup::findShipById(Id_t id) const
{
    return const_cast<Setup&>(*this).findShipById(id);
}

// Find unused ship Id.
game::Id_t
game::sim::Setup::findUnusedShipId(Id_t firstToCheck, const GameInterface* gi) const
{
    // ex GSimState::getFreeId, ccsim.pas:NewId
    // \change add firstToCheck to bring the "add N ships" operation down from O(n**3)
    // \change no limit to the maximum setup size
    Slot_t tmp;
    Id_t i = firstToCheck;
    while ((gi != 0 && gi->hasShip(i)) || findShipSlotById(i, tmp)) {
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

// Sort ships.
void
game::sim::Setup::sortShips(int compare(const Ship&, const Ship&))
{
    m_ships.sort(CompareShips(compare));
    m_structureChanged = true;
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
game::sim::Setup::setRandomFriendlyCodes(util::RandomNumberGenerator& rng)
{
    // ex GSimState::assignRandomFCodes
    for (size_t i = 0, n = getNumObjects(); i < n; ++i) {
        if (Object* p = getObject(i)) {
            p->setRandomFriendlyCode(rng);
        }
    }
}

// Set a sequential friendly code.
void
game::sim::Setup::setSequentialFriendlyCode(Slot_t slot)
{
    // ex WSimListWithHandler::incrFCode, ccsim.pas:IncrFCode
    // FIXME: pass a RNG!
    // FIXME: the handling of fl_RandomDigits is strange? (but same as in PCC2 and PCC1)
    if (Object* p = getObject(slot)) {
        // Determine previous friendly code
        String_t friendlyCode;
        int32_t previousFlags = 0;
        if (const Object* prev = getObject(slot-1)) {
            friendlyCode = prev->getFriendlyCode();
            previousFlags = prev->getFlags();
        }

        // Copy "random digits" flags
        const int32_t newFlags = ((previousFlags & Object::fl_RandomDigits)
                                  | (p->getFlags() & ~Object::fl_RandomDigits));

        // Initialize by making friendly code 3 digits
        if (friendlyCode.size() < 3) {
            friendlyCode.append(3 - friendlyCode.size(), ' ');
        }

        // Make it numeric by randomizing random digits (if any) and non-numerics
        for (int i = 0; i < 3; ++i) {
            if (((newFlags & Object::fl_RandomFC) != 0
                 && (newFlags & (Object::fl_RandomFC1 << i)) != 0)
                || (friendlyCode[i] < '0' || friendlyCode[i] > '9'))
            {
                friendlyCode[i] = static_cast<char>('0' + (std::rand() % 10));
            }
        }

        // Increase to find new code.
        // Attempt to make it unique, but don't crash if that's not possible.
        // The code is completely numeric, so this might time-out if someone makes a .ccb file with >1000 entries).
        for (int tries = 0; tries < 1000; ++tries) {
            // Increase fcode
            for (int i = 3; i > 0; --i) {
                ++friendlyCode[i-1];
                if (friendlyCode[i-1] <= '9') {
                    break;
                }
                friendlyCode[i-1] = '0';
            }

            // Check for uniqueness
            bool ok = true;
            for (Slot_t i = 0, n = getNumObjects(); i < n && ok; ++i) {
                if (const Object* obj = getObject(i)) {
                    if (i != slot && obj->getFriendlyCode() == friendlyCode) {
                        ok = false;
                    }
                }
            }
            if (ok) {
                break;
            }
        }

        // Assign new fcode
        p->setFriendlyCode(friendlyCode);
        p->setFlags(newFlags);
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

// Copy to game using a GameInterface, all units.
game::sim::Setup::Status
game::sim::Setup::copyToGame(GameInterface& gi) const
{
    return copyToGame(gi, 0, getNumObjects());
}

// Copy to game using a GameInterface, range.
game::sim::Setup::Status
game::sim::Setup::copyToGame(GameInterface& gi, size_t from, size_t to) const
{
    // ex WSimScreen::writeToGame (part), ccsim.pas:Writeback
    // FIXME: PCC1 does multiple passes to resolve dependencies;
    // its copyShipToGame (DoWriteback) can therefore return "partial" in addition to "ok" and "fail".
    Status st(0, 0);
    for (size_t i = from; i < to; ++i) {
        const Object* obj = getObject(i);
        if (const Ship* sh = dynamic_cast<const Ship*>(obj)) {
            if (gi.getShipRelation(*sh) == GameInterface::Playable) {
                if (gi.copyShipToGame(*sh)) {
                    ++st.succeeded;
                } else {
                    ++st.failed;
                }
            }
        } else if (const Planet* pl = dynamic_cast<const Planet*>(obj)) {
            if (gi.getPlanetRelation(*pl) == GameInterface::Playable) {
                if (gi.copyPlanetToGame(*pl)) {
                    ++st.succeeded;
                } else {
                    ++st.failed;
                }
            }
        } else {
            // what?
        }
    }
    return st;
}

// Copy from game using a GameInterface, all units.
game::sim::Setup::Status
game::sim::Setup::copyFromGame(const GameInterface& gi)
{
    return copyFromGame(gi, 0, getNumObjects());
}

// Copy from game using a GameInterface, range.
game::sim::Setup::Status
game::sim::Setup::copyFromGame(const GameInterface& gi, size_t from, size_t to)
{
    // ex WSimScreen::updateFromGame (part), ccsim.pas:UpdateFromGame
    // @change PCC1 works entirely different here: nonexistant units are deleted or given to aliens
    Status st(0, 0);
    for (size_t i = from; i < to; ++i) {
        Object* obj = getObject(i);
        if (Ship* sh = dynamic_cast<Ship*>(obj)) {
            if (gi.getShipRelation(*sh) != GameInterface::Unknown) {
                if (gi.copyShipFromGame(*sh)) {
                    ++st.succeeded;
                } else {
                    ++st.failed;
                }
            }
        } else if (Planet* pl = dynamic_cast<Planet*>(obj)) {
            if (gi.getPlanetRelation(*pl) != GameInterface::Unknown) {
                if (gi.copyPlanetFromGame(*pl)) {
                    ++st.succeeded;
                } else {
                    ++st.failed;
                }
            }
        } else {
            // what?
        }
    }
    return st;
}
