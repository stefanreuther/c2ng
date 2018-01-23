/**
  *  \file u/t_server_monitor_status.cpp
  *  \brief Test for server::monitor::Status
  */

#include "server/monitor/status.hpp"

#include "t_server_monitor.hpp"
#include "server/monitor/statusobserver.hpp"

namespace {
    class TestObserver : public server::monitor::Observer {
     public:
        TestObserver(String_t name, Status st)
            : m_name(name),
              m_status(st)
            { }
        String_t getName()
            { return m_name; }
        String_t getId()
            { return "ID"; }
        String_t getUnit()
            { return "unit"; }
        bool handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
            { return false; }
        Result check()
            { return Result(m_status, 7); }
     private:
        String_t m_name;
        Status m_status;
    };
}

/** Test default-initialized (empty) Status. */
void
TestServerMonitorStatus::testEmpty()
{
    server::monitor::Status testee;
    afl::sys::Time time;
    TS_ASSERT_EQUALS(testee.render(time), "");
    TS_ASSERT_EQUALS(time, afl::sys::Time());
}

/** Test null observers.
    Those are ignored and do not change the outcome. */
void
TestServerMonitorStatus::testNull()
{
    server::monitor::Status testee;

    // Adding null observers does not change anything
    testee.addNewObserver(0);
    testee.addNewObserver(0);

    afl::sys::Time time;
    TS_ASSERT_EQUALS(testee.render(time), "");
    TS_ASSERT_EQUALS(time, afl::sys::Time());
}

/** Test single observer. */
void
TestServerMonitorStatus::testSingle()
{
    server::monitor::Status testee;
    testee.addNewObserver(new TestObserver("TestObserver", TestObserver::Running));

    // Initial query returns "unknown"
    afl::sys::Time time;
    TS_ASSERT_EQUALS(testee.render(time),
                     "      <div class=\"service unknown-service\" id=\"service0\">\n"
                     "        <h2>TestObserver</h2>\n"
                     "        <span class=\"status\">unknown</span>\n"
                     "      </div>\n");

    // Update once
    testee.update();
    TS_ASSERT_EQUALS(testee.render(time),
                     "      <div class=\"service active-service\" id=\"service0\">\n"
                     "        <h2>TestObserver</h2>\n"
                     "        <span class=\"status\">active</span>\n"
                     "        <span class=\"latency\">7&nbsp;ms</span>\n"
                     "      </div>\n");
}

void
TestServerMonitorStatus::testMulti()
{
    // Create a status and add multiple observers.
    server::monitor::Status testee;
    testee.addNewObserver(new TestObserver("A", TestObserver::Broken));
    testee.addNewObserver(new TestObserver("B", TestObserver::Down));
    testee.addNewObserver(new TestObserver("C", TestObserver::Running));
    testee.addNewObserver(new TestObserver("D", TestObserver::Value));

    // Update the existing observers. The fifth observer, added last, will remain in status Unknown.
    testee.update();
    testee.addNewObserver(new TestObserver("E", TestObserver::Running));

    // Verify
    afl::sys::Time time;
    TS_ASSERT_EQUALS(testee.render(time),
                     "      <div class=\"service broken-service\" id=\"service0\">\n"
                     "        <h2>A</h2>\n"
                     "        <span class=\"status\">broken</span>\n"
                     "      </div>\n"
                     "      <div class=\"service failed-service\" id=\"service1\">\n"
                     "        <h2>B</h2>\n"
                     "        <span class=\"status\">down</span>\n"
                     "      </div>\n"
                     "      <div class=\"service active-service\" id=\"service2\">\n"
                     "        <h2>C</h2>\n"
                     "        <span class=\"status\">active</span>\n"
                     "        <span class=\"latency\">7&nbsp;ms</span>\n"
                     "      </div>\n"
                     "      <div class=\"service active-service\" id=\"service3\">\n"
                     "        <h2>D</h2>\n"
                     "        <span class=\"value\">7&nbsp;unit</span>\n"
                     "      </div>\n"
                     "      <div class=\"service unknown-service\" id=\"service4\">\n"
                     "        <h2>E</h2>\n"
                     "        <span class=\"status\">unknown</span>\n"
                     "      </div>\n");
}

