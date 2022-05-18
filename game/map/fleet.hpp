/**
  *  \file game/map/fleet.hpp
  */
#ifndef C2NG_GAME_MAP_FLEET_HPP
#define C2NG_GAME_MAP_FLEET_HPP

#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/unitscoredefinitionlist.hpp"

namespace game { namespace map {

    class Configuration;
    class Universe;
    class Ship;

    class Fleet {
     public:
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

        String_t getTitle(afl::string::Translator& tx) const;

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

        static String_t getTitle(const Ship& ship, afl::string::Translator& tx);

     private:
        Universe& m_universe;
        Ship& m_ship;
    };

} }

#endif
