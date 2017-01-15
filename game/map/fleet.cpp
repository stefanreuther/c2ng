/**
  *  \file game/map/fleet.cpp
  */

#include "game/map/fleet.hpp"
#include "game/map/anyshiptype.hpp"
#include "game/map/ship.hpp"
#include "game/spec/missionlist.hpp"
#include "game/spec/mission.hpp"
#include "game/playerset.hpp"
#include "game/map/universe.hpp"
#include "game/map/shiputils.hpp"

using game::spec::Mission;

namespace {
    /** Manipulating: check whether fleet member is being towed. */
    bool isFleetMemberTowed(game::map::Universe& univ, const game::map::Ship& ship)
    {
        const game::Id_t fid = ship.getFleetNumber();
        if (fid != 0) {
            game::map::AnyShipType ships(univ);
            for (game::Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
                if (const game::map::Ship* other = ships.getObjectByIndex(i)) {
                    if (i != ship.getId() && other->getFleetNumber() == fid) {
                        if (other->getMission().isSame(Mission::msn_Tow)
                            && other->getMissionParameter(game::TowParameter).isSame(ship.getId()))
                        {
                            return true;
                        }
                    }
                }
            }
        }

        return false;
    }

}

game::map::Fleet::Fleet(Universe& univ, Ship& ship)
    : m_universe(univ),
      m_ship(ship)
{ }

// /** Mark a fleet dirty. Marks all fleet members dirty.
//     \param univ Universe to work on
//     \param fid Fleet Id */
void
game::map::Fleet::markDirty()
{
    // ex game/fleet.h:markFleetDirty
    AnyShipType ships(m_universe);
    const int fleetNumber = m_ship.getId();
    for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
        if (Ship* sh = ships.getObjectByIndex(i)) {
            if (sh->getFleetNumber() == fleetNumber) {
                sh->markDirty();
            }
        }
    }
}

// /** Synchronize a fleet. Synchronizes all waypoints of the fleet.
//     \param univ Universe to work on
//     \param fid Fleet Id */
void
game::map::Fleet::synchronize(const game::config::HostConfiguration& config,
                              const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:synchronizeFleet
    AnyShipType ships(m_universe);
    const int fleetNumber = m_ship.getId();
    for (Id_t i = ships.getNextIndex(0); i != 0; i = ships.getNextIndex(i)) {
        if (Ship* sh = ships.getObjectByIndex(i)) {
            if (sh->getFleetNumber() == fleetNumber) {
                synchronizeFleetMember(m_universe, i, config, shipList);
            }
        }
    }
}


// /** Synchronize a fleet member. Synchronizes waypoint of a single fleet member.
//     \param univ Universe to work on
//     \param sid Ship Id */
// FIXME: I'm not entirely happy with the placement of this function.
// It would make sense to put this into FleetMember, but then we'd have a call chain FleetMember -> Fleet -> FleetMember.
void
game::map::Fleet::synchronizeFleetMember(Universe& univ, Id_t sid,
                                         const game::config::HostConfiguration& config,
                                         const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:synchronizeFleetMember
    const game::spec::MissionList& missions = shipList.missions();

    // Fetch ship
    Ship* sh = univ.ships().get(sid);
    if (sh == 0) {
        // Ship does not exist? Error, ignore.
    } else if (sh->getFleetNumber() == 0) {
        // Ship is not member of a fleet, ignore.
    } else if (sh->getFleetNumber() == sid) {
        // I am the leader, so nothing to do.
    } else {
        // I am a member. Fetch the leader.
        if (const Ship* leader = univ.ships().get(sh->getFleetNumber())) {
            // Check for intercept mission
            const Mission* msn = getShipMission(*leader, config, missions);
            const bool xcept = (msn != 0 && msn->getFlags().contains(Mission::WaypointMission));

            if (xcept) {
                // We're intercepting
                if (leader->getMissionParameter(game::InterceptParameter).isSame(sid)) {
                    // Fleet is intercepting this very ship.
                    sh->setMission(Mission::msn_Explore, 0, 0);
                    sh->clearWaypoint();

                    int engineType;
                    if (sh->getEngineType().get(engineType)) {
                        if (game::spec::Engine* engine = shipList.engines().get(engineType)) {
                            sh->setWarpFactor(engine->getMaxEfficientWarp());
                        }
                    }
                } else {
                    // Fleet is intercepting someone else.
                    sh->setMission(leader->getMission(),
                                   leader->getMissionParameter(InterceptParameter),
                                   leader->getMissionParameter(TowParameter));
                    setInterceptWaypoint(univ, *sh);
                    sh->setWarpFactor(leader->getWarpFactor());
                }
            } else {
                // We're moving normally
                if (isFleetMemberTowed(univ, *sh)) {
                    // Member is under tow, so avoid that it escapes.
                    sh->setWarpFactor(0);
                    sh->clearWaypoint();
                } else {
                    // Normal move
                    sh->setWarpFactor(leader->getWarpFactor());

                    Point shipPos, leaderPos;
                    if (sh->getPosition(shipPos) && leader->getWaypoint().get(leaderPos)) {
                        sh->setWaypoint(univ.config().getSimpleNearestAlias(leaderPos, shipPos));
                    }
                }

                // Cancel any intercept
                const Mission* shipMission = getShipMission(*sh, config, missions);
                if (shipMission != 0 && shipMission->getFlags().contains(Mission::WaypointMission)) {
                    sh->setMission(Mission::msn_Explore, 0, 0);
                }
            }
        } else {
            // Leader does not exist? Error, ignore.
        }
    }        
}
