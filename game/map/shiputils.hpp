/**
  *  \file game/map/shiputils.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPUTILS_HPP
#define C2NG_GAME_MAP_SHIPUTILS_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/missionlist.hpp"
#include "game/spec/mission.hpp"

namespace game { namespace map {

    class Ship;
    class Universe;

    const game::spec::Mission* getShipMission(const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions);
    const game::spec::Mission* getShipMissionByNumber(int nr, const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions);

    void setInterceptWaypoint(Universe& univ, Ship& sh);

} }

#endif
