/**
  *  \file server/interface/hostcron.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTCRON_HPP
#define C2NG_SERVER_INTERFACE_HOSTCRON_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "server/types.hpp"
#include "afl/base/optional.hpp"

namespace server { namespace interface {

    class HostCron : public afl::base::Deletable {
     public:
        enum Action {
            UnknownAction,
            NoAction,
            HostAction,
            ScheduleChangeAction,
            MasterAction
        };
        struct Event {
            int32_t gameId;
            Action action;
            Time_t time;

            Event(int32_t gameId, Action action, Time_t time)
                : gameId(gameId), action(action), time(time)
                { }
            Event()
                : gameId(0), action(UnknownAction), time(0)
                { }
        };

        // CRONGET game:GID (Host Command)
        virtual Event getGameEvent(int32_t gameId) = 0;

        // CRONLIST [LIMIT n:Int] (Host Command)
        virtual void listGameEvents(afl::base::Optional<int32_t> limit, std::vector<Event>& result) = 0;

        // CRONKICK game:GID (Host Command)
        virtual bool kickstartGame(int32_t gameId) = 0;
    };

} }

#endif
