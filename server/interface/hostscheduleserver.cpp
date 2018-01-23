/**
  *  \file server/interface/hostscheduleserver.cpp
  */

#include <stdexcept>
#include "server/interface/hostscheduleserver.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"

using afl::data::Hash;
using afl::data::HashValue;
using afl::data::Vector;
using afl::data::VectorValue;

server::interface::HostScheduleServer::HostScheduleServer(HostSchedule& impl)
    : ComposableCommandHandler(),
      m_implementation(impl)
{ }

server::interface::HostScheduleServer::~HostScheduleServer()
{ }

bool
server::interface::HostScheduleServer::handleCommand(const String_t& upcasedCommand, interpreter::Arguments& args, std::auto_ptr<Value_t>& result)
{
    if (upcasedCommand == "SCHEDULEADD") {
        /* @q SCHEDULEADD game:GID [scheduleParams...] (Host Command)
           Add new schedule for a game.
           This schedule will be the active schedule until it expires.

           The %scheduleParams are a list of keywords and parameters:
           - STOP (schedule type: do not run host)
           - WEEKLY n:{@type Int} (schedule type: host runs on fixed weekdays. %n is the bitfield of days; 0=Sunday, 1=Monday, ... 6=Saturday)
           - DAILY n:{@type Int} (schedule type: host runs every %n days)
           - ASAP (schedule type: host runs when all turns are in)
           - MANUAL (schedule type: host is run manually)
           - EARLY (acceleration: host runs early if all turns are in)
           - NOEARLY (acceleration: host does not run early)
           - DELAY n:{@type Int} (acceleration: minutes after last turn file to wait until host runs)
           - DAYTIME n:{@type Int} (for weekly/daily schedules: runs %n minutes after midnight)
           - LIMIT n:{@type Int} (for weekly/daily schedules: maximum minutes acceptable delay after which a host date is moved)
           - UNTILTURN n:{@type Int} (expiration: schedule expires after turn %n)
           - UNTILTIME n:{@type Time} (expiration: schedule expires at time %n)
           - FOREVER (expiration: schedule does not expire)

           Defaults are STOP, EARLY, DELAY 30 (half an hour), LIMIT 360 (six hours), FOREVER.
           The default DAYTIME is assigned individually for each game (see {game:hours}).

           Permissions: config-access to game.

           @uses game:$GID:schedule:list, game:$GID:schedule:$SID, game:$GID:schedule:lastId
           @see SCHEDULESET, SCHEDULEMOD, SCHEDULELIST
           @change PCC2 requires at least one token in the schedule; c2host-ng will transport empty commands */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        m_implementation.add(gameId, parseSchedule(args));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SCHEDULESET") {
        /* @q SCHEDULESET game:GID [scheduleParams...] (Host Command)
           Replace schedule.
           See {SCHEDULEADD} for a description of the %scheduleParams.

           The difference between this command and {SCHEDULEMOD} is that
           this one drops the front schedule and builds a new one from scratch.
           If there is no schedule, this command behaves like {SCHEDULEADD}.

           Permissions: config-access to game.

           @uses game:$GID:schedule:list, game:$GID:schedule:$SID, game:$GID:schedule:lastId
           @see SCHEDULEADD, SCHEDULEMOD, SCHEDULELIST
           @change PCC2 requires at least one token in the schedule; c2host-ng will transport empty commands */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        m_implementation.replace(gameId, parseSchedule(args));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SCHEDULEMOD") {
        /* @q SCHEDULEMOD game:GID [scheduleParams...] (Host Command)
           Modify schedule.
           See {SCHEDULEADD} for a description of the %scheduleParams.

           The difference between this command and {SCHEDULESET} is that
           this one edits the existing schedule in place and fails if there isn't one.

           Permissions: config-access to game.

           @err 410 Game does not have a schedule
           @uses game:$GID:schedule:list, game:$GID:schedule:$SID, game:$GID:schedule:lastId
           @see SCHEDULEADD, SCHEDULESET, SCHEDULELIST */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        m_implementation.modify(gameId, parseSchedule(args));
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SCHEDULELIST") {
        /* @q SCHEDULELIST game:GID (Host Command)
           List schedules of game.

           Permissions: read-access to game.

           @retval HostSchedule[] List of active schedules */
        args.checkArgumentCount(1);
        int32_t gameId = toInteger(args.getNext());

        std::vector<HostSchedule::Schedule> schedules;
        m_implementation.getAll(gameId, schedules);

        Vector::Ref_t v = Vector::create();
        for (size_t i = 0, n = schedules.size(); i < n; ++i) {
            v->pushBackNew(packSchedule(schedules[i]));
        }
        result.reset(new VectorValue(v));
        return true;
    } else if (upcasedCommand == "SCHEDULEDROP") {
        /* @q SCHEDULEDROP game:GID (Host Command)
           Drop first schedule.
           If the game has multiple schedules, the following schedule will kick in.
           Otherwise, the game will stop hosting.

           Permissions: config-access to game.

           @uses game:$GID:schedule:list */
        args.checkArgumentCount(1);
        int32_t gameId = toInteger(args.getNext());
        m_implementation.drop(gameId);
        result.reset(makeStringValue("OK"));
        return true;
    } else if (upcasedCommand == "SCHEDULESHOW") {
        /* @q SCHEDULESHOW game:GID [TIMELIMIT n:Int] [TURNLIMIT n:Int] (Host Command)
           Lists all future host dates.
           Computes future hosting times until either the time limit or the turn limit has been reached,
           or no more host times can be computed.

           Permissions: read-access to game.

           @retval IntList list of {@type Time}s of future host runs
           @rettype Time */
        args.checkArgumentCountAtLeast(1);
        int32_t gameId = toInteger(args.getNext());
        afl::base::Optional<Time_t> timeLimit;
        afl::base::Optional<int32_t> turnLimit;
        while (args.getNumArgs() > 0) {
            String_t keyword = afl::string::strUCase(toString(args.getNext()));
            if (keyword == "TIMELIMIT") {
                args.checkArgumentCountAtLeast(1);
                timeLimit = toInteger(args.getNext());
            } else if (keyword == "TURNLIMIT") {
                args.checkArgumentCountAtLeast(1);
                turnLimit = toInteger(args.getNext());
            } else {
                throw std::runtime_error(INVALID_OPTION);
            }
        }

        afl::data::IntegerList_t times;
        m_implementation.preview(gameId, timeLimit, turnLimit, times);

        Vector::Ref_t v = Vector::create();
        v->pushBackElements(times);
        result.reset(new VectorValue(v));
        return true;
    } else {
        return false;
    }
}

// /** Pack a schedule into an object.
//     \param h "game:GID:schedule:ID" object */
server::Value_t*
server::interface::HostScheduleServer::packSchedule(const HostSchedule::Schedule& sch)
{
    // ex planetscentral/host/cmdsched.cc:packSchedule
    /* @type HostSchedule
       Describes a schedule.
       This is essentially the content of {game:$GID:schedule:$SID}.

       @key type:Int        (0=stopped / 1=weekly / 2=daily / 3=asap / 4=manual)
       @key weekdays:Int    (if weekly: sum of 1<<0(Sunday) .. 1<<6(Saturday))
       @key interval:Int    (if daily: days between host)
       @key daytime:Int     (if weekly/daily: preferred day time (minutes))
       @key hostEarly:Int   (if weekly/daily: host early when all turns are in; defaults to true if asap)
       @key hostDelay:Int   (if hostEarly or asap: delay after last turn submission, minutes, default: 30)
       @key hostLimit:Int   (skip next host if host terminated more than this many minutes after start, default: 360 = 6 hours)
       @key condition:Int   (0=none / 1=turn / 2=time)
       @key condTurn:Int    (if turn: drop this condition at turn N)
       @key condTime:Time   (if time: drop this condition at time)
       @c        [- tempLimit (time limit for temporary turns)] */

    Hash::Ref_t h = Hash::create();
    if (const HostSchedule::Type* t = sch.type.get()) {
        h->setNew("type", makeIntegerValue(HostSchedule::formatType(*t)));
    }
    if (const int32_t* p = sch.weekdays.get()) {
        h->setNew("weekdays", makeIntegerValue(*p));
    }
    if (const int32_t* p = sch.interval.get()) {
        h->setNew("interval", makeIntegerValue(*p));
    }
    if (const int32_t* p = sch.daytime.get()) {
        h->setNew("daytime", makeIntegerValue(*p));
    }
    if (const bool* p = sch.hostEarly.get()) {
        h->setNew("hostEarly", makeIntegerValue(*p));
    }
    if (const int32_t* p = sch.hostDelay.get()) {
        h->setNew("hostDelay", makeIntegerValue(*p));
    }
    if (const int32_t* p = sch.hostLimit.get()) {
        h->setNew("hostLimit", makeIntegerValue(*p));
    }
    if (const HostSchedule::Condition* p = sch.condition.get()) {
        h->setNew("condition", makeIntegerValue(HostSchedule::formatCondition(*p)));
    }
    if (const int32_t* p = sch.conditionTurn.get()) {
        h->setNew("condTurn", makeIntegerValue(*p));
    }
    if (const int32_t* p = sch.conditionTime.get()) {
        // FIXME: this packs the condTime in raw format, which is an error
        // when Host.TimeScale is not 60 (should be passed through getUserTimeFromTime).
        h->setNew("condTime", makeIntegerValue(*p));
    }
    return new HashValue(h);
}


// /** Parse commands for a SCHEDULESET/ADD/MOD command.
//     \param args [in] Arguments received from user
//     \param sched [in/out] Schedule to fill in
//     \param hadDaytime [out] Reports whether the schedule contained a DAYTIME option
//     \return true on success, false on failure */
server::interface::HostSchedule::Schedule
server::interface::HostScheduleServer::parseSchedule(interpreter::Arguments& args)
{
    // ex planetscentral/host/cmdsched.cc:parseSchedule
    HostSchedule::Schedule result;
    while (args.getNumArgs() > 0) {
        String_t keyword = afl::string::strUCase(toString(args.getNext()));
        if (keyword == "STOP") {
            result.type = HostSchedule::Stopped;
        } else if (keyword == "WEEKLY") {
            args.checkArgumentCountAtLeast(1);
            result.type = HostSchedule::Weekly;
            result.weekdays = toInteger(args.getNext());
        } else if (keyword == "DAILY") {
            args.checkArgumentCountAtLeast(1);
            result.type = HostSchedule::Daily;
            result.interval = toInteger(args.getNext());
        } else if (keyword == "ASAP") {
            result.type = HostSchedule::Quick;
        } else if (keyword == "MANUAL") {
            result.type = HostSchedule::Manual;
        } else if (keyword == "DAYTIME") {
            args.checkArgumentCountAtLeast(1);
            result.daytime = toInteger(args.getNext());
        } else if (keyword == "EARLY") {
            result.hostEarly = true;
        } else if (keyword == "NOEARLY") {
            result.hostEarly = false;
        } else if (keyword == "DELAY") {
            args.checkArgumentCountAtLeast(1);
            result.hostDelay = toInteger(args.getNext());
        } else if (keyword == "LIMIT") {
            args.checkArgumentCountAtLeast(1);
            result.hostLimit = toInteger(args.getNext());
        } else if (keyword == "UNTILTURN") {
            args.checkArgumentCountAtLeast(1);
            result.condition = HostSchedule::Turn;
            result.conditionTurn = toInteger(args.getNext());
        } else if (keyword == "UNTILTIME") {
            args.checkArgumentCountAtLeast(1);
            result.condition = HostSchedule::Time;
            result.conditionTime = toInteger(args.getNext());
        } else if (keyword == "FOREVER") {
            result.condition = HostSchedule::None;
        } else {
            throw std::runtime_error(INVALID_OPTION);
        }
    }
    return result;
}
