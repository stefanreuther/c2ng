/**
  *  \file server/interface/hostcronclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTCRONCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTCRONCLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "afl/data/value.hpp"
#include "server/interface/hostcron.hpp"

namespace server { namespace interface {

    class HostCronClient : public HostCron {
     public:
        explicit HostCronClient(afl::net::CommandHandler& commandHandler);

        virtual Event getGameEvent(int32_t gameId);
        virtual void listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result);
        virtual void kickstartGame(int32_t gameId);

        static Event unpackEvent(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
