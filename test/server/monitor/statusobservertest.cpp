/**
  *  \file test/server/monitor/statusobservertest.cpp
  *  \brief Test for server::monitor::StatusObserver
  */

#include "server/monitor/statusobserver.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("server.monitor.StatusObserver", a)
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
    a.checkEqual("01. status", result.status, server::monitor::Observer::Running);
    a.checkEqual("02. getUnit", Tester().getUnit(), "ms");
    a.checkLessEqual("03. value", result.value, 2);
}
