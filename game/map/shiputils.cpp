/**
  *  \file game/map/shiputils.cpp
  */

#include "game/map/shiputils.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"

const game::spec::Mission*
game::map::getShipMission(const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions)
{
    int nr, owner;
    if (ship.getMission().get(nr) && ship.getRealOwner().get(owner)) {
        return missions.getMissionByNumber(nr, PlayerSet_t(config.getPlayerMissionNumber(owner)));
    } else {
        return 0;
    }
}

const game::spec::Mission*
game::map::getShipMissionByNumber(int nr, const Ship& ship, const game::config::HostConfiguration& config, const game::spec::MissionList& missions)
{
    int owner;
    if (ship.getRealOwner().get(owner)) {
        return missions.getMissionByNumber(nr, PlayerSet_t(config.getPlayerMissionNumber(owner)));
    } else {
        return 0;
    }
}

// /** Manipulating: set waypoint for intercepting ship. */
void
game::map::setInterceptWaypoint(Universe& univ, Ship& sh)
{
    // ex game/fleet.h:setInterceptWaypoint
    int i;
    if (sh.getMissionParameter(InterceptParameter).get(i)) {
        if (Ship* target = univ.ships().get(i)) {
            // FIXME: handle THost where intercept does not cross the seam?
            Point shipPos, targetPos;
            if (target->getPosition(targetPos) && sh.getPosition(shipPos)) {
                sh.setWaypoint(univ.config().getSimpleNearestAlias(targetPos, shipPos));
            }
        }
    }
}
