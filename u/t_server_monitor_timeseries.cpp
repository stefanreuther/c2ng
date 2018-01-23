/**
  *  \file u/t_server_monitor_timeseries.cpp
  *  \brief Test for server::monitor::TimeSeries
  */

#include "server/monitor/timeseries.hpp"

#include "t_server_monitor.hpp"

/** Test add(), size(), and get(). */
void
TestServerMonitorTimeSeries::testAddGet()
{
    server::monitor::TimeSeries t;
    t.add(afl::sys::Time::fromUnixTime(10), true, 7);
    t.add(afl::sys::Time::fromUnixTime(12), true, 8);
    t.add(afl::sys::Time::fromUnixTime(14), false, 9);
    t.add(afl::sys::Time::fromUnixTime(16), true, 10);

    TS_ASSERT_EQUALS(t.size(), 4U);

    afl::sys::Time timeOut;
    bool validOut;
    int32_t valueOut;
    TS_ASSERT_EQUALS(t.get(0, timeOut, validOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 10);
    TS_ASSERT_EQUALS(validOut, true);
    TS_ASSERT_EQUALS(valueOut, 7);

    TS_ASSERT_EQUALS(t.get(2, timeOut, validOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 14);
    TS_ASSERT_EQUALS(validOut, false);
    TS_ASSERT_EQUALS(valueOut, 9);

    TS_ASSERT_EQUALS(t.get(3, timeOut, validOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 16);
    TS_ASSERT_EQUALS(validOut, true);
    TS_ASSERT_EQUALS(valueOut, 10);

    TS_ASSERT_EQUALS(t.get(0, timeOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 10);
    TS_ASSERT_EQUALS(valueOut, 7);

    TS_ASSERT_EQUALS(t.get(2, timeOut, valueOut), false);

    TS_ASSERT_EQUALS(t.get(4, timeOut, validOut, valueOut), false);
    TS_ASSERT_EQUALS(t.get(4, timeOut, valueOut), false);
}

/** Test compact(). */
void
TestServerMonitorTimeSeries::testCompact()
{
    // Create 2000 elements
    server::monitor::TimeSeries t;
    for (int i = 1; i <= 2000; ++i) {
        t.add(afl::sys::Time::fromUnixTime(i), true, i);
    }

    // Compact down to 1500
    t.compact(0, 1000, 2);

    // Verify
    TS_ASSERT_EQUALS(t.size(), 1500U);

    afl::sys::Time timeOut;
    int32_t valueOut;
    TS_ASSERT_EQUALS(t.get(1499, timeOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 2000);
    TS_ASSERT_EQUALS(valueOut, 2000);

    TS_ASSERT_EQUALS(t.get(500, timeOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 1001);
    TS_ASSERT_EQUALS(valueOut, 1001);

    TS_ASSERT_EQUALS(t.get(0, timeOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 1);
    TS_ASSERT_EQUALS(valueOut, 1);

    TS_ASSERT_EQUALS(t.get(100, timeOut, valueOut), true);
    TS_ASSERT_EQUALS(timeOut.getUnixTime(), 201);
    TS_ASSERT_EQUALS(valueOut, 201);
}

/** Test render(). */
void
TestServerMonitorTimeSeries::testRender()
{
    // Create 2000 elements
    server::monitor::TimeSeries t;
    int counter = 0;
    for (int i = 0; i < 2000; ++i) {
        ++counter;
        t.add(afl::sys::Time::fromUnixTime(counter), true, counter);
    }

    // Compact three times, always fill up again
    for (int q = 0; q < 3; ++q) {
        t.compact(0, 1000, 2);
        for (int i = 0; i < 500; ++i) {
            ++counter;
            t.add(afl::sys::Time::fromUnixTime(counter), true, counter);
        }
    }
    TS_ASSERT_EQUALS(t.size(), 2000U);

    // Render
    String_t result = t.render(500, 500);

    // There must be 4 plot segments
    TS_ASSERT_DIFFERS(result.find("plot0"), String_t::npos);
    TS_ASSERT_DIFFERS(result.find("plot1"), String_t::npos);
    TS_ASSERT_DIFFERS(result.find("plot2"), String_t::npos);
    TS_ASSERT_DIFFERS(result.find("plot3"), String_t::npos);
    TS_ASSERT_EQUALS(result.find("plot4"), String_t::npos);

    // Verify line lengths. There must not be a line longer than 2000 characters.
    // The origin of this limit is that we're limiting paths to 100 points, and each point requires a dozen bytes.
    do {
        TS_ASSERT_LESS_THAN_EQUALS(afl::string::strFirst(result, "\n").size(), 2000U);
    } while (afl::string::strRemove(result, "\n"));
}

/** Test render() on empty series. */
void
TestServerMonitorTimeSeries::testRenderEmpty()
{
    // Render
    String_t result = server::monitor::TimeSeries().render(400, 200);

    // Verify: this produces just a coordinate grid
    TS_ASSERT_EQUALS(result,
                     "<text x=\"45\" y=\"10\" text-anchor=\"end\" class=\"axes\">5</text>\n"
                     "<text x=\"45\" y=\"100\" text-anchor=\"end\" class=\"axes\">0</text>\n"
                     "<path d=\"M50,0 L50,100 L400,100\" class=\"axes\" />\n");
}

/** Test render() on simple case. */
void
TestServerMonitorTimeSeries::testRenderSimple()
{
    server::monitor::TimeSeries t;
    t.add(afl::sys::Time::fromUnixTime(10), true, 10);
    t.add(afl::sys::Time::fromUnixTime(70), true, 20);
    t.add(afl::sys::Time::fromUnixTime(130), true, 10);
    t.add(afl::sys::Time::fromUnixTime(140), true, 30);
    t.add(afl::sys::Time::fromUnixTime(150), true, 10);
    t.add(afl::sys::Time::fromUnixTime(160), true, 20);

    TS_ASSERT_EQUALS(t.render(500, 200),
                     "<text x=\"45\" y=\"10\" text-anchor=\"end\" class=\"axes\">50</text>\n"
                     "<text x=\"45\" y=\"100\" text-anchor=\"end\" class=\"axes\">0</text>\n"
                     "<path d=\"M50,0 L50,100 L500,100\" class=\"axes\" />\n"
                     "<text x=\"275\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 275,105)\" class=\"axes\">now</text>\n"
                     "<text x=\"230\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 230,105)\" class=\"axes\">-10 s</text>\n"
                     "<text x=\"185\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 185,105)\" class=\"axes\">-20 s</text>\n"
                     "<text x=\"140\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 140,105)\" class=\"axes\">-30 s</text>\n"
                     "<text x=\"95\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 95,105)\" class=\"axes\">-2 min</text>\n"
                     "<text x=\"50\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 50,105)\" class=\"axes\">-3 min</text>\n"
                     "<path d=\"M140,80 L185,40 L230,80 L275,60\" class=\"plot plot0\" />\n"
                     "<path d=\"M50,80 L95,60 L140,80\" class=\"plot plot1\" />\n");
}

/** Test render(), verify age formatting. */
void
TestServerMonitorTimeSeries::testRenderAges()
{
    server::monitor::TimeSeries t;

    t.add(afl::sys::Time::fromUnixTime(10),     true, 10);
    t.add(afl::sys::Time::fromUnixTime(172800), true, 10);   // + 2d
    t.add(afl::sys::Time::fromUnixTime(180000), true, 10);   // + 2h
    t.add(afl::sys::Time::fromUnixTime(180060), true, 10);   // + 1min
    t.add(afl::sys::Time::fromUnixTime(180061), true, 10);   // + 1s
    t.add(afl::sys::Time::fromUnixTime(180062), true, 10);   // + 1s

    TS_ASSERT_EQUALS(t.render(300, 200),
                     "<text x=\"45\" y=\"10\" text-anchor=\"end\" class=\"axes\">10</text>\n"
                     "<text x=\"45\" y=\"100\" text-anchor=\"end\" class=\"axes\">0</text>\n"
                     "<path d=\"M50,0 L50,100 L300,100\" class=\"axes\" />\n"
                     "<text x=\"175\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 175,105)\" class=\"axes\">now</text>\n"
                     "<text x=\"150\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 150,105)\" class=\"axes\">-1 s</text>\n"
                     "<text x=\"125\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 125,105)\" class=\"axes\">-2 s</text>\n"
                     "<text x=\"100\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 100,105)\" class=\"axes\">-1 min</text>\n"
                     "<text x=\"75\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 75,105)\" class=\"axes\">-2 h</text>\n"
                     "<text x=\"50\" y=\"105\" text-anchor=\"end\" transform=\"rotate(-90 50,105)\" class=\"axes\">-2 d</text>\n"
                     "<path d=\"M125,0 L150,0 L175,0\" class=\"plot plot0\" />\n"
                     "<path d=\"M75,0 L100,0 L125,0\" class=\"plot plot1\" />\n"
                     "<path d=\"M50,0 L75,0\" class=\"plot plot2\" />\n");
    
}

