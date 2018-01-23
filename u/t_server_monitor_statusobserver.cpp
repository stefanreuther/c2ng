/**
  *  \file u/t_server_monitor_statusobserver.cpp
  *  \brief Test for server::monitor::StatusObserver
  */

#include "server/monitor/statusobserver.hpp"

#include "t_server_monitor.hpp"

/** Simple test. */
void
TestServerMonitorStatusObserver::testIt()
{
    class Tester : public server::monitor::StatusObserver {
     public:
        virtual String_t getName()
            { return String_t(); }
        virtual String_t getId()
            { return String_t(); }
        virtual bool handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
            { return false; }
        virtual Status checkStatus()
            { return Running; }
    };

    server::monitor::Observer::Result result = Tester().check();
    TS_ASSERT_EQUALS(result.status, server::monitor::Observer::Running);
    TS_ASSERT_EQUALS(Tester().getUnit(), "ms");
    TS_ASSERT_LESS_THAN_EQUALS(result.value, 2);
}

