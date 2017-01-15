/**
  *  \file game/sim/setup.hpp
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

    // /*! \class GSimState
    //   \brief Simulator State

    //   This object carries a simulation set-up. Users can edit these
    //   objects; it provides methods for change tracking. In addition, the
    //   actual simulator operates on GSimState objects.

    //   GSimState can contain a number of ships and up to one planet. */
    class Setup {
     public:
        typedef size_t Slot_t;

        Setup();
        Setup(const Setup& other);
        ~Setup();

        Setup& operator=(const Setup& other);

        // Planet operations
        Planet* addPlanet();
        bool hasPlanet() const;
        const Planet* getPlanet() const;
        Planet* getPlanet();
        void removePlanet();

        // Ship operations
        Ship* addShip();
        Slot_t getNumShips() const;
        const Ship* getShip(Slot_t slot) const;
        Ship* getShip(Slot_t slot);
        void removeShip(Slot_t slot);

        // Object operations
        Slot_t getNumObjects() const;
        Object* getObject(Slot_t slot);

        // Ship list operations
        void duplicateShip(Slot_t slot, Id_t newId, afl::string::Translator& tx);
        void swapShips(Slot_t a, Slot_t b);
        bool findIndex(const Ship* ship, Slot_t& result) const;
        bool findShipSlotById(Id_t id, Slot_t& result) const;
        Ship* findShipById(Id_t id);
        Id_t findUnusedShipId(Id_t firstToCheck) const;

        // Global operations
        void notifyListeners();
        void setRandomFriendlyCodes();
        bool isMatchingShipList(const game::spec::ShipList& shipList) const;

        afl::base::Signal<void()> sig_structureChange;
        afl::base::Signal<void()> sig_planetChange;
        afl::base::Signal<void(Slot_t)> sig_shipChange;

     private:
        afl::container::PtrVector<Ship> m_ships;
        std::auto_ptr<Planet> m_planet;
        bool m_structureChanged;
    };

} }

#endif
