/**
  *  \file game/map/fleetmember.hpp
  *  \brief Class game::map::FleetMember
  */
#ifndef C2NG_GAME_MAP_FLEETMEMBER_HPP
#define C2NG_GAME_MAP_FLEETMEMBER_HPP

#include "game/map/point.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/mutexlist.hpp"

namespace game { namespace map {

    class Universe;
    class Ship;

    /** Operations on Fleet Members.
        This class provides operations on ships in their role as a possible fleet member.
        Methods of this class should be used instead of like-named operations of Ship when fleets should be supported.
        All operations work on fleet leaders, fleet members, and non-fleet-members.
        Operations can be declined if not valid without violating fleet invariants. */
    class FleetMember {
     public:
        /** Constructor.
            \param univ Universe
            \param ship Ship */
        FleetMember(Universe& univ, Ship& ship);

        /** Set fleet number.
            This gets a ship into and out of a fleet.
            \param nfid     New fleet Id. Can be 0 (leave fleet), same as ship Id (make new fleet), or Id of an existing fleet.
            \param config   Host configuration (needed to parse missions)
            \param shipList Ship list (needed to parse missions)
            \retval true success
            \retval false operation declined (invalid nfid) */
        bool setFleetNumber(Id_t nfid,
                            const game::config::HostConfiguration& config,
                            const game::spec::ShipList& shipList);

        /** Set fleet name.
            This can be called for fleet leaders only.
            \param nname New name
            \retval true success
            \retval false operation declined (ship is not a fleet leader). */
        bool setFleetName(String_t nname);

        /** Set fleet waypoint.
            This can be called for fleet leaders and single ships.
            Setting a waypoint cancels an Intercept mission.
            \param pt       New waypoint
            \param config   Host configuration (needed to parse missions)
            \param shipList Ship list (needed to parse missions)
            \retval true success
            \retval false operation declined (ship is a fleet member) */
        bool setWaypoint(Point pt,
                         const game::config::HostConfiguration& config,
                         const game::spec::ShipList& shipList);

        /** Set fleet warp factor.
            This can be called for fleet leaders and single ships.
            \param speed    New warp factor
            \param config   Host configuration (needed to parse missions)
            \param shipList Ship list (needed to parse missions)
            \retval true success
            \retval false operation declined (ship is a fleet member) */
        bool setWarpFactor(int speed,
                           const game::config::HostConfiguration& config,
                           const game::spec::ShipList& shipList);

        /** Set fleet mission.
            This can be called for fleet leaders and single ships.
            It can also be called for fleet members if the fleet does not intercept a ship.
            \param m        Mission number
            \param i        Intercept parameter
            \param t        Tow parameter
            \param shipList Ship list (needed to parse missions)
            \retval true success
            \retval false operation declined */
        bool setMission(int m, int i, int t,
                        const game::config::HostConfiguration& config,
                        const game::spec::ShipList& shipList);

        static const int AcceptLeaders = 1;  // ex mlAcceptLeaders
        static const int OverrideLocks = 2;  // ex mlOverrideLocks

        /** Check for locked mission.
            A mission can be locked (=prevented from change) if it is an intercept mission that is required
            - by the fleet
            - by script mutexes
            \param flags    Flags
                            - AcceptLeaders: accept (=return false) if this is a fleet leader
                            - OverrideLocks: accept (=return false) even if there is a script mutex on it
            \param config   Host configuration (needed to parse missions)
            \param shipList Ship list (needed to parse missions)
            \retval true Mission is locked and should not be changed
            \retval false Mission is not locked and can be changed */
        bool isMissionLocked(int flags,
                             const game::config::HostConfiguration& config,
                             const game::spec::ShipList& shipList,
                             const interpreter::MutexList& mtxl) const;

     private:
        Universe& m_universe;
        Ship& m_ship;
    };

} }

#endif
