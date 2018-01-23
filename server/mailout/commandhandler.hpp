/**
  *  \file server/mailout/commandhandler.hpp
  *  \brief Class server::mailout::CommandHandler
  */
#ifndef C2NG_SERVER_MAILOUT_COMMANDHANDLER_HPP
#define C2NG_SERVER_MAILOUT_COMMANDHANDLER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace mailout {

    class Root;
    class Session;

    /** CommandHandler implementation for Mailout service.

        It does not hold state and can thus be short-lived.
        All session state is in Session, all service state is in Root. */
    class CommandHandler : public afl::net::CommandHandler {
     public:
        /** Constructor.
            \param root Service root
            \param session Session state */
        CommandHandler(Root& root, Session& session);

        // CommandHandler:
        virtual Value_t* call(const Segment_t& command);
        virtual void callVoid(const Segment_t& command);

     private:
        Root& m_root;
        Session& m_session;
    };

} }

#endif
