/**
  *  \file server/file/commandhandler.hpp
  *  \brief Class server::file::CommandHandler
  */
#ifndef C2NG_SERVER_FILE_COMMANDHANDLER_HPP
#define C2NG_SERVER_FILE_COMMANDHANDLER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace file {

    class Root;
    class Session;

    /** CommandHandler implementation for File service.
        This dispatches to the services offered by a File service, namely
        - Base
        - FileBase
        - FileGame

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

        String_t getHelp();

        void logCommand(const String_t& verb, interpreter::Arguments args);
    };

} }

#endif
