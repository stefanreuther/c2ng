/**
  *  \file game/sim/setup.hpp
  *  \brief Class game::sim::Setup
  */
#ifndef C2NG_GAME_SIM_SETUP_HPP
#define C2NG_GAME_SIM_SETUP_HPP

#include <memory>
#include "afl/container/ptrvector.hpp"
#include "afl/base/signal.hpp"
#include "game/types.hpp"
#include "game/spec/shiplist.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace sim {

    class Ship;
    class Planet;
    class Object;

    /** Simulation setup/state.
        This object carries a simulation set-up.
        Users can edit these objects; it provides methods for change tracking.
        In addition, the actual simulator operates on Setup objects.

        Setup can contain a number of ships and up to one planet.
        Objects can be queried individually (planet (getPlanet()) and list-of-ships (getShip())),
        or as a virtual list with the planet at the end (getObject()).

        Changes are tracked internally; updates can be posted using notifyListeners().

        All contained objects are owned by the Setup. */
    class Setup {
     public:
        /** Slot number. */
        typedef size_t Slot_t;


        /** Construct empty list. */
        Setup();

        /** Copy a setup.
            \param other Origin */
        Setup(const Setup& other);

        /** Destructor. */
        ~Setup();

        /** Assign another setup.
            \param other Other setup */
        Setup& operator=(const Setup& other);

        /*
         *  Planet operations
         */

        /** Add planet.
            If the setup does not contain a planet, adds one; otherwise, no change.
            \return planet; null on failure (cannot currently happen) */
        Planet* addPlanet();

        /** Check presence of planet.
            \return true if setup has a planet */
        bool hasPlanet() const;

        /** Get planet.
            \return planet, if any; null if no planet */
        const Planet* getPlanet() const;

        /** Get planet.
            \return planet, if any; null if no planet */
        Planet* getPlanet();

        /** Remove planet.
            If the setup does not contain a planet, no change. */
        void removePlanet();

        /*
         *  Ship operations
         */

        /** Add a ship.
            \return ship; null on failure (cannot currently happen) */
        Ship* addShip();

        /** Get number of ships.
            \return number of ships */
        Slot_t getNumShips() const;

        /** Get ship, given a slot number.
            \param slot Slot [0,getNumShips())
            \return ship; null if slot is out of range */
        const Ship* getShip(Slot_t slot) const;

        /** Get ship, given a slot number.
            \param slot Slot [0,getNumShips())
            \return ship; null if slot is out of range */
        Ship* getShip(Slot_t slot);

        /** Remove ship, given a slot number.
            \param slot Slot [0,getNumShips()); call is ignored if this is out of range */
        void removeShip(Slot_t slot);

        /*
         *  Object operations
         */

        /** Get number of objects.
            This is the number of ships plus planets.
            \return number */
        Slot_t getNumObjects() const;

        /** Get object, given a slot number.
            \param slot Slot [0,getNumObjects())
            \return object; null if slot is out of range */
        const Object* getObject(Slot_t slot) const;

        /** Get object, given a slot number.
            \param slot Slot [0,getNumObjects())
            \return object; null if slot is out of range */
        Object* getObject(Slot_t slot);

        /** Find slot, given an object.
            \param [in] obj Object, as obtained from a call on this object
            \param [out] result Slot number such that getObject(result) == obj
            \retval true Object was found, \c result has been updated
            \retval false Object not found */
        bool findIndex(const Object* obj, Slot_t& result) const;

        /*
         *  Operations on the list of ships
         */

        /** Duplicate a ship.
            Creates a new ship as a duplicate of the ship at the given slot, and inserts it in the slot below.
            \param slot Slot number [0,getNumShips()); call is ignored if this is out of range
            \param newId Id number of the new ship
            \param tx Translator */
        void duplicateShip(Slot_t slot, Id_t newId, afl::string::Translator& tx);

        /** Swap two ships.
            \param a,b Slots [0,getNumShips()); call is ignored if either is out of range */
        void swapShips(Slot_t a, Slot_t b);

        /** Find ship slot, given an object.
            \param [in] ship Ship object, as obtained from a call on this object
            \param [out] result Slot number such that getShip(result) == ship
            \retval true Ship was found, \c result has been updated
            \retval false Ship not found */
        // FIXME: rename to findShipSlot?
        bool findIndex(const Ship* ship, Slot_t& result) const;

        /** Find ship slot, given an Id.
            \param [in] id Ship Id
            \param [out] result Slot number such that getShip(result)->getId() == id
            \retval true Ship was found, \c result has been updated
            \retval false Ship not found */
        bool findShipSlotById(Id_t id, Slot_t& result) const;

        /** Find ship, given an Id.
            \param id Ship Id
            \return ship such that result->getId() == id; null if none found */
        Ship* findShipById(Id_t id);

        /** Find unused ship Id.
            Attempts to find a ship id such that findShipById(id)==0.
            \param firstToCheck First Id to check.
                   When calling this function in a loop to allocate ships, pass the last allocated Id here.
                   Otherwise, the allocation loop will have O(n^3).
            \return unused ship Id */
        Id_t findUnusedShipId(Id_t firstToCheck) const;

        /** Merge from other setup.
            Objects not contained in this setup are added;
            if the other setup contains an object already in this setup (ship with same Id, planet), it is replaced.
            Caller must use notifyListeners() to notify changes.
            \param other Other setup */
        void merge(const Setup& other);

        /*
         *  Global operations
         */

        /** Notify listeners.
            If any changes have accumulated after the last call, calls the respective listeners. */
        void notifyListeners();

        /** Set random friendly codes.
            Calls setRandomFriendlyCodes() on all contained objects.
            This will assign random friendly codes to all objects that are configured to do so. */
        void setRandomFriendlyCodes();

        /** Check whether this setup matches a ship list.
            \param shipList ship list
            \retval true all ships are valid according to the ship list
            \retval false some ships are not valid with this ship list */
        bool isMatchingShipList(const game::spec::ShipList& shipList) const;

        /** Signal: structure change.
            Called after structural changes, i.e. objects moved, added, removed. */
        afl::base::Signal<void()> sig_structureChange;

        /** Signal: planet change.
            Called after change to the planet. */
        afl::base::Signal<void()> sig_planetChange;

        /** Signal: ship change.
            Called after change to a ship.
            \param slot slot number */
        afl::base::Signal<void(Slot_t)> sig_shipChange;

     private:
        afl::container::PtrVector<Ship> m_ships;
        std::auto_ptr<Planet> m_planet;
        bool m_structureChanged;
    };

} }

#endif
