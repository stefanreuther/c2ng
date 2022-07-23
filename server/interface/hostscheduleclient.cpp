/**
  *  \file server/interface/hostscheduleclient.cpp
  */

#include <memory>
#include "server/interface/hostscheduleclient.hpp"
#include "afl/data/access.hpp"
#include "afl/data/segment.hpp"
#include "afl/except/invaliddataexception.hpp"

using afl::data::Segment;
using afl::data::Access;

server::interface::HostScheduleClient::HostScheduleClient(afl::net::CommandHandler& commandHandler)
    : m_commandHandler(commandHandler)
{ }

// SCHEDULEADD game:GID [scheduleParams...]
void
server::interface::HostScheduleClient::add(int32_t gameId, const Schedule& sched)
{
    Segment cmd;
    cmd.pushBackString("SCHEDULEADD");
    cmd.pushBackInteger(gameId);
    packSchedule(cmd, sched);
    m_commandHandler.callVoid(cmd);
}

// SCHEDULESET game:GID [scheduleParams...]
void
server::interface::HostScheduleClient::replace(int32_t gameId, const Schedule& sched)
{
    Segment cmd;
    cmd.pushBackString("SCHEDULESET");
    cmd.pushBackInteger(gameId);
    packSchedule(cmd, sched);
    m_commandHandler.callVoid(cmd);
}

// SCHEDULEMOD game:GID [scheduleParams...]
void
server::interface::HostScheduleClient::modify(int32_t gameId, const Schedule& sched)
{
    Segment cmd;
    cmd.pushBackString("SCHEDULEMOD");
    cmd.pushBackInteger(gameId);
    packSchedule(cmd, sched);
    m_commandHandler.callVoid(cmd);
}

// SCHEDULELIST game:GID
void
server::interface::HostScheduleClient::getAll(int32_t gameId, std::vector<Schedule>& result)
{
    std::auto_ptr<Value_t> p(m_commandHandler.call(Segment().pushBackString("SCHEDULELIST").pushBackInteger(gameId)));
    Access a(p);
    for (size_t i = 0, n = a.getArraySize(); i < n; ++i) {
        result.push_back(unpackSchedule(a[i].getValue()));
    }
}

// SCHEDULEDROP game:GID
void
server::interface::HostScheduleClient::drop(int32_t gameId)
{
    m_commandHandler.callVoid(Segment().pushBackString("SCHEDULEDROP").pushBackInteger(gameId));
}

// SCHEDULESHOW game:GID
void
server::interface::HostScheduleClient::preview(int32_t gameId,
                                               afl::base::Optional<Time_t> timeLimit,
                                               afl::base::Optional<int32_t> turnLimit,
                                               afl::data::IntegerList_t& result)
{
    Segment cmd;
    cmd.pushBackString("SCHEDULESHOW");
    cmd.pushBackInteger(gameId);
    if (const Time_t* p = timeLimit.get()) {
        cmd.pushBackString("TIMELIMIT");
        cmd.pushBackInteger(*p);
    }
    if (const int32_t* p = turnLimit.get()) {
        cmd.pushBackString("TURNLIMIT");
        cmd.pushBackInteger(*p);
    }
    std::auto_ptr<Value_t> p(m_commandHandler.call(cmd));
    Access(p).toIntegerList(result);
}

server::interface::HostSchedule::Schedule
server::interface::HostScheduleClient::unpackSchedule(const Value_t* p)
{
    Schedule sch;
    Access a(p);

    // type
    if (const Value_t* pp = a("type").getValue()) {
        Type type;
        if (!parseType(Access(pp).toInteger(), type)) {
            throw afl::except::InvalidDataException("<HostSchedule.unpackSchedule: type>");
        }
        sch.type = type;
    }

    // weekdays
    sch.weekdays = toOptionalInteger(a("weekdays").getValue());

    // interval
    sch.interval = toOptionalInteger(a("interval").getValue());

    // daytime
    sch.daytime = toOptionalInteger(a("daytime").getValue());

    // hostEarly
    if (const Value_t* pp = a("hostEarly").getValue()) {
        sch.hostEarly = Access(pp).toInteger() != 0;
    }

    // hostDelay
    sch.hostDelay = toOptionalInteger(a("hostDelay").getValue());

    // hostLimit
    sch.hostLimit = toOptionalInteger(a("hostLimit").getValue());

    // condition
    if (const Value_t* pp = a("condition").getValue()) {
        Condition condition;
        if (!parseCondition(Access(pp).toInteger(), condition)) {
            throw afl::except::InvalidDataException("<HostSchedule.unpackSchedule: condition>");
        }
        sch.condition = condition;
    }

    // condTurn
    sch.conditionTurn = toOptionalInteger(a("condTurn").getValue());

    // condTime
    sch.conditionTime = toOptionalInteger(a("condTime").getValue());
    return sch;
}

void
server::interface::HostScheduleClient::packSchedule(afl::data::Segment& cmd, const Schedule& sched)
{
    if (const Type* ty = sched.type.get()) {
        switch (*ty) {
         case Stopped:
            cmd.pushBackString("STOP");
            break;
         case Weekly:
            cmd.pushBackString("WEEKLY");
            cmd.pushBackInteger(sched.weekdays.orElse(0));
            break;
         case Daily:
            cmd.pushBackString("DAILY");
            cmd.pushBackInteger(sched.interval.orElse(0));
            break;
         case Quick:
            cmd.pushBackString("ASAP");
            break;
         case Manual:
            cmd.pushBackString("MANUAL");
            break;
        }
    }

    if (const int32_t* p = sched.daytime.get()) {
        cmd.pushBackString("DAYTIME");
        cmd.pushBackInteger(*p);
    }

    if (const bool* p = sched.hostEarly.get()) {
        if (*p) {
            cmd.pushBackString("EARLY");
        } else {
            cmd.pushBackString("NOEARLY");
        }
    }

    if (const int32_t* p = sched.hostDelay.get()) {
        cmd.pushBackString("DELAY");
        cmd.pushBackInteger(*p);
    }

    if (const int32_t* p = sched.hostLimit.get()) {
        cmd.pushBackString("LIMIT");
        cmd.pushBackInteger(*p);
    }

    if (const Condition* pc = sched.condition.get()) {
        switch (*pc) {
         case None:
            cmd.pushBackString("FOREVER");
            break;
         case Turn:
            cmd.pushBackString("UNTILTURN");
            cmd.pushBackInteger(sched.conditionTurn.orElse(0));
            break;
         case Time:
            cmd.pushBackString("UNTILTIME");
            cmd.pushBackInteger(sched.conditionTime.orElse(0));
            break;
        }
    }
}
