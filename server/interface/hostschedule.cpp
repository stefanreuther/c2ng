/**
  *  \file server/interface/hostschedule.cpp
  */

#include "server/interface/hostschedule.hpp"

server::interface::HostSchedule::Schedule::Schedule()
    : type(),
      weekdays(),
      interval(),
      daytime(),
      hostEarly(),
      hostDelay(),
      hostLimit(),
      condition(),
      conditionTurn(),
      conditionTime()
{ }

server::interface::HostSchedule::Schedule::~Schedule()
{ }

int32_t
server::interface::HostSchedule::formatType(Type t)
{
    return int32_t(t);
}

bool
server::interface::HostSchedule::parseType(int32_t i, Type& t)
{
    switch (i) {
     case int32_t(Stopped):
     case int32_t(Weekly):
     case int32_t(Daily):
     case int32_t(Quick):
     case int32_t(Manual):
        t = static_cast<Type>(i);
        return true;
     default:
        return false;
    }
}

int32_t
server::interface::HostSchedule::formatCondition(Condition c)
{
    return int32_t(c);
}

bool
server::interface::HostSchedule::parseCondition(int32_t i, Condition& c)
{
    switch (i) {
     case int32_t(None):
     case int32_t(Turn):
     case int32_t(Time):
        c = static_cast<Condition>(i);
        return true;
     default:
        return false;
    }
}
