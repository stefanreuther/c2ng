/**
  *  \file u/t_server_monitor_timeserieswriter.cpp
  *  \brief Test for server::monitor::TimeSeriesWriter
  */

#include "server/monitor/timeserieswriter.hpp"

#include "t_server_monitor.hpp"
#include "afl/io/internalstream.hpp"
#include "server/monitor/timeseries.hpp"
#include "afl/string/string.hpp"

using afl::sys::Time;

/** Test writing empty file.
    If no add() is called, the resulting file must be empty. */
void
TestServerMonitorTimeSeriesWriter::testEmpty()
{
    server::monitor::TimeSeriesWriter testee;

    afl::io::InternalStream out;
    testee.save(out);

    TS_ASSERT_EQUALS(out.getSize(), 0U);
}

/** Test writing normal file. */
void
TestServerMonitorTimeSeriesWriter::testNormal()
{
    server::monitor::TimeSeriesWriter testee;

    // One time series
    server::monitor::TimeSeries a;
    a.add(Time::fromUnixTime(22), true, 10);
    a.add(Time::fromUnixTime(25), false, 11);
    a.add(Time::fromUnixTime(29), true, 12);
    testee.add("ONE", a);

    // Another time series
    server::monitor::TimeSeries b;
    b.add(Time::fromUnixTime(75), true, -9);
    b.add(Time::fromUnixTime(77), true, +8);
    testee.add("TWO", b);

    // Verify
    afl::io::InternalStream out;
    testee.save(out);

    String_t content = afl::string::fromBytes(out.getContent());
    String_t::size_type n;
    while ((n = content.find('\r')) != String_t::npos) {
        content.erase(n, 1);
    }
    TS_ASSERT_EQUALS(content,
                     "[ONE]\n"
                     "22000\t1\t10\n"
                     "25000\t0\t11\n"
                     "29000\t1\t12\n"
                     "[TWO]\n"
                     "75000\t1\t-9\n"
                     "77000\t1\t8\n");
}

