/**
  *  \file test/server/monitor/timeserieswritertest.cpp
  *  \brief Test for server::monitor::TimeSeriesWriter
  */

#include "server/monitor/timeserieswriter.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/string/string.hpp"
#include "afl/test/testrunner.hpp"
#include "server/monitor/timeseries.hpp"
#include "util/io.hpp"

using afl::sys::Time;

/** Test writing empty file.
    If no add() is called, the resulting file must be empty. */
AFL_TEST("server.monitor.TimeSeriesWriter:empty", a)
{
    server::monitor::TimeSeriesWriter testee;

    afl::io::InternalStream out;
    testee.save(out);

    a.checkEqual("01. getSize", out.getSize(), 0U);
}

/** Test writing normal file. */
AFL_TEST("server.monitor.TimeSeriesWriter:normal", a)
{
    server::monitor::TimeSeriesWriter testee;

    // One time series
    server::monitor::TimeSeries ta;
    ta.add(Time::fromUnixTime(22), true, 10);
    ta.add(Time::fromUnixTime(25), false, 11);
    ta.add(Time::fromUnixTime(29), true, 12);
    testee.add("ONE", ta);

    // Another time series
    server::monitor::TimeSeries tb;
    tb.add(Time::fromUnixTime(75), true, -9);
    tb.add(Time::fromUnixTime(77), true, +8);
    testee.add("TWO", tb);

    // Verify
    afl::io::InternalStream out;
    testee.save(out);

    String_t content = util::normalizeLinefeeds(out.getContent());
    a.checkEqual("01. content", content,
                 "[ONE]\n"
                 "22000\t1\t10\n"
                 "25000\t0\t11\n"
                 "29000\t1\t12\n"
                 "[TWO]\n"
                 "75000\t1\t-9\n"
                 "77000\t1\t8\n");
}
