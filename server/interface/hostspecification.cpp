/**
  *  \file server/interface/hostspecification.cpp
  *  \brief Interface server::interface::HostSpecification
  */

#include "server/interface/hostspecification.hpp"

String_t
server::interface::HostSpecification::formatFormat(Format fmt)
{
    switch (fmt) {
     case Direct:     return "direct";
     case JsonString: return "json";
    }
    return String_t();
}

afl::base::Optional<server::interface::HostSpecification::Format>
server::interface::HostSpecification::parseFormat(const String_t& str)
{
    if (str == "direct") {
        return Direct;
    } else if (str == "json") {
        return JsonString;
    } else {
        return afl::base::Nothing;
    }
}
