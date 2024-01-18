/**
  *  \file test/server/monitor/timeseriesloadertest.cpp
  *  \brief Test for server::monitor::TimeSeriesLoader
  */

#include "server/monitor/timeseriesloader.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/test/testrunner.hpp"
#include "server/monitor/timeseries.hpp"

/** Simple test. */
AFL_TEST("server.monitor.TimeSeriesLoader", a)
{
    server::monitor::TimeSeriesLoader testee;
    server::monitor::TimeSeries ts;
    testee.add("T", ts);

    // Provide a file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("1\t1\t11\n"
                                                       "[A]\n"
                                                       "2\t0\t22\n"
                                                       "\n"
                                                       "[T]\n"
                                                       "3\t1\t33\n"      // valid line
                                                       "4\t0\t44\n"      // valid line
                                                       "5\t5\t55\n"
                                                       "6\t1\t-66\n"     // valid line
                                                       "7\t1\n"));
    testee.load(ms);

    // Verify content
    a.checkEqual("01. size", ts.size(), 3U);

    const afl::sys::Time epoch = afl::sys::Time::fromUnixTime(0);
    afl::sys::Time time;
    bool valid;
    int32_t value;

    a.checkEqual("11. get", ts.get(0, time, valid, value), true);
    a.checkEqual("12. time diff", (time - epoch).getMilliseconds(), 3);
    a.checkEqual("13. valid", valid, true);
    a.checkEqual("14. value", value, 33);
}
