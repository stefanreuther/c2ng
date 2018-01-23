/**
  *  \file u/t_server_monitor_observer.cpp
  *  \brief Test for server::monitor::Observer
  */

#include "server/monitor/observer.hpp"

#include "t_server_monitor.hpp"

/** Interface test. */
void
TestServerMonitorObserver::testInterface()
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

