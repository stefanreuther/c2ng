/**
  *  \file server/interface/hostschedule.hpp
  *  \brief Interface server::interface::HostSchedule
  */
#ifndef C2NG_SERVER_INTERFACE_HOSTSCHEDULE_HPP
#define C2NG_SERVER_INTERFACE_HOSTSCHEDULE_HPP

#include <vector>
#include "afl/base/deletable.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/data/integerlist.hpp"
#include "server/types.hpp"

namespace server { namespace interface {

    /** Host file schedule interface.
        This interface allows to access game schedules. */
    class HostSchedule : public afl::base::Deletable {
     public:
        /** Schedule type. */
        enum Type {
            Stopped,                                        ///< Scheduler does not run.
            Weekly,                                         ///< Weekday-based schedule.
            Daily,                                          ///< Every-n-days schedule.
            Quick,                                          ///< Run when turns are in.
            Manual                                          ///< Host runs triggered manually.
        };

        /** Schedule expiration condition. */
        enum Condition {
            None,                                           ///< Condition not active.
            Turn,                                           ///< Check turn number.
            Time                                            ///< Check time.
        };

        /** Schedule status or update information. */
        struct Schedule {
            afl::base::Optional<Type> type;                 ///< Schedule type.
            afl::base::Optional<int32_t> weekdays;          ///< weekly: bits for weekdays, 1<<0(Sunday) .. 1<<6(Saturday)
            afl::base::Optional<int32_t> interval;          ///< daily: days between host.
            afl::base::Optional<int32_t> daytime;           ///< weekly/daily: preferred day time for host.
            afl::base::Optional<bool> hostEarly;            ///< weekly/daily: host early when all turns are in.
            afl::base::Optional<int32_t> hostDelay;         ///< Delay after last turn submission before Quick/hostEarly host runs.
            afl::base::Optional<int32_t> hostLimit;         ///< Maximum time host can be delayed before next host is skipped.
            afl::base::Optional<Condition> condition;       ///< Expiration condition.
            afl::base::Optional<int32_t> conditionTurn;     ///< Expiration turn.
            afl::base::Optional<int32_t> conditionTime;     ///< Expiration time.

            Schedule();
            ~Schedule();
        };

        /** Add new schedule (SCHEDULEADD).
            @param gameId   Game Id
            @param sched    New schedule */
        virtual void add(int32_t gameId, const Schedule& sched) = 0;

        /** Replace schedule (SCHEDULESET).
            @param gameId   Game Id
            @param sched    New schedule */
        virtual void replace(int32_t gameId, const Schedule& sched) = 0;

        /** Modify schedule (SCHEDULEMOD).
            @param gameId   Game Id
            @param sched    Schedule modification */
        virtual void modify(int32_t gameId, const Schedule& sched) = 0;

        /** List schedules (SCHEDULELIST).
            @param [in]  gameId   Game Id
            @param [out] result   Result */
        virtual void getAll(int32_t gameId, std::vector<Schedule>& result) = 0;

        /** Drop first schedule (SCHEDULEDROP).
            @param gameId   Game Id */
        virtual void drop(int32_t gameId) = 0;

        /** Preview schedule (SCHEDULESHOW).
            @param [in]  gameId    Game Id
            @param [in]  timeLimit Limit time of report
            @param [in]  turnLimit Limit number of turns reported
            @param [out] result    List of predicted host times */
        virtual void preview(int32_t gameId,
                             afl::base::Optional<Time_t> timeLimit,
                             afl::base::Optional<int32_t> turnLimit,
                             afl::data::IntegerList_t& result) = 0;

        /** Format Type to string.
            @param t Type
            @return String representation */
        static int32_t formatType(Type t);

        /** Parse string into Type.
            @param [in]  i String
            @param [out] t Result
            @return true on success, false if string does not match a valid Type */
        static bool parseType(int32_t i, Type& t);

        /** Format Condition to string.
            @param c Condition
            @return String representation */
        static int32_t formatCondition(Condition c);

        /** Parse string into Condition.
            @param [in]  i String
            @param [out] c Result
            @return true on success, false if string does not match a valid Condition */
        static bool parseCondition(int32_t i, Condition& c);
    };

} }

#endif
