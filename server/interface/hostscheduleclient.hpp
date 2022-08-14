/**
  *  \file server/interface/hostscheduleclient.hpp
  *  \brief Class server::interface::HostScheduleClient
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSCHEDULECLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSCHEDULECLIENT_HPP

#include "afl/net/commandhandler.hpp"
#include "server/interface/hostgame.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Client for host schedule access.
        Uses a CommandHandler to send commands to a server, and receives the results. */
    class HostScheduleClient : public HostSchedule {
     public:
        /** Constructor.
            @param commandHandler Server connection. Lifetime must exceed that of the HostScheduleClient. */
        explicit HostScheduleClient(afl::net::CommandHandler& commandHandler);

        // HostSchedule virtuals:
        virtual void add(int32_t gameId, const Schedule& sched);
        virtual void replace(int32_t gameId, const Schedule& sched);
        virtual void modify(int32_t gameId, const Schedule& sched);
        virtual void getAll(int32_t gameId, std::vector<Schedule>& result);
        virtual void drop(int32_t gameId);
        virtual void preview(int32_t gameId,
                             afl::base::Optional<Time_t> timeLimit,
                             afl::base::Optional<int32_t> turnLimit,
                             afl::data::IntegerList_t& result);

        /** Unpack a schedule/schedule modification.
            @param p Value received from server
            @return Schedule */
        static Schedule unpackSchedule(const Value_t* p);

        /** Pack a schedule/schedule modification into a command sequence.
            @param [out] cmd    Command options added here
            @param [in]  sched  Schedule modification to format */
        static void packSchedule(afl::data::Segment& cmd, const Schedule& sched);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
