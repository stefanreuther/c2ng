/**
  *  \file test/server/monitor/timeseriestest.cpp
  *  \brief Test for server::monitor::TimeSeries
  */

#include "server/monitor/timeseries.hpp"
#include "afl/test/testrunner.hpp"

/** Test add(), size(), and get(). */
AFL_TEST("server.monitor.TimeSeries:basics", a)
{
    server::monitor::TimeSeries t;
    t.add(afl::sys::Time::fromUnixTime(10), true, 7);
    t.add(afl::sys::Time::fromUnixTime(12), true, 8);
    t.add(afl::sys::Time::fromUnixTime(14), false, 9);
    t.add(afl::sys::Time::fromUnixTime(16), true, 10);

    a.checkEqual("01. size", t.size(), 4U);

    afl::sys::Time timeOut;
    bool validOut;
    int32_t valueOut;
    a.checkEqual("11. get", t.get(0, timeOut, validOut, valueOut), true);
    a.checkEqual("12. getUnixTime", timeOut.getUnixTime(), 10);
    a.checkEqual("13. validOut", validOut, true);
    a.checkEqual("14. valueOut", valueOut, 7);

    a.checkEqual("21. get", t.get(2, timeOut, validOut, valueOut), true);
    a.checkEqual("22. getUnixTime", timeOut.getUnixTime(), 14);
    a.checkEqual("23. validOut", validOut, false);
    a.checkEqual("24. valueOut", valueOut, 9);

    a.checkEqual("31. get", t.get(3, timeOut, validOut, valueOut), true);
    a.checkEqual("32. getUnixTime", timeOut.getUnixTime(), 16);
    a.checkEqual("33. validOut", validOut, true);
    a.checkEqual("34. valueOut", valueOut, 10);

    a.checkEqual("41. get", t.get(0, timeOut, valueOut), true);
    a.checkEqual("42. getUnixTime", timeOut.getUnixTime(), 10);
    a.checkEqual("43. valueOut", valueOut, 7);

    a.checkEqual("51. get", t.get(2, timeOut, valueOut), false);

    a.checkEqual("61. get", t.get(4, timeOut, validOut, valueOut), false);
    a.checkEqual("62. get", t.get(4, timeOut, valueOut), false);
}

/** Test compact(). */
AFL_TEST("server.monitor.TimeSeries:compact", a)
{
    // Create 2000 elements
    server::monitor::TimeSeries t;
    for (int i = 1; i <= 2000; ++i) {
        t.add(afl::sys::Time::fromUnixTime(i), true, i);
    }

    // Compact down to 1500
    t.compact(0, 1000, 2);

    // Verify
    a.checkEqual("01. size", t.size(), 1500U);

    afl::sys::Time timeOut;
    int32_t valueOut;
    a.checkEqual("11. get", t.get(1499, timeOut, valueOut), true);
    a.checkEqual("12. getUnixTime", timeOut.getUnixTime(), 2000);
    a.checkEqual("13. valueOut", valueOut, 2000);

    a.checkEqual("21. get", t.get(500, timeOut, valueOut), true);
    a.checkEqual("22. getUnixTime", timeOut.getUnixTime(), 1001);
    a.checkEqual("23. valueOut", valueOut, 1001);

    a.checkEqual("31. get", t.get(0, timeOut, valueOut), true);
    a.checkEqual("32. getUnixTime", timeOut.getUnixTime(), 1);
    a.checkEqual("33. valueOut", valueOut, 1);

    a.checkEqual("41. get", t.get(100, timeOut, valueOut), true);
    a.checkEqual("42. getUnixTime", timeOut.getUnixTime(), 201);
    a.checkEqual("43. valueOut", valueOut, 201);
}

/** Test render(). */
AFL_TEST("server.monitor.TimeSeries:render", a)
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
    a.checkEqual("01. size", t.size(), 2000U);

    // Render
    String_t result = t.render(500, 500);

    // There must be 4 plot segments
    a.checkDifferent("11. plot0", result.find("plot0"), String_t::npos);
    a.checkDifferent("12. plot1", result.find("plot1"), String_t::npos);
    a.checkDifferent("13. plot2", result.find("plot2"), String_t::npos);
    a.checkDifferent("14. plot3", result.find("plot3"), String_t::npos);
    a.checkEqual("15. plot4", result.find("plot4"), String_t::npos);

    // Verify line lengths. There must not be a line longer than 2000 characters.
    // The origin of this limit is that we're limiting paths to 100 points, and each point requires a dozen bytes.
    do {
        a.checkLessEqual("21. line", afl::string::strFirst(result, "\n").size(), 2000U);
    } while (afl::string::strRemove(result, "\n"));
}

/** Test render() on empty series. */
AFL_TEST("server.monitor.TimeSeries:render:empty", a)
{
    // Render
    String_t result = server::monitor::TimeSeries().render(400, 200);

    // Verify: this produces just a coordinate grid
    a.checkEqual("01. result", result,
                 "<text x=\"45\" y=\"10\" text-anchor=\"end\" class=\"axes\">5</text>\n"
                 "<text x=\"45\" y=\"100\" text-anchor=\"end\" class=\"axes\">0</text>\n"
                 "<path d=\"M50,0 L50,100 L400,100\" class=\"axes\" />\n");
}

/** Test render() on simple case. */
AFL_TEST("server.monitor.TimeSeries:render:simple", a)
{
    server::monitor::TimeSeries t;
    t.add(afl::sys::Time::fromUnixTime(10), true, 10);
    t.add(afl::sys::Time::fromUnixTime(70), true, 20);
    t.add(afl::sys::Time::fromUnixTime(130), true, 10);
    t.add(afl::sys::Time::fromUnixTime(140), true, 30);
    t.add(afl::sys::Time::fromUnixTime(150), true, 10);
    t.add(afl::sys::Time::fromUnixTime(160), true, 20);

    a.checkEqual("01. result", t.render(500, 200),
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
AFL_TEST("server.monitor.TimeSeries:render:age", a)
{
    server::monitor::TimeSeries t;

    t.add(afl::sys::Time::fromUnixTime(10),     true, 10);
    t.add(afl::sys::Time::fromUnixTime(172800), true, 10);   // + 2d
    t.add(afl::sys::Time::fromUnixTime(180000), true, 10);   // + 2h
    t.add(afl::sys::Time::fromUnixTime(180060), true, 10);   // + 1min
    t.add(afl::sys::Time::fromUnixTime(180061), true, 10);   // + 1s
    t.add(afl::sys::Time::fromUnixTime(180062), true, 10);   // + 1s

    a.checkEqual("01. result", t.render(300, 200),
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
