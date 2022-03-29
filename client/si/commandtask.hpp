/**
  *  \file client/si/commandtask.hpp
  */
#ifndef C2NG_CLIENT_SI_COMMANDTASK_HPP
#define C2NG_CLIENT_SI_COMMANDTASK_HPP

#include <memory>
#include "game/interface/contextprovider.hpp"
#include "afl/string/string.hpp"
#include "client/si/scripttask.hpp"

namespace client { namespace si {

    class CommandTask : public ScriptTask {
     public:
        CommandTask(String_t command, bool verbose, String_t name, std::auto_ptr<game::interface::ContextProvider> ctxp);
        virtual void execute(uint32_t pgid, game::Session& session);

     private:
        String_t m_command;
        bool m_verbose;
        String_t m_name;
        std::auto_ptr<game::interface::ContextProvider> m_contextProvider;
    };

} }

#endif
