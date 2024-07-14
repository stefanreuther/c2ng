/**
  *  \file server/talk/commandhandler.hpp
  *  \brief Class server::talk::CommandHandler
  */
#ifndef C2NG_SERVER_TALK_COMMANDHANDLER_HPP
#define C2NG_SERVER_TALK_COMMANDHANDLER_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace talk {

    class Root;
    class Session;

    /** CommandHandler implementation for Talk service.
        Dispatches to all sub-interfaces (TalkPost, TalkUser, TalkSyntax, etc.).
        This is supposed to be a short-lived instance. */
    class CommandHandler : public server::interface::ComposableCommandHandler {
     public:
        /** Constructor.
            @param root Service root
            @param session Session */
        CommandHandler(Root& root, Session& session);

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        Root& m_root;
        Session& m_session;

        static String_t getHelp(String_t topic);

        void logCommand(const String_t& verb, interpreter::Arguments args);
    };

} }

#endif
