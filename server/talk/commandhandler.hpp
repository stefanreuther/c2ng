/**
  *  \file server/talk/commandhandler.hpp
  */
#ifndef C2NG_SERVER_TALK_COMMANDHANDLER_HPP
#define C2NG_SERVER_TALK_COMMANDHANDLER_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace talk {

    class Root;
    class Session;

    class CommandHandler : public server::interface::ComposableCommandHandler {
     public:
        CommandHandler(Root& root, Session& session);

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        Root& m_root;
        Session& m_session;

        String_t getHelp(String_t topic) const;

        void logCommand(const String_t& verb, interpreter::Arguments args);
    };

} }

#endif
