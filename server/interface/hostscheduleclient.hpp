/**
  *  \file server/interface/hostscheduleclient.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSCHEDULECLIENT_HPP
#define C2NG_SERVER_INTERFACE_HOSTSCHEDULECLIENT_HPP

#include "server/interface/hostgame.hpp"
#include "afl/net/commandhandler.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    class HostScheduleClient : public HostSchedule {
     public:
        explicit HostScheduleClient(afl::net::CommandHandler& commandHandler);
    
        // SCHEDULEADD game:GID [scheduleParams...]
        virtual void add(int32_t gameId, const Schedule& sched);

        // SCHEDULESET game:GID [scheduleParams...]
        virtual void replace(int32_t gameId, const Schedule& sched);

        // SCHEDULEMOD game:GID [scheduleParams...]
        virtual void modify(int32_t gameId, const Schedule& sched);

        // SCHEDULELIST game:GID
        virtual void getAll(int32_t gameId, std::vector<Schedule>& result);

        // SCHEDULEDROP game:GID
        virtual void drop(int32_t gameId);

        // SCHEDULESHOW game:GID
        virtual void preview(int32_t gameId,
                             afl::base::Optional<Time_t> timeLimit,
                             afl::base::Optional<int32_t> turnLimit,
                             afl::data::IntegerList_t& result);

        static Schedule unpackSchedule(const Value_t* p);
        static void packSchedule(afl::data::Segment& cmd, const Schedule& sched);

     private:
        afl::net::CommandHandler& m_commandHandler;
    };

} }

#endif
