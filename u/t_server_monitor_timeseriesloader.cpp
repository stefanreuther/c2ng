/**
  *  \file u/t_server_monitor_timeseriesloader.cpp
  *  \brief Test for server::monitor::TimeSeriesLoader
  */

#include "server/monitor/timeseriesloader.hpp"

#include "t_server_monitor.hpp"
#include "afl/io/constmemorystream.hpp"
#include "server/monitor/timeseries.hpp"

/** Simple test. */
void
TestServerMonitorTimeSeriesLoader::testIt()
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
    TS_ASSERT_EQUALS(ts.size(), 3U);

    const afl::sys::Time epoch = afl::sys::Time::fromUnixTime(0);
    afl::sys::Time time;
    bool valid;
    int32_t value;

    TS_ASSERT_EQUALS(ts.get(0, time, valid, value), true);
    TS_ASSERT_EQUALS((time - epoch).getMilliseconds(), 3);
    TS_ASSERT_EQUALS(valid, true);
    TS_ASSERT_EQUALS(value, 33);
}

