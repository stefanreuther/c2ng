/**
  *  \file game/map/fleetmember.cpp
  *  \brief Class game::map::FleetMember
  *
  *  FIXME: reconsider how preconditions are checked in this module.
  *  This does not check whether a manipulator function actually deals with a playable ship.
  */

#include "game/map/fleetmember.hpp"
#include "afl/string/format.hpp"
#include "game/map/configuration.hpp"
#include "game/map/fleet.hpp"
#include "game/map/ship.hpp"
#include "game/map/shiputils.hpp"
#include "game/map/universe.hpp"

using game::spec::Mission;
using game::Id_t;

namespace {

    /** Remove member from a fleet.
        \param univ Universe to work on
        \param sid Ship Id
        \return Id of fleet this ship was removed from (new Id if fleet Id changed) */
    Id_t removeFleetMember(game::map::Universe& univ, const Id_t sid)
    {
        // ex fleet.pas:LeaveFleet
        game::map::Ship* pShip = univ.ships().get(sid);
        if (!pShip) {
            return 0;
        }
        const Id_t fid = pShip->getFleetNumber();

        if (fid != sid) {
            // Member
            pShip->setFleetNumber(0);
            return fid;
        } else {
            // Leader. This means we have to find a new leader.
            Id_t newfid = 0;
            for (Id_t i = 1, n = univ.ships().size(); i <= n; ++i) {
                if (i != sid) {
                    if (game::map::Ship* p = univ.ships().get(i)) {
                        if (p->isPlayable(game::map::Object::Playable) && p->getFleetNumber() == fid) {
                            newfid = i;
                            break;
                        }
                    }
                }
            }
            if (newfid != 0) {
                univ.ships().get(newfid)->setFleetName(pShip->getFleetName());
                univ.ships().get(newfid)->setFleetNumber(newfid);
            }
            pShip->setFleetName(String_t());
            pShip->setFleetNumber(0);

            // Move all members.
            // PCC2 comment: This does not check isPlayable(), thus also moves nonexistant members, which are cleaned up by postprocessFleet().
            for (Id_t i = 1, n = univ.ships().size(); i <= n; ++i) {
                if (game::map::Ship* p = univ.ships().get(i)) {
                    if (p->getFleetNumber() == fid) {
                        p->setFleetNumber(newfid);
                    }
                }
            }
            return newfid;
        }
    }

    /** Process towee of a ship. */
    void synchronizeTowee(game::map::Universe& univ, const Id_t sid,
                          const game::map::Configuration& mapConfig,
                          const game::config::HostConfiguration& config,
                          const game::spec::ShipList& shipList)
    {
        game::map::Ship* pShip = univ.ships().get(sid);
        if (pShip != 0 && pShip->getMission().isSame(game::spec::Mission::msn_Tow)) {
            int tow = pShip->getMissionParameter(game::TowParameter).orElse(0);
            game::map::Ship* towedShip = univ.ships().get(tow);
            if (towedShip != 0 && towedShip->isPlayable(game::map::Object::Playable)) {
                game::map::Fleet::synchronizeFleetMember(univ, tow, mapConfig, config, shipList);
            }
        }
    }

    /** Leave a fleet. Makes the specified ship leave the fleet. Handles all cases
        (in particular, the case of the leader leaving the fleet).
        \param univ Universe to work on
        \param sid Ship Id */
    void leaveFleet(game::map::Universe& univ, const Id_t sid,
                    const game::map::Configuration& mapConfig,
                    const game::config::HostConfiguration& config,
                    const game::spec::ShipList& shipList)
    {
        game::map::Ship* pShip = univ.ships().get(sid);
        if (!pShip) {
            return;
        }

        const Id_t oldfid = pShip->getFleetNumber();
        if (oldfid != 0) {
            // Remove
            const Id_t fid = removeFleetMember(univ, sid);
            univ.fleets().handleFleetChange(fid);

            // If we are towing a fleet member, they may now get free
            synchronizeTowee(univ, sid, mapConfig, config, shipList);

            // Notify
            pShip->markDirty();
            if (fid != 0) {
                if (game::map::Ship* p = univ.ships().get(fid)) {
                    if (fid != oldfid) {
                        // Fleet Id changed, so notify whole fleet
                        game::map::Fleet(univ, *p).markDirty();
                    } else {
                        // Notify just the leader
                        p->markDirty();
                    }
                }
            }
        }
    }
}


// Constructor.
game::map::FleetMember::FleetMember(Universe& univ, Ship& ship, const Configuration& mapConfig)
    : m_universe(univ),
      m_ship(ship),
      m_mapConfig(mapConfig)
{ }

// Set fleet number.
bool
game::map::FleetMember::setFleetNumber(Id_t nfid,
                                       const game::config::HostConfiguration& config,
                                       const game::spec::ShipList& shipList)
{
    // ex game/fleet.h:setFleetNumber
    const Id_t fid = m_ship.getFleetNumber();
    if (nfid == fid) {
        // no change, ok
        return true;
    } else if (nfid == 0) {
        // leave fleet
        leaveFleet(m_universe, m_ship.getId(), m_mapConfig, config, shipList);
        return true;
    } else {
        game::map::Ship* nsh = m_universe.ships().get(nfid);
        int thisOwner, newOwner;
        if (nsh != 0
            && nsh->isPlayable(game::map::Object::Playable)
            && m_ship.getOwner(thisOwner)
            && nsh->getOwner(newOwner)
            && thisOwner == newOwner)
        {
            // enter or change fleet
            if (nfid == m_ship.getId()) {
                // start new fleet
                removeFleetMember(m_universe, m_ship.getId());
                Fleet(m_universe, m_ship).markDirty();           // FIXME: needed?
                m_ship.setFleetNumber(nfid);
                m_universe.fleets().handleFleetChange(nfid);
                m_ship.markDirty();                              // FIXME: needed?
                synchronizeTowee(m_universe, m_ship.getId(), m_mapConfig, config, shipList);
                return true;
            } else if (nsh->getFleetNumber() == nfid) {
                // join a fleet
                removeFleetMember(m_universe, m_ship.getId());
                Fleet(m_universe, m_ship).markDirty();           // FIXME: needed?
                m_ship.setFleetNumber(nfid);
                Fleet::synchronizeFleetMember(m_universe, m_ship.getId(), m_mapConfig, config, shipList);
                Fleet(m_universe, *nsh).markDirty();             // FIXME: needed?
                m_universe.fleets().handleFleetChange(nfid);
                m_ship.markDirty();                              // FIXME: needed?
                nsh->markDirty();                                // FIXME: needed?
                synchronizeTowee(m_universe, m_ship.getId(), m_mapConfig, config, shipList);
                return true;
            } else {
                // invalid: not a fleet leader
                return false;
            }
        } else {
            // invalid
            return false;
        }
    }
}

// Set fleet name.
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

// Set fleet waypoint.
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
                m_ship.setWaypoint(m_mapConfig.getSimpleNearestAlias(pt, position));
            }

            // Cancel intercept, if any
            const Mission*const msn = getShipMission(m_ship, config, shipList.missions());
            if (msn != 0 && msn->hasFlag(Mission::WaypointMission)) {
                m_ship.setMission(Mission::msn_Explore, 0, 0);
            }

            // Distribute change
            if (m_ship.isFleetLeader()) {
                Fleet(m_universe, m_ship).synchronize(config, shipList, m_mapConfig);
            }
        }
        return true;
    }
}

// Set fleet warp factor.
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
            Fleet(m_universe, m_ship).synchronize(config, shipList, m_mapConfig);
        }
        return true;
    }
}

// Set fleet mission.
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
        setInterceptWaypoint(m_universe, m_ship, m_mapConfig);
    }

    // Propagate to members
    if (m_ship.isFleetLeader()) {
        Fleet(m_universe, m_ship).synchronize(config, shipList, m_mapConfig);
    }

    // Postprocess tow
    if (oldMission == Mission::msn_Tow) {
        if (Ship* oldTowee = m_universe.ships().get(oldTow)) {
            if (oldTowee->getFleetNumber() == m_ship.getFleetNumber()) {
                Fleet::synchronizeFleetMember(m_universe, oldTow, m_mapConfig, config, shipList);
            }
        }
    }
    if (m == Mission::msn_Tow) {
        if (Ship* newTowee = m_universe.ships().get(t)) {
            if (newTowee->getFleetNumber() == m_ship.getFleetNumber()) {
                Fleet::synchronizeFleetMember(m_universe, t, m_mapConfig, config, shipList);
            }
        }
    }

    return true;
}

// Check for locked mission.
bool
game::map::FleetMember::isMissionLocked(int flags,
                                        const game::config::HostConfiguration& config,
                                        const game::spec::ShipList& shipList,
                                        const interpreter::MutexList& mtxl) const
{
    // ex game/fleet.h:isMissionLocked
    // Is it an intercept mission after all?
    const Mission*const m = getShipMission(m_ship, config, shipList.missions());
    if (m == 0 || !m->hasFlag(Mission::WaypointMission)) {
        return false;
    }

    // Is it locked because we're a fleet member?
    if (m_ship.getFleetNumber() != 0) {
        if (!m_ship.isFleetLeader() || (flags & AcceptLeaders) == 0) {
            return true;
        }
    }

    // Is it locked because someone has the waypoint mutex?
    if ((flags & OverrideLocks) == 0 && mtxl.query(afl::string::Format("S%d.WAYPOINT", m_ship.getId())) != 0) {
        return true;
    }

    // Not locked
    return false;
}
