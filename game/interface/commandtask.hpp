/**
  *  \file game/interface/commandtask.hpp
  *  \brief Class game::interface::CommandTask
  */
#ifndef C2NG_GAME_INTERFACE_COMMANDTASK_HPP
#define C2NG_GAME_INTERFACE_COMMANDTASK_HPP

#include <memory>
#include "game/interface/contextprovider.hpp"
#include "game/interface/scripttask.hpp"
#include "game/session.hpp"

namespace game { namespace interface {

    /** Task for executing a script command.
        Compiles and executes a single command as a process in the target process group. */
    class CommandTask : public ScriptTask {
     public:
        /** Constructor.
            @param command   Command entered by user
            @param verbose   Verbose mode. If set, the script's status ("Suspended", expression result...)
                             is logged on the console after the process completes.
            @param name      Process name
            @param ctxp      ContextProvider to provide additional contexts (e.g. current ship); can be null */
        CommandTask(String_t command, bool verbose, String_t name, std::auto_ptr<ContextProvider> ctxp);

        // ScriptTask:
        virtual void execute(uint32_t pgid, Session& session);

     private:
        String_t m_command;
        bool m_verbose;
        String_t m_name;
        std::auto_ptr<ContextProvider> m_contextProvider;
    };

} }

#endif
