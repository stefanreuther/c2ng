/**
  *  \file test/server/host/scheduletest.cpp
  *  \brief Test for server::host::Schedule
  */

#include "server/host/schedule.hpp"

#include "afl/net/redis/hashkey.hpp"
#include "afl/net/redis/integerfield.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/test/testrunner.hpp"
#include "server/host/configuration.hpp"

/** Test host data calculations. */
AFL_TEST("server.host.Schedule:getNextHost", a)
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
    a.checkEqual("01", sch.getNextHost(nov22), nov25);
    a.checkEqual("02", sch.getNextHost(nov25), nov29);

    // With 5 hour delay
    a.checkEqual("11", sch.getNextHost(nov22 + 300), nov25);
    a.checkEqual("12", sch.getNextHost(nov25 + 300), nov29);

    // With 6 hour delay
    a.checkEqual("21", sch.getNextHost(nov22 + 360), nov25);
    a.checkEqual("22", sch.getNextHost(nov25 + 360), nov29);

    // With 6:01 hour delay
    a.checkEqual("31", sch.getNextHost(nov22 + 361), nov29);
    a.checkEqual("32", sch.getNextHost(nov25 + 361), dec02);

    // Virtual host
    a.checkEqual("41", sch.getPreviousVirtualHost(nov25), nov22);
    a.checkEqual("42", sch.getPreviousVirtualHost(nov25+1), nov22);
    a.checkEqual("43", sch.getPreviousVirtualHost(nov25+360), nov22);
    a.checkEqual("44", sch.getPreviousVirtualHost(nov25+361), nov25);
    a.checkEqual("45", sch.getPreviousVirtualHost(nov25-1), nov22);
    a.checkEqual("46", sch.getPreviousVirtualHost(nov25-360), nov22);
}

/** Test data preserval. */
AFL_TEST("server.host.Schedule:data", a)
{
    server::host::Schedule sch;

    // Initial state
    a.checkEqual("01. getType",         sch.getType(), server::interface::HostSchedule::Stopped);
    a.check("02. getWeekDays",          sch.getWeekDays().empty());
    a.checkEqual("03. getInterval",     sch.getInterval(), 3);
    a.checkEqual("04. getDaytime",      sch.getDaytime(), 6*60);
    a.checkEqual("05. getHostEarly",    sch.getHostEarly(), true);
    a.checkEqual("06. getHostDelay",    sch.getHostDelay(), 30);
    a.checkEqual("07. getHostLimit",    sch.getHostLimit(), 360);
    a.checkEqual("08. getCondition",    sch.getCondition(), server::interface::HostSchedule::None);
    a.checkEqual("09. getConditionArg", sch.getConditionArg(), 0);

    // Modify (Weekly, using the Weekdays parameter)
    sch.setType(server::interface::HostSchedule::Weekly);
    sch.setWeekDays(afl::bits::SmallSet<int8_t>() + 3 + 4);
    sch.setDaytime(500);
    sch.setHostEarly(false);
    sch.setHostDelay(20);
    sch.setHostLimit(100);
    sch.setCondition(server::interface::HostSchedule::Time, 999999);

    a.checkEqual("11. getType",         sch.getType(), server::interface::HostSchedule::Weekly);
    a.checkEqual("12. getWeekDays",     sch.getWeekDays().toInteger(), 0x18U);
    a.checkEqual("13. getDaytime",      sch.getDaytime(), 500);
    a.checkEqual("14. getHostEarly",    sch.getHostEarly(), false);
    a.checkEqual("15. getHostDelay",    sch.getHostDelay(), 20);
    a.checkEqual("16. getHostLimit",    sch.getHostLimit(), 100);
    a.checkEqual("17. getCondition",    sch.getCondition(), server::interface::HostSchedule::Time);
    a.checkEqual("18. getConditionArg", sch.getConditionArg(), 999999);

    // Modify again (Daily, using the Interval parameter)
    sch.setType(server::interface::HostSchedule::Daily);
    sch.setInterval(6);
    sch.setDaytime(1400);
    sch.setHostEarly(true);
    sch.setHostDelay(15);
    sch.setHostLimit(720);
    sch.setCondition(server::interface::HostSchedule::Turn, 80);

    a.checkEqual("21. getType",         sch.getType(), server::interface::HostSchedule::Daily);
    a.checkEqual("22. getInterval",     sch.getInterval(), 6);
    a.checkEqual("23. getDaytime",      sch.getDaytime(), 1400);
    a.checkEqual("24. getHostEarly",    sch.getHostEarly(), true);
    a.checkEqual("25. getHostDelay",    sch.getHostDelay(), 15);
    a.checkEqual("26. getHostLimit",    sch.getHostLimit(), 720);
    a.checkEqual("27. getCondition",    sch.getCondition(), server::interface::HostSchedule::Turn);
    a.checkEqual("28. getConditionArg", sch.getConditionArg(), 80);
}

/** Test persisting. */
AFL_TEST("server.host.Schedule:persist", a)
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

    a.checkEqual("01", k.intField("type").get(), 2);
    a.checkEqual("02", k.intField("interval").get(), 6);
    a.checkEqual("03", k.intField("daytime").get(), 1400);
    a.checkEqual("04", k.intField("hostEarly").get(), 1);
    a.checkEqual("05", k.intField("hostDelay").get(), 15);
    a.checkEqual("06", k.intField("hostLimit").get(), 720);
    a.checkEqual("07", k.intField("condition").get(), 1);
    a.checkEqual("08", k.intField("condTurn").get(), 80);

    // Restore
    server::host::Schedule s2;
    s2.loadFrom(k);

    a.checkEqual("11. getType",         s2.getType(), server::interface::HostSchedule::Daily);
    a.checkEqual("12. getInterval",     s2.getInterval(), 6);
    a.checkEqual("13. getDaytime",      s2.getDaytime(), 1400);
    a.checkEqual("14. getHostEarly",    s2.getHostEarly(), true);
    a.checkEqual("15. getHostDelay",    s2.getHostDelay(), 15);
    a.checkEqual("16. getHostLimit",    s2.getHostLimit(), 720);
    a.checkEqual("17. getCondition",    s2.getCondition(), server::interface::HostSchedule::Turn);
    a.checkEqual("18. getConditionArg", s2.getConditionArg(), 80);
}

/** Test conditions. */
AFL_TEST("server.host.Schedule:condition", a)
{
    server::host::Schedule sch;

    // Default is no condition.
    a.check("01", !sch.isExpired(33, 8888888));

    // Turn condition expires AFTER the turn.
    sch.setCondition(server::interface::HostSchedule::Turn, 33);
    a.check("11", !sch.isExpired(32, 8888888));
    a.check("12", !sch.isExpired(33, 8888888));
    a.check("13",  sch.isExpired(34, 8888888));

    // Time condition expires AT the given time.
    sch.setCondition(server::interface::HostSchedule::Time, 8888888);
    a.check("21", !sch.isExpired(33, 8888887));
    a.check("22",  sch.isExpired(33, 8888888));
    a.check("23",  sch.isExpired(33, 8888889));
}

/** Test describe(). */
AFL_TEST("server.host.Schedule:describe", a)
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
    a.check("01",  r.type.isSame(server::interface::HostSchedule::Weekly));
    a.check("02",  r.weekdays.isSame(0x18));
    a.check("03", !r.interval.isValid());
    a.check("04",  r.daytime.isSame(500));
    a.check("05",  r.hostEarly.isSame(false));
    a.check("06",  r.hostDelay.isSame(20));
    a.check("07",  r.hostLimit.isSame(100));
    a.check("08",  r.condition.isSame(server::interface::HostSchedule::Time));
    a.check("09",  r.conditionTime.isSame(999999));
    a.check("10", !r.conditionTurn.isValid());
}

/** Test describe(). */
AFL_TEST("server.host.Schedule:describe:2", a)
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
    a.check("01",  r.type.isSame(server::interface::HostSchedule::Daily));
    a.check("02", !r.weekdays.isValid());
    a.check("03",  r.interval.isSame(6));
    a.check("04",  r.daytime.isSame(1400));
    a.check("05",  r.hostEarly.isSame(true));
    a.check("06",  r.hostDelay.isSame(15));
    a.check("07",  r.hostLimit.isSame(720));
    a.check("08",  r.condition.isSame(server::interface::HostSchedule::Turn));
    a.check("09", !r.conditionTime.isValid());
    a.check("10",  r.conditionTurn.isSame(80));
}
