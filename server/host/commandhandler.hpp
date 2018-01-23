/**
  *  \file server/host/commandhandler.hpp
  *  \brief Class server::host::CommandHandler
  */
#ifndef C2NG_SERVER_HOST_COMMANDHANDLER_HPP
#define C2NG_SERVER_HOST_COMMANDHANDLER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace host {

    class Root;
    class Session;

    /** CommandHandler implementation for Host service.
        This dispatches to the services offered by a Host service, which is Base plus all Host services.

        It does not hold state and can thus be short-lived.
        All session state is in Session, all service state is in Root. */
    class CommandHandler : public server::interface::ComposableCommandHandler {
     public:
        /** Constructor.
            \param root Service root
            \param session Session state */
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
