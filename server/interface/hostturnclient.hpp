/**
  *  \file server/interface/hostturnclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTTURNCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTTURNCLIENT_HPP

#include "server/interface/hostturn.hpp"
#include "afl/net/commandhandler.hpp"

namespace server { namespace interface {

    class HostTurnClient : public HostTurn {
     public:
        explicit HostTurnClient(afl::net::CommandHandler& commandHandler);

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
