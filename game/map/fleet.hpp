/**
  *  \file game/map/fleet.hpp
  *  \brief Class game::map::Fleet
  */
#ifndef C2NG_GAME_MAP_FLEET_HPP
#define C2NG_GAME_MAP_FLEET_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Configuration;
    class Ship;
    class Universe;

    /** Fleet operations.
        Provides operations on entire fleets.
        This is intended to be used as a temporary object. */
    class Fleet {
     public:
        /** Constructor.
            \param univ Universe
            \param ship Fleet leader ship */
        Fleet(Universe& univ, Ship& ship);

        /** Mark a fleet dirty.
            Marks all fleet members dirty. */
        void markDirty();

        /** Synchronize a fleet.
            Synchronizes all waypoints of the fleet.
            \param config Host configuration
            \param shipList Ship list
            \param mapConfig Map configuration (for starchart geometry) */
        void synchronize(const game::config::HostConfiguration& config,
                         const game::spec::ShipList& shipList,
                         const Configuration& mapConfig);

        /** Check whether the fleet can do a particular special function.
            \param basicFunction Function to check
            \param scoreDefinitions Ship score definitions
            \param shipList         Ship list
            \param config           Host configuration
            \retval true all fleet members can do this function
            \retval false not all fleet members can do this function */
        bool hasSpecialFunction(int basicFunction,
                                const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const game::config::HostConfiguration& config) const;

        /** Get title of this fleet.
            \param tx Translator
            \return title, never empty */
        String_t getTitle(afl::string::Translator& tx) const;

        /** Count number of fleet members.
            \return number of fleet members */
        int countFleetMembers() const;

        /** Synchronize a fleet member.
            Synchronizes waypoint of a single fleet member with its leader, if any.
            Does nothing if the ship is not a fleet member.
            \param univ Universe to work on
            \param sid  Ship Id
            \param mapConfig Map configuration (for starchart geometry)
            \param config Host configuration (for missions)
            \param shipList Ship list (for missions) */
        static void synchronizeFleetMember(Universe& univ, Id_t sid,
                                           const Configuration& mapConfig,
                                           const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList);

        /** Get title of a fleet led by a given ship.
            Same as Fleet(univ, ship).getTitle(tx), but does not require a Fleet object.
            \param ship Fleet leader
            \param tx   Translator */
        static String_t getTitle(const Ship& ship, afl::string::Translator& tx);

     private:
        Universe& m_universe;
        Ship& m_ship;
    };

} }

#endif
