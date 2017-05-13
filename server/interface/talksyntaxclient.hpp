/**
  *  \file server/interface/talksyntaxclient.hpp
  *  \brief Class server::interface::TalkSyntaxClient
  */
#ifndef C2NG_SERVER_INTERFACE_TALKSYNTAXCLIENT_HPP
#define C2NG_SERVER_INTERFACE_TALKSYNTAXCLIENT_HPP

#include "server/interface/talksyntax.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    /** Client for syntax-table inquiry.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class TalkSyntaxClient : public TalkSyntax {
     public:
        /** Constructor.
            \param commandHandler Server connection. Lifetime must exceed that of the TalkSyntaxClient. */
        explicit TalkSyntaxClient(afl::net::CommandHandler& commandHandler);

        /** Destructor. */
        ~TalkSyntaxClient();

        // TalkSyntax:
        virtual String_t get(String_t key);
        virtual afl::base::Ref<afl::data::Vector> mget(afl::base::Memory<const String_t> keys);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
