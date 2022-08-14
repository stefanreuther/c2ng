/**
  *  \file server/interface/hostturnclient.hpp
  *  \brief Class server::interface::HostTurnClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTURNCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTTURNCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/hostturn.hpp"

namespace server { namespace interface {

    /** Client for turn file submission.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostTurnClient : public HostTurn {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostTurnClient. */
        explicit HostTurnClient(afl::net::CommandHandler& commandHandler);

        // HostTurn virtuals:
        virtual Result submit(const String_t& blob,
                              afl::base::Optional<int32_t> game,
                              afl::base::Optional<int32_t> slot,
                              afl::base::Optional<String_t> mail,
                              afl::base::Optional<String_t> info);
        virtual void setTemporary(int32_t gameId, int32_t slot, bool flag);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
