/**
  *  \file server/file/commandhandler.hpp
  */
#ifndef C2NG_SERVER_FILE_COMMANDHANDLER_HPP
#define C2NG_SERVER_FILE_COMMANDHANDLER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace file {

    class Root;
    class Session;

    class CommandHandler : public server::interface::ComposableCommandHandler {
     public:
        CommandHandler(Root& root, Session& session);

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        Root& m_root;
        Session& m_session;

        String_t getHelp();

        void logCommand(const String_t& verb, interpreter::Arguments args);
    };

} }

#endif
