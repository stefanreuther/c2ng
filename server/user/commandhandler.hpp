/**
  *  \file server/user/commandhandler.hpp
  *  \brief Class server::user::CommandHandler
  */
#ifndef C2NG_SERVER_USER_COMMANDHANDLER_HPP
#define C2NG_SERVER_USER_COMMANDHANDLER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace user {

    class Root;

    /** User Server CommandHandler implementation.
        Dispatches all commands for this service. */
    class CommandHandler : public server::interface::ComposableCommandHandler {
     public:
        /** Constructor.
            @param root Service root */
        explicit CommandHandler(Root& root);

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        String_t getHelp(String_t topic) const;

        Root& m_root;
    };

} }

#endif
