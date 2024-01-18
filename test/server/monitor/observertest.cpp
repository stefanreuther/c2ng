/**
  *  \file test/server/monitor/observertest.cpp
  *  \brief Test for server::monitor::Observer
  */

#include "server/monitor/observer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.monitor.Observer")
{
    class Tester : public server::monitor::Observer {
     public:
        virtual String_t getName()
            { return String_t(); }
        virtual String_t getId()
            { return String_t(); }
        virtual String_t getUnit()
            { return String_t(); }
        virtual bool handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
            { return false; }
        virtual Result check()
            { return Result(); }
    };
    Tester t;
}
