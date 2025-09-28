/**
  *  \file game/interface/taskwaypoints.hpp
  *  \brief Class game::interface::TaskWaypoints
  */
#ifndef C2NG_GAME_INTERFACE_TASKWAYPOINTS_HPP
#define C2NG_GAME_INTERFACE_TASKWAYPOINTS_HPP

#include <vector>
#include "afl/base/signalconnection.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/extra.hpp"
#include "game/map/point.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "interpreter/process.hpp"

namespace game { namespace interface {

    /** Auto task waypoints.
        TaskWaypoints can be added to a Session as an extra.
        If the session contains any ship auto tasks with movement orders,
        it will prepare a list of those waypoints (Track) for these ships.

        Each track will be updated:
        - when an auto task changes state or terminates,
          in particular, after it is run by turn loading or editing
        - when a game is connected or disconnected

        Normal operation is to create the TaskWaypoints as a session extra (TaskWaypoints::create()) to make it available,
        and use TaskWaypoints::get() to obtain the instance; everything else is automatic. */
    class TaskWaypoints : public Extra {
     public:
        /** Information about one ship's movement. */
        struct Track {
            /** List of waypoints. */
            std::vector<game::map::Point> waypoints;
        };

        /** Constructor.
            @param session Session
            @see TaskWaypoints::create */
        explicit TaskWaypoints(Session& session);

        /** Destructor. */
        ~TaskWaypoints();

        /** Get information about one ship's waypoint.
            @param id Ship Id
            @return No information available for given Id (invalid Id, or ship has no appropriate task) */
        const Track* getTrack(Id_t id) const;

        /** Update information for all ship tasks.
            Normally called automatically; public for testing. */
        void updateAll();

        /** Update information for one task.
            If the given task is a ship task, rebuilds the waypoint information.
            If there is a change, signals a change to current turn's universe,
            to have the map redraw.

            This function is normally called automatically; public for testing.

            @param proc        Process
            @param willDelete  true if process is about to be deleted */
        void updateProcess(const interpreter::Process& proc, bool willDelete);

        /** Create TaskWaypoints object.
            If the session already has a TaskWaypoints extra, returns that; otherwise, creates one.
            @param session Session
            @return TaskWaypoints */
        static TaskWaypoints& create(Session& session);

        /** Get TaskWaypoints object.
            If the session has a TaskWaypoints extra, returns it; otherwise, return null.
            @param session Session
            @return TaskWaypoints, if any */
        static TaskWaypoints* get(Session& session);

     private:
        Session& m_session;
        afl::container::PtrVector<Track> m_data;
        afl::base::SignalConnection conn_processStateChanged;
        afl::base::SignalConnection conn_connectionChange;
    };

} }

#endif
