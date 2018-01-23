/**
  *  \file server/interface/hostschedule.hpp
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSCHEDULE_HPP
#define C2NG_SERVER_INTERFACE_HOSTSCHEDULE_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/types.hpp"
#include "afl/base/optional.hpp"
#include "server/types.hpp"
#include "afl/data/integerlist.hpp"

namespace server { namespace interface {

    class HostSchedule : public afl::base::Deletable {
     public:
        enum Type {
            Stopped,
            Weekly,
            Daily,
            Quick,
            Manual
        };

        enum Condition {
            None,
            Turn,
            Time
        };

        struct Schedule {
            afl::base::Optional<Type> type;
            afl::base::Optional<int32_t> weekdays;
            afl::base::Optional<int32_t> interval;
            afl::base::Optional<int32_t> daytime;
            afl::base::Optional<bool> hostEarly;
            afl::base::Optional<int32_t> hostDelay;
            afl::base::Optional<int32_t> hostLimit;
            afl::base::Optional<Condition> condition;
            afl::base::Optional<int32_t> conditionTurn;
            afl::base::Optional<int32_t> conditionTime;

            Schedule();
            ~Schedule();
        };

        // SCHEDULEADD game:GID [scheduleParams...]
        virtual void add(int32_t gameId, const Schedule& sched) = 0;

        // SCHEDULESET game:GID [scheduleParams...]
        virtual void replace(int32_t gameId, const Schedule& sched) = 0;

        // SCHEDULEMOD game:GID [scheduleParams...]
        virtual void modify(int32_t gameId, const Schedule& sched) = 0;

        // SCHEDULELIST game:GID
        virtual void getAll(int32_t gameId, std::vector<Schedule>& result) = 0;

        // SCHEDULEDROP game:GID
        virtual void drop(int32_t gameId) = 0;

        // SCHEDULESHOW game:GID
        virtual void preview(int32_t gameId,
                             afl::base::Optional<Time_t> timeLimit,
                             afl::base::Optional<int32_t> turnLimit,
                             afl::data::IntegerList_t& result) = 0;

        static int32_t formatType(Type t);
        static bool parseType(int32_t i, Type& t);
        static int32_t formatCondition(Condition c);
        static bool parseCondition(int32_t i, Condition& c);
    };

} }

#endif
