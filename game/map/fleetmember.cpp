/**
  *  \file game/map/fleetmember.cpp
  */

#include "game/map/fleetmember.hpp"
#include "game/map/fleet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/map/shiputils.hpp"

using game::spec::Mission;

game::map::FleetMember::FleetMember(Universe& univ, Ship& ship)
    : m_universe(univ),
      m_ship(ship)
{ }

// bool
// game::map::FleetMember::setFleetNumber(int nfid)
// {
// }

// /** Set name of a fleet.
//     \param univ Universe to work on
//     \param sid Ship or Fleet Id, must be Id of existing ship
//     \param nname New name
//     \retval true success
//     \retval false error, %sid was not a valid fleet id */
bool
game::map::FleetMember::setFleetName(String_t nname)
{
    // ex game/fleet.h:setFleetName
    if (m_ship.isFleetLeader()) {
        m_ship.setFleetName(nname);
        Fleet(m_universe, m_ship).markDirty();
        return true;
    } else {
        return false;
    }

}

// /** Set waypoint of a fleet.
//     \param univ Universe to work on
//     \param sid Ship Id
//     \param pt Desired waypoint
//     \retval true success, waypoint of fleet or lone ship changed
//     \retval false failure, this was a fleet member */
bool
game::map::FleetMember::setWaypoint(Point pt,
                                    const game::config::HostConfiguration& config,
                                    const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:setFleetWaypoint
    if (m_ship.isFleetMember()) {
        // Fleet member cannot change waypoint
        return false;
    } else {
        if (!m_ship.getWaypoint().isSame(pt)) {
            // Set waypoint
            Point position;
            if (m_ship.getPosition(position)) {
                m_ship.setWaypoint(m_universe.config().getSimpleNearestAlias(pt, position));
            }

            // Cancel intercept, if any
            const Mission*const msn = getShipMission(m_ship, config, shipList.missions());
            if (msn != 0 && msn->hasFlag(Mission::WaypointMission)) {
                m_ship.setMission(Mission::msn_Explore, 0, 0);
            }

            // Distribute change
            if (m_ship.isFleetLeader()) {
                Fleet(m_universe, m_ship).synchronize(config, shipList);
            }
        }
        return true;
    }
}

// /** Set speed of a fleet.
//     \param univ Universe to work on
//     \param sid Ship Id
//     \param speed Desired speed
//     \retval true success, speed of fleet or lone ship changed
//     \retval false failure, this was a fleet member */
bool
game::map::FleetMember::setWarpFactor(int speed,
                                      const game::config::HostConfiguration& config,
                                      const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:setFleetSpeed
    if (m_ship.isFleetMember()) {
        return false;
    } else {
        m_ship.setWarpFactor(speed);
        if (m_ship.isFleetLeader()) {
            Fleet(m_universe, m_ship).synchronize(config, shipList);
        }
        return true;
    }
}

// /** Set mission of a fleet or member.
//     \param univ Universe to work on
//     \param sid Ship Id
//     \param m,i,t Desired mission
//     \retval true success, speed of fleet, member, or lone ship changed
//     \retval false failure, this was a fleet member and this fleet was intercepting */
bool
game::map::FleetMember::setMission(int m, int i, int t,
                                   const game::config::HostConfiguration& config,
                                   const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:setFleetMission

    // Is this a change after all?
    if (m_ship.getMission().isSame(m)
        && m_ship.getMissionParameter(InterceptParameter).isSame(i)
        && m_ship.getMissionParameter(TowParameter).isSame(t))
    {
        return true;
    }

    // New mission
    const Mission*const msn = getShipMissionByNumber(m, m_ship, config, shipList.missions());
    const bool xcept = (msn != 0 && msn->hasFlag(Mission::WaypointMission));

    // Check for intercept
    if (m_ship.isFleetMember()) {
        // Fail if old leader has intercept, or new mission is intercept.
        // Those are not allowed on members.
        if (xcept) {
            return false;
        }

        if (const Ship* leader = m_universe.ships().get(m_ship.getFleetNumber())) {
            if (const Mission* leaderMission = getShipMission(*leader, config, shipList.missions())) {
                if (leaderMission->hasFlag(Mission::WaypointMission)) {
                    return false;
                }
            }
        }
    }

    // Remember old mission
    int oldMission = m_ship.getMission().orElse(0);
    int oldTow     = m_ship.getMissionParameter(TowParameter).orElse(0);

    // Valid change
    m_ship.setMission(m, i, t);

    // Postprocess intercept
    if (xcept) {
        // Set waypoint to intercept target
        setInterceptWaypoint(m_universe, m_ship);

        // Propagate to members
        if (m_ship.isFleetLeader()) {
            Fleet(m_universe, m_ship).synchronize(config, shipList);
        }
    }

    // Postprocess tow
    if (oldMission == Mission::msn_Tow) {
        if (Ship* oldTowee = m_universe.ships().get(oldTow)) {
            if (oldTowee->getFleetNumber() == m_ship.getId()) {
                Fleet::synchronizeFleetMember(m_universe, oldTow, config, shipList);
            }
        }
    }
    if (m == Mission::msn_Tow) {
        if (Ship* newTowee = m_universe.ships().get(t)) {
            if (newTowee->getFleetNumber() == m_ship.getId()) {
                Fleet::synchronizeFleetMember(m_universe, t, config, shipList);
            }
        }
    }

    return true;
}
