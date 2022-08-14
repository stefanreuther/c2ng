/**
  *  \file server/interface/hosttool.cpp
  *  \brief Interface server::interface::HostTool
  */

#include "server/interface/hosttool.hpp"

const char*
server::interface::HostTool::toString(Area a)
{
    switch (a) {
     case Host:     return "HOST";
     case ShipList: return "SHIPLIST";
     case Master:   return "MASTER";
     case Tool:     return "TOOL";
    }
    return 0;
}
