/**
  *  \file game/map/fleetmember.hpp
  */
#ifndef C2NG_GAME_MAP_FLEETMEMBER_HPP
#define C2NG_GAME_MAP_FLEETMEMBER_HPP

#include "game/map/point.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace map {

    class Universe;
    class Ship;

    class FleetMember {
     public:
        FleetMember(Universe& univ, Ship& ship);

        // bool setFleetNumber(int nfid);
        bool setFleetName(String_t nname);
        bool setWaypoint(Point pt,
                         const game::config::HostConfiguration& config,
                         const game::spec::ShipList& shipList);
        bool setWarpFactor(int speed,
                           const game::config::HostConfiguration& config,
                           const game::spec::ShipList& shipList);
        bool setMission(int m, int i, int t,
                        const game::config::HostConfiguration& config,
                        const game::spec::ShipList& shipList);

     private:
        Universe& m_universe;
        Ship& m_ship;
    };

} }

#endif
