/**
  *  \file u/t_server_host_schedule.cpp
  *  \brief Test for server::host::Schedule
  */

#include "server/host/schedule.hpp"

#include "t_server_host.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "server/host/configuration.hpp"

/** Test host data calculations. */
void
TestServerHostSchedule::testHostDate()
{
    server::host::Schedule sch;
    sch.setType(server::interface::HostSchedule::Weekly);
    sch.setWeekDays(afl::bits::SmallSet<int8_t>::fromInteger(17));  // Thu+Sun
    sch.setDaytime(360);                                            // 6:00
    sch.setHostEarly(1);
    sch.setHostDelay(30);
    sch.setHostLimit(360);                                          // 6 hours

    const int32_t nov22 = 22559400;  // Thu Nov 22 06:00:00 2012
    const int32_t nov25 = 22563720;  // Sun Nov 25 06:00:00 2012
    const int32_t nov29 = 22569480;  // Thu Nov 29 06:00:00 2012
    const int32_t dec02 = 22573800;  // Sun Dec  2 06:00:00 2012

    // Regular progression
    TS_ASSERT_EQUALS(sch.getNextHost(nov22), nov25);
    TS_ASSERT_EQUALS(sch.getNextHost(nov25), nov29);

    // With 5 hour delay
    TS_ASSERT_EQUALS(sch.getNextHost(nov22 + 300), nov25);
    TS_ASSERT_EQUALS(sch.getNextHost(nov25 + 300), nov29);

    // With 6 hour delay
    TS_ASSERT_EQUALS(sch.getNextHost(nov22 + 360), nov25);
    TS_ASSERT_EQUALS(sch.getNextHost(nov25 + 360), nov29);

    // With 6:01 hour delay
    TS_ASSERT_EQUALS(sch.getNextHost(nov22 + 361), nov29);
    TS_ASSERT_EQUALS(sch.getNextHost(nov25 + 361), dec02);

    // Virtual host
    TS_ASSERT_EQUALS(sch.getPreviousVirtualHost(nov25), nov22);
    TS_ASSERT_EQUALS(sch.getPreviousVirtualHost(nov25+1), nov22);
    TS_ASSERT_EQUALS(sch.getPreviousVirtualHost(nov25+360), nov22);
    TS_ASSERT_EQUALS(sch.getPreviousVirtualHost(nov25+361), nov25);
    TS_ASSERT_EQUALS(sch.getPreviousVirtualHost(nov25-1), nov22);
    TS_ASSERT_EQUALS(sch.getPreviousVirtualHost(nov25-360), nov22);
}

/** Test data preserval. */
void
TestServerHostSchedule::testData()
{
    server::host::Schedule sch;

    // Initial state
    TS_ASSERT_EQUALS(sch.getType(), server::interface::HostSchedule::Stopped);
    TS_ASSERT(sch.getWeekDays().empty());
    TS_ASSERT_EQUALS(sch.getInterval(), 3);
    TS_ASSERT_EQUALS(sch.getDaytime(), 6*60);
    TS_ASSERT_EQUALS(sch.getHostEarly(), true);
    TS_ASSERT_EQUALS(sch.getHostDelay(), 30);
    TS_ASSERT_EQUALS(sch.getHostLimit(), 360);
    TS_ASSERT_EQUALS(sch.getCondition(), server::interface::HostSchedule::None);
    TS_ASSERT_EQUALS(sch.getConditionArg(), 0);

    // Modify (Weekly, using the Weekdays parameter)
    sch.setType(server::interface::HostSchedule::Weekly);
    sch.setWeekDays(afl::bits::SmallSet<int8_t>() + 3 + 4);
    sch.setDaytime(500);
    sch.setHostEarly(false);
    sch.setHostDelay(20);
    sch.setHostLimit(100);
    sch.setCondition(server::interface::HostSchedule::Time, 999999);

    TS_ASSERT_EQUALS(sch.getType(), server::interface::HostSchedule::Weekly);
    TS_ASSERT_EQUALS(sch.getWeekDays().toInteger(), 0x18U);
    TS_ASSERT_EQUALS(sch.getDaytime(), 500);
    TS_ASSERT_EQUALS(sch.getHostEarly(), false);
    TS_ASSERT_EQUALS(sch.getHostDelay(), 20);
    TS_ASSERT_EQUALS(sch.getHostLimit(), 100);
    TS_ASSERT_EQUALS(sch.getCondition(), server::interface::HostSchedule::Time);
    TS_ASSERT_EQUALS(sch.getConditionArg(), 999999);

    // Modify again (Daily, using the Interval parameter)
    sch.setType(server::interface::HostSchedule::Daily);
    sch.setInterval(6);
    sch.setDaytime(1400);
    sch.setHostEarly(true);
    sch.setHostDelay(15);
    sch.setHostLimit(720);
    sch.setCondition(server::interface::HostSchedule::Turn, 80);

    TS_ASSERT_EQUALS(sch.getType(), server::interface::HostSchedule::Daily);
    TS_ASSERT_EQUALS(sch.getInterval(), 6);
    TS_ASSERT_EQUALS(sch.getDaytime(), 1400);
    TS_ASSERT_EQUALS(sch.getHostEarly(), true);
    TS_ASSERT_EQUALS(sch.getHostDelay(), 15);
    TS_ASSERT_EQUALS(sch.getHostLimit(), 720);
    TS_ASSERT_EQUALS(sch.getCondition(), server::interface::HostSchedule::Turn);
    TS_ASSERT_EQUALS(sch.getConditionArg(), 80);
}

/** Test persisting. */
void
TestServerHostSchedule::testPersist()
{
    // Create a schedule
    server::host::Schedule sch;
    sch.setType(server::interface::HostSchedule::Daily);
    sch.setInterval(6);
    sch.setDaytime(1400);
    sch.setHostEarly(true);
    sch.setHostDelay(15);
    sch.setHostLimit(720);
    sch.setCondition(server::interface::HostSchedule::Turn, 80);

    // Save into a DB
    afl::net::redis::InternalDatabase db;
    afl::net::redis::HashKey k(db, "x");
    sch.saveTo(k);

    TS_ASSERT_EQUALS(k.intField("type").get(), 2);
    TS_ASSERT_EQUALS(k.intField("interval").get(), 6);
    TS_ASSERT_EQUALS(k.intField("daytime").get(), 1400);
    TS_ASSERT_EQUALS(k.intField("hostEarly").get(), 1);
    TS_ASSERT_EQUALS(k.intField("hostDelay").get(), 15);
    TS_ASSERT_EQUALS(k.intField("hostLimit").get(), 720);
    TS_ASSERT_EQUALS(k.intField("condition").get(), 1);
    TS_ASSERT_EQUALS(k.intField("condTurn").get(), 80);

    // Restore
    server::host::Schedule s2;
    s2.loadFrom(k);

    TS_ASSERT_EQUALS(s2.getType(), server::interface::HostSchedule::Daily);
    TS_ASSERT_EQUALS(s2.getInterval(), 6);
    TS_ASSERT_EQUALS(s2.getDaytime(), 1400);
    TS_ASSERT_EQUALS(s2.getHostEarly(), true);
    TS_ASSERT_EQUALS(s2.getHostDelay(), 15);
    TS_ASSERT_EQUALS(s2.getHostLimit(), 720);
    TS_ASSERT_EQUALS(s2.getCondition(), server::interface::HostSchedule::Turn);
    TS_ASSERT_EQUALS(s2.getConditionArg(), 80);
}

/** Test conditions. */
void
TestServerHostSchedule::testCondition()
{
    server::host::Schedule sch;

    // Default is no condition.
    TS_ASSERT(!sch.isExpired(33, 8888888));

    // Turn condition expires AFTER the turn.
    sch.setCondition(server::interface::HostSchedule::Turn, 33);
    TS_ASSERT(!sch.isExpired(32, 8888888));
    TS_ASSERT(!sch.isExpired(33, 8888888));
    TS_ASSERT( sch.isExpired(34, 8888888));

    // Time condition expires AT the given time.
    sch.setCondition(server::interface::HostSchedule::Time, 8888888);
    TS_ASSERT(!sch.isExpired(33, 8888887));
    TS_ASSERT( sch.isExpired(33, 8888888));
    TS_ASSERT( sch.isExpired(33, 8888889));
}

/** Test describe(). */
void
TestServerHostSchedule::testDescribe()
{
    // Create schedule
    server::host::Schedule sch;
    sch.setType(server::interface::HostSchedule::Weekly);
    sch.setWeekDays(afl::bits::SmallSet<int8_t>() + 3 + 4);
    sch.setDaytime(500);
    sch.setHostEarly(false);
    sch.setHostDelay(20);
    sch.setHostLimit(100);
    sch.setCondition(server::interface::HostSchedule::Time, 999999);

    // Describe
    server::host::Configuration config;
    server::interface::HostSchedule::Schedule r = sch.describe(config);

    // Validate
    TS_ASSERT( r.type.isSame(server::interface::HostSchedule::Weekly));
    TS_ASSERT( r.weekdays.isSame(0x18));
    TS_ASSERT(!r.interval.isValid());
    TS_ASSERT( r.daytime.isSame(500));
    TS_ASSERT( r.hostEarly.isSame(false));
    TS_ASSERT( r.hostDelay.isSame(20));
    TS_ASSERT( r.hostLimit.isSame(100));
    TS_ASSERT( r.condition.isSame(server::interface::HostSchedule::Time));
    TS_ASSERT( r.conditionTime.isSame(999999));
    TS_ASSERT(!r.conditionTurn.isValid());
}

/** Test describe(). */
void
TestServerHostSchedule::testDescribe2()
{
    // Create schedule
    server::host::Schedule sch;
    sch.setType(server::interface::HostSchedule::Daily);
    sch.setInterval(6);
    sch.setDaytime(1400);
    sch.setHostEarly(true);
    sch.setHostDelay(15);
    sch.setHostLimit(720);
    sch.setCondition(server::interface::HostSchedule::Turn, 80);

    // Describe
    server::host::Configuration config;
    server::interface::HostSchedule::Schedule r = sch.describe(config);

    // Validate
    TS_ASSERT( r.type.isSame(server::interface::HostSchedule::Daily));
    TS_ASSERT(!r.weekdays.isValid());
    TS_ASSERT( r.interval.isSame(6));
    TS_ASSERT( r.daytime.isSame(1400));
    TS_ASSERT( r.hostEarly.isSame(true));
    TS_ASSERT( r.hostDelay.isSame(15));
    TS_ASSERT( r.hostLimit.isSame(720));
    TS_ASSERT( r.condition.isSame(server::interface::HostSchedule::Turn));
    TS_ASSERT(!r.conditionTime.isValid());
    TS_ASSERT( r.conditionTurn.isSame(80));
}

