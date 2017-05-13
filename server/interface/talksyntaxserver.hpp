/**
  *  \file server/interface/talksyntaxserver.hpp
  *  \brief Class server::interface::TalkSyntaxServer
  */
#ifndef C2NG_SERVER_INTERFACE_TALKSYNTAXSERVER_HPP
#define C2NG_SERVER_INTERFACE_TALKSYNTAXSERVER_HPP

#include "server/interface/composablecommandhandler.hpp"

namespace server { namespace interface {

    class TalkSyntax;

    /** Syntax-table server.
        Implements a CommandHandler that accepts SYNTAX commands and translates them into calls on a TalkSyntax instance.
        Unknown commands produce an exception. */
    class TalkSyntaxServer : public ComposableCommandHandler {
     public:
        /** Constructor.
            \param impl Implementation. Lifetime must exceed that of the TalkSyntaxServer. */
        explicit TalkSyntaxServer(TalkSyntax& impl);

        /** Destructor. */
        ~TalkSyntaxServer();

        // ComposableCommandHandler:
        virtual bool handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result);

     private:
        TalkSyntax& m_implementation;
    };

} }

#endif
