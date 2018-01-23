/**
  *  \file server/host/schedule.cpp
  *  \brief Class server::host::Schedule
  *
  *  FIXME: this module still needs a lot of love
  */

#include <stdexcept>
#include "server/host/schedule.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "server/host/configuration.hpp"
#include "server/errors.hpp"

using server::interface::HostSchedule;

namespace {
    const int MINUTES_PER_DAY = 60*24;

    int8_t getWeekdayFromTime(server::Time_t time)
    {
        /* Counting starts at a Thursday, which we want to be day 4. */
        return int8_t(((time / (24*60)) + 4) % 7);
    }
}

// Default constructor.
server::host::Schedule::Schedule()
    : m_type(HostSchedule::Stopped),
      m_weekdays(),
      m_interval(3),
      m_daytime(6*60),
      m_hostEarly(true),
      m_hostDelay(30),
      m_hostLimit(360),
      m_condition(HostSchedule::None),
      m_condTurnOrTime(0)
{
    // ex Schedule::Schedule
}

// Set schedule type.
void
server::host::Schedule::setType(Type_t type)
{
    // ex Schedule::setType
    m_type = type;
}

// Set weekdays (for Weekly schedule).
void
server::host::Schedule::setWeekDays(afl::bits::SmallSet<int8_t> weekdays)
{
    // ex Schedule::setWeekDays
    m_weekdays = weekdays;
}

// Set host interval in days (for Daily schedule).
void
server::host::Schedule::setInterval(int interval)
{
    // ex Schedule::setInterval
    m_interval = interval;
}

// Set daytime.
void
server::host::Schedule::setDaytime(int daytime)
{
    // ex Schedule::setDaytime
    m_daytime = daytime;
}

// Set host-early flag.
void
server::host::Schedule::setHostEarly(bool value)
{
    // ex Schedule::setHostEarly
    m_hostEarly = value;
}

// Set host delay.
void
server::host::Schedule::setHostDelay(int32_t hostDelay)
{
    // ex Schedule::setHostDelay
    m_hostDelay = hostDelay;
}

// Set host limit.
void
server::host::Schedule::setHostLimit(int32_t minutes)
{
    // ex Schedule::setHostLimit
    m_hostLimit = minutes;
}

// Set condition.
void
server::host::Schedule::setCondition(Condition_t condition, int32_t arg)
{
    // ex Schedule::setCondition
    m_condition      = condition;
    m_condTurnOrTime = arg;
}

// Get schedule type.
server::host::Schedule::Type_t
server::host::Schedule::getType() const
{
    // ex Schedule::getType
    return m_type;
}

// Get weekdays.
afl::bits::SmallSet<int8_t>
server::host::Schedule::getWeekDays() const
{
    // ex Schedule::getWeekDays
    return m_weekdays;
}

// Get host interval in days.
int
server::host::Schedule::getInterval() const
{
    // ex Schedule::getInterval
    return m_interval;
}

// Get daytime.
int
server::host::Schedule::getDaytime() const
{
    // ex Schedule::getDaytime
    return m_daytime;
}

// Get host-early flag.
bool
server::host::Schedule::getHostEarly() const
{
    // ex Schedule::getHostEarly
    return m_hostEarly;
}

// Get host delay.
int32_t
server::host::Schedule::getHostDelay() const
{
    // ex Schedule::getHostDelay
    return m_hostDelay;
}

// Get host limit.
int32_t
server::host::Schedule::getHostLimit() const
{
    // ex Schedule::getHostLimit
    return m_hostLimit;
}

// Get condition type.
server::host::Schedule::Condition_t
server::host::Schedule::getCondition() const
{
    // ex Schedule::getCondition
    return m_condition;
}

// Get condition argument.
int32_t
server::host::Schedule::getConditionArg() const
{
    // ex Schedule::getConditionArg
    return m_condTurnOrTime;
}

// Check whether schedule is expired.
bool
server::host::Schedule::isExpired(int32_t turn, Time_t time) const
{
    switch (m_condition) {
     case HostSchedule::Turn:
        // "Until turn 20" includes turn 20
        return turn > m_condTurnOrTime;
     case HostSchedule::Time:
        // "Until time X" means it is discarded at time X, because the
        // next time we'll look at it it will be later.
        return time >= m_condTurnOrTime;
     case HostSchedule::None:
        return false;
    }
    return false;
}

// Load from database key.
void
server::host::Schedule::loadFrom(afl::net::redis::HashKey h)
{
    // ex Schedule::loadFrom
    if (!HostSchedule::parseType(h.intField("type").get(), m_type)) {
        throw std::runtime_error(DATABASE_ERROR);
    }
    if (m_type == HostSchedule::Weekly) {
        m_weekdays = afl::bits::SmallSet<int8_t>::fromInteger(h.intField("weekdays").get());
    }
    if (m_type == HostSchedule::Daily) {
        m_interval = h.intField("interval").get();
    }
    m_daytime   = h.intField("daytime").get();
    m_hostEarly = h.intField("hostEarly").get();
    m_hostDelay = h.intField("hostDelay").get();
    m_hostLimit = h.intField("hostLimit").get();
    if (!HostSchedule::parseCondition(h.intField("condition").get(), m_condition)) {
        throw std::runtime_error(DATABASE_ERROR);
    }
    if (m_condition == HostSchedule::Turn) {
        m_condTurnOrTime = h.intField("condTurn").get();
    }
    if (m_condition == HostSchedule::Time) {
        m_condTurnOrTime = h.intField("condTime").get();
    }
}

// Save to database key.
void
server::host::Schedule::saveTo(afl::net::redis::HashKey h) const
{
    // ex Schedule::saveTo
    h.intField("type").set(HostSchedule::formatType(m_type));
    if (m_type == HostSchedule::Weekly) {
        h.intField("weekdays").set(m_weekdays.toInteger());
    }
    if (m_type == HostSchedule::Daily) {
        h.intField("interval").set(m_interval);
    }
    h.intField("daytime").set(m_daytime);
    h.intField("hostEarly").set(m_hostEarly);
    h.intField("hostDelay").set(m_hostDelay);
    h.intField("hostLimit").set(m_hostLimit);
    h.intField("condition").set(HostSchedule::formatCondition(m_condition));
    if (m_condition == HostSchedule::Turn) {
        h.intField("condTurn").set(m_condTurnOrTime);
    }
    if (m_condition == HostSchedule::Time) {
        h.intField("condTime").set(m_condTurnOrTime);
    }
}

// Get next host.
server::Time_t
server::host::Schedule::getNextHost(int32_t now) const
{
    // ex Schedule::getNextHost
    switch (m_type) {
     case HostSchedule::Stopped:
     case HostSchedule::Quick:
     case HostSchedule::Manual:
        return 0;

     case HostSchedule::Daily:
        // Host happens precisely interval days after the previous host.
        // If host was delayed too much, move it farther a day.
        {
            Time_t prev = getPreviousHost(now);
            Time_t next = prev + m_interval*MINUTES_PER_DAY;
            if (now - prev > m_hostLimit) {
                // Host was delayed too much, so skip a day.
                next += MINUTES_PER_DAY;
            }
            return next;
        }

     case HostSchedule::Weekly:
        {
            // Find next day.
            Time_t prev = getPreviousHost(now);
            Time_t day = ((now - m_daytime) / MINUTES_PER_DAY);
            for (int i = 0; i < 14; ++i) {
                // Must look at 14 days, because getPreviousHost might end up juuust at the
                // current and only host day, causing us to miss the next one.
                ++day;
                int8_t weekday = getWeekdayFromTime(MINUTES_PER_DAY * day);
                if (m_weekdays.contains(weekday)) {
                    int32_t thisDate = m_daytime + MINUTES_PER_DAY * day;
                    if (now - prev > m_hostLimit) {
                        // Host was delayed too much, so skip this one
                        prev = thisDate;
                    } else {
                        // Accept
                        return thisDate;
                    }
                }
            }
        }
    }
    return 0;
}

// Get previous host.
server::Time_t
server::host::Schedule::getPreviousHost(int32_t now) const
{
    // ex Schedule::getPreviousHost
    switch (m_type) {
     case HostSchedule::Stopped:
     case HostSchedule::Quick:
     case HostSchedule::Manual:
        return 0;

     case HostSchedule::Daily:
        // Hosts can happen each day. So round up down to preceding occasion of /daytime/.
        return m_daytime + MINUTES_PER_DAY * ((now - m_daytime) / MINUTES_PER_DAY);

     case HostSchedule::Weekly:
        // Find a day before now where this host could have happened.
        {
            Time_t day = ((now - m_daytime) / MINUTES_PER_DAY);
            for (int i = 0; i < 14; ++i) {
                // Must look at 14 days, because getPreviousHost might end up juuust at the
                // current and only host day, causing us to miss the next one.
                int8_t weekday = getWeekdayFromTime(MINUTES_PER_DAY * day);
                if (m_weekdays.contains(weekday)) {
                    break;
                }
                --day;
            }
            return m_daytime + MINUTES_PER_DAY * day;
        }
    }
    return 0;
}

// Get first possile host before a date.
server::Time_t
server::host::Schedule::getPreviousVirtualHost(int32_t now) const
{
    // ex Schedule::getPreviousVirtualHost
    // If now is at or slightly after a sensible host run, this would otherwise
    // return a date today for the virtual host run, setting the next one far
    // into the future. Thus, go a little back in time, using hostDelay as the
    // maximum allowed host delay. This will set the previous virtual host to
    // the previous date, placing the one following it again close to now, which
    // is what we want.
    now -= m_hostLimit;
    now -= 1;

    switch (m_type) {
     case HostSchedule::Stopped:
     case HostSchedule::Quick:
     case HostSchedule::Manual:
        return 0;

     case HostSchedule::Daily:
        // Hosts can happen each day. getPreviousHost() returns yesterday.
        // To get a host run for today, we'd have to add one day; the
        // virtual host would have to happen n-1 days before.
        return getPreviousHost(now) - (m_interval-1) * MINUTES_PER_DAY;

     case HostSchedule::Weekly:
        // Find a day before now where this host could have happened. This happens
        // to be the same as getPreviousHost().
        return getPreviousHost(now);
    }
    return 0;
}

// Describe using interface type.
server::interface::HostSchedule::Schedule
server::host::Schedule::describe(const Configuration& config) const
{
    // ex planetscentral/host/cmdsched.h:packSchedule [heavily different]
    using server::interface::HostSchedule;

    HostSchedule::Schedule result;
    result.type = m_type;
    if (m_type == HostSchedule::Weekly) {
        result.weekdays = getWeekDays().toInteger();
    }
    if (m_type == HostSchedule::Daily) {
        result.interval = getInterval();
    }
    result.daytime = getDaytime();
    result.hostEarly = getHostEarly();
    result.hostDelay = getHostDelay();
    result.hostLimit = getHostLimit();
    result.condition = getCondition();
    switch (getCondition()) {
     case HostSchedule::Turn:
        result.conditionTurn = getConditionArg();
        break;
     case HostSchedule::Time:
        result.conditionTime = config.getUserTimeFromTime(getConditionArg());
        break;
     case HostSchedule::None:
        break;
    }
    return result;
}
