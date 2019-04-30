/**
  *  \file client/si/scripttask.hpp
  */
#ifndef C2NG_CLIENT_SI_SCRIPTTASK_HPP
#define C2NG_CLIENT_SI_SCRIPTTASK_HPP

#include "game/session.hpp"

namespace client { namespace si {

    class ScriptTask {
     public:
        enum Verbosity {
            Default,            // Default: no logging
            Verbose,            // Verbose: show non-standard states (verbose && !hasResult)
            Result              // Result: show final result (verbose && hasResult)
        };
        virtual ~ScriptTask()
            { }

        /** Execute task.
            \param pgid    [in] Process group Id (supplied by caller). Place your processes in this group.
            \param session [in] Session
            \param v       [out] Verbosity if anything other then default is desired
            \return created process. If returned, this process's state is returned to the caller.
            If not process is returned, caller receives a successful termination upon completion of the process group. */
        virtual interpreter::Process* execute(uint32_t pgid, game::Session& session, Verbosity& v) = 0;
    };

} }

#endif
