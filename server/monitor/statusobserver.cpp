/**
  *  \file server/monitor/statusobserver.cpp
  */

#include "server/monitor/statusobserver.hpp"
#include "afl/sys/time.hpp"

String_t
server::monitor::StatusObserver::getUnit()
{
    // We're measuring millisecond latencies
    return "ms";
}

server::monitor::Observer::Result
server::monitor::StatusObserver::check()
{
    uint32_t t0 = afl::sys::Time::getTickCounter();
    Status st = checkStatus();
    uint32_t t1 = afl::sys::Time::getTickCounter();
    return Result(st, t1 - t0);
}
