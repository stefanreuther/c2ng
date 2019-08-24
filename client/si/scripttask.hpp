/**
  *  \file client/si/scripttask.hpp
  */
#ifndef C2NG_CLIENT_SI_SCRIPTTASK_HPP
#define C2NG_CLIENT_SI_SCRIPTTASK_HPP

#include "game/session.hpp"

namespace client { namespace si {

    class ScriptTask {
     public:
        virtual ~ScriptTask()
            { }

        /** Execute task.
            \param pgid    [in] Process group Id (supplied by caller). Place your processes in this group.
            \param session [in] Session */
        virtual void execute(uint32_t pgid, game::Session& session) = 0;
    };

} }

#endif
