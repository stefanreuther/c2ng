/**
  *  \file server/user/commandhandler.hpp
  */
#ifndef C2NG_SERVER_USER_COMMANDHANDLER_HPP
#define C2NG_SERVER_USER_COMMANDHANDLER_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace user {

    class Root;

    class CommandHandler : public server::interface::ComposableCommandHandler {
     public:
        CommandHandler(Root& root);

        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        String_t getHelp(String_t topic) const;

        Root& m_root;
    };

} }

#endif
