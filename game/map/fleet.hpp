/**
  *  \file game/map/fleet.hpp
  */
#ifndef C2NG_GAME_MAP_FLEET_HPP
#define C2NG_GAME_MAP_FLEET_HPP

#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/interpreterinterface.hpp"

namespace game { namespace map {

    class Universe;
    class Ship;

    class Fleet {
     public:
        Fleet(Universe& univ, Ship& ship);

        void markDirty();

        void synchronize(const game::config::HostConfiguration& config,
                         const game::spec::ShipList& shipList);

        bool hasSpecialFunction(int basicFunction,
                                const UnitScoreDefinitionList& scoreDefinitions,
                                const game::spec::ShipList& shipList,
                                const game::config::HostConfiguration& config) const;

        String_t getTitle(afl::string::Translator& tx, InterpreterInterface& iface) const;

        static void synchronizeFleetMember(Universe& univ, Id_t sid,
                                           const game::config::HostConfiguration& config,
                                           const game::spec::ShipList& shipList);

        static String_t getTitle(const Ship& ship, afl::string::Translator& tx, InterpreterInterface& iface);

     private:
        Universe& m_universe;
        Ship& m_ship;
    };

} }

#endif
