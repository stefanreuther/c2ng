/**
  *  \file server/host/schedule.hpp
  *  \brief Class server::host::Schedule
  */
#ifndef C2NG_SERVER_HOST_SCHEDULE_HPP
#define C2NG_SERVER_HOST_SCHEDULE_HPP

#include "afl/base/types.hpp"
#include "afl/bits/smallset.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "server/interface/hostschedule.hpp"
#include "server/types.hpp"

namespace server { namespace host {

    class Configuration;

    /** Schedule.
        This represents the in-memory version of a game schedule.
        It provides methods to manipulate, store in the database, and transfer to the interface,
        as well as to compute derived information. */
    class Schedule {
     public:
        /** Shortcut for the "schedule type" type. */
        typedef server::interface::HostSchedule::Type Type_t;

        /** Shortcut for the "end condition" type. */
        typedef server::interface::HostSchedule::Condition Condition_t;


        /** Default constructor.
            Creates a default-initialized schedule. */
        Schedule();

        /** Set schedule type.
            \param type Type */
        void setType(Type_t type);

        /** Set weekdays (for Weekly schedule).
            \param weekdays Set of weekdays. Day 0 is Sunday. */
        void setWeekDays(afl::bits::SmallSet<int8_t> weekdays);

        /** Set host interval in days (for Daily schedule).
            \param interval Number of days between hosts */
        void setInterval(int interval);

        /** Set daytime.
            \param daytime Minutes after midnight */
        void setDaytime(int daytime);

        /** Set host-early flag.
            \param value Value */
        void setHostEarly(bool value);

        /** Set host delay.
            Host is delayed this many minutes after last turn submission.
            \param hostDelay Host delay in minutes */
        void setHostDelay(int32_t hostDelay);

        /** Set host limit.
            Host is allowed to be delayed at most this many minutes to allow the next event to take place as scheduled.
            \param hostLimit Host limit in minutes */
        void setHostLimit(int32_t minutes);

        /** Set condition.
            The schedule ends when this condition has been reached.
            \param condition Condition
            \param arg Argument (turn number or time) */
        void setCondition(Condition_t condition, int32_t arg);

        /** Get schedule type.
            \return type */
        Type_t getType() const;

        /** Get weekdays.
            \return weekdays
            \see setWeekDays */
        afl::bits::SmallSet<int8_t> getWeekDays() const;

        /** Get host interval in days.
            \return interval
            \see setInterval */
        int getInterval() const;

        /** Get daytime.
            \return daytime
            \see setDaytime */
        int getDaytime() const;

        /** Get host-early flag.
            \return flag
            \see setHostEarly */
        bool getHostEarly() const;

        /** Get host delay.
            \return delay in minutes
            \see setHostDelay */
        int32_t getHostDelay() const;

        /** Get host limit.
            \return limit in minutes
            \see setHostLimit */
        int32_t getHostLimit() const;

        /** Get condition type.
            \return type */
        Condition_t getCondition() const;

        /** Get condition argument.
            \return time or turn number */
        int32_t getConditionArg() const;

        /** Check whether schedule is expired.
            \param turn Current turn
            \param time Current time
            \return true iff schedule is expired */
        bool isExpired(int32_t turn, Time_t time) const;

        /** Load from database key.
            \param k Key to load from
            \throw std::exception if database content is invalid */
        void loadFrom(afl::net::redis::HashKey h);

        /** Save to database key.
            \param k Key to store to */
        void saveTo(afl::net::redis::HashKey h) const;


        /*
         *  Computations
         */

        // FIXME: these methods suck; can we rework them?

        /** Get next host.
            Returns a time > now for the next possible host.
            \param now Current time
            \return next possible host time */
        Time_t getNextHost(int32_t now) const;

        /** Get previous host.
            Returns a time <= now for a possible previous host.
            \param now Current time
            \return previous possible host time */
        Time_t getPreviousHost(int32_t now) const;

        /** Get first possile host before a date.
            Used after discontinuities in the schedule.
            If a weekly/daily schedule follows a Quick/Manual schedule that hosted rarely,
            getNextHost(lastHostTime) will usually return currentTime.
            However, players expect the host to run at the set time.
            Therefore, we fake a lastHostTime that causes the next host to be generated at a sufficiently sensible day.
            \param now Current time
            \return possible host date */
        Time_t getPreviousVirtualHost(int32_t now) const;

        /** Describe using interface type.
            \param config Configuration
            \return Schedule description using interface type */
        server::interface::HostSchedule::Schedule describe(const Configuration& config) const;

     private:
        /** Schedule type (database field "type"). */
        Type_t m_type;

        /** Weekdays (database field "weekdays"). */
        afl::bits::SmallSet<int8_t> m_weekdays;

        /** Host interval (database field "interval"). */
        int m_interval;

        /** Daytime (database field "daytime"). */
        int32_t m_daytime;

        /** Host-early flag (database field "hostEarly"). */
        bool m_hostEarly;

        /** Host delay (database field "hostDelay"). */
        int32_t m_hostDelay;

        /** Host limit (database field "hostLimit"). */
        int32_t m_hostLimit;

        /** Validity condition (database field "condition"). */
        Condition_t m_condition;

        /** Validity condition parameter (database field "condTurn" or "condTime"). */
        // FIXME: this is overloaded due to hysterical reasons. Maybe fix someday.
        int32_t m_condTurnOrTime;
    };

} }

#endif
