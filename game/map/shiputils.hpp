/**
  *  \file game/map/shiputils.hpp
  */
#ifndef C2NG_GAME_MAP_SHIPUTILS_HPP
#define C2NG_GAME_MAP_SHIPUTILS_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/friendlycodelist.hpp"
#include "game/spec/missionlist.hpp"
#include "game/spec/mission.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/cargocontainer.hpp"
#include "game/element.hpp"

namespace game { namespace map {

    class Ship;
    class Planet;
    class Universe;

    const game::spec::Mission* getShipMission(const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions);
    const game::spec::Mission* getShipMissionByNumber(int nr, const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions);

    void setInterceptWaypoint(Universe& univ, Ship& sh);
    void cancelAllCloneOrders(Universe& univ, const Planet& pl, const game::spec::FriendlyCodeList& list, util::RandomNumberGenerator& rng);

    const game::spec::Hull* getShipHull(const Ship& ship, const game::spec::ShipList& shipList);
    int32_t getShipTransferMaxCargo(const CargoContainer& cont, Element::Type type, const Ship& ship, const game::spec::ShipList& shipList);

} }

#endif
