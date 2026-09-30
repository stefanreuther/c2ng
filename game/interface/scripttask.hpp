/**
  *  \file game/interface/scripttask.hpp
  *  \brief Interface game::interface::ScriptTask
  */
#ifndef C2NG_GAME_INTERFACE_SCRIPTTASK_HPP
#define C2NG_GAME_INTERFACE_SCRIPTTASK_HPP

#include "game/session.hpp"

namespace game { namespace interface {

    /** Interface for defining a script task.
        A script task executes in a process group, on a session.
        This interface provides a factory method for such tasks,
        to provide a way to implement re-usable tasks.
        A script integration will then provide a way to run such tasks. */
    class ScriptTask {
     public:
        virtual ~ScriptTask()
            { }

        /** Execute task.
            @param pgid    [in] Process group Id (supplied by caller).
                           This method shall create/obtain processes and place them in this group.
            @param session [in] Session */
        virtual void execute(uint32_t pgid, Session& session) = 0;
    };

} }

#endif
