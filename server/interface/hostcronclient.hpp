/**
  *  \file server/interface/hostcronclient.hpp
  *  \brief Class server::interface::HostCronClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTCRONCLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTCRONCLIENT_HPP

#include "afl/data/value.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/interface/hostcron.hpp"

namespace server { namespace interface {

    /** Client for host scheduler.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostCronClient : public HostCron {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostCronClient. */
        explicit HostCronClient(afl::net::CommandHandler& commandHandler);

        // HostCron virtuals:
        virtual Event getGameEvent(int32_t gameId);
        virtual void listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result);
        virtual bool kickstartGame(int32_t gameId);
        virtual void suspendScheduler(int32_t relativeTime);
        virtual void getBrokenGames(BrokenMap_t& result);

        /** Unpack an event received from the server.
            @param p Value tree received from server
            @return unpacked event */
        static Event unpackEvent(const afl::data::Value* p);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
