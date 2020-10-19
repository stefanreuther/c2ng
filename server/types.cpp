/**
  *  \file server/types.cpp
  *  \brief Convenience functions and types for server applications
  */

#include "server/types.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/access.hpp"

namespace {
    /* Time scale.
       We are storing times in minutes-since-unix-epoch.
       This is enough precision for a forum, and avoids 32/64-bit time_t trouble;
       these times go up to the year 6053 before overflowing. */
    const int32_t TIME_SCALE = 60;
}

int32_t
server::toInteger(const Value_t* v)
{
    // ex planetscentral/dbclient/connection.h:toIntResult (sort-of)
    return afl::data::Access(v).toInteger();
}

String_t
server::toString(const Value_t* v)
{
    // ex planetscentral/dbclient/connection.h:toStringResult
    return afl::data::Access(v).toString();
}

server::Value_t*
server::makeIntegerValue(int32_t val)
{
    return new afl::data::IntegerValue(val);
}

server::Value_t*
server::makeStringValue(const String_t& str)
{
    return new afl::data::StringValue(str);
}

server::Time_t
server::packTime(afl::sys::Time t)
{
    return Time_t(t.getUnixTime() / TIME_SCALE);
}

afl::sys::Time
server::unpackTime(Time_t t)
{
    return afl::sys::Time::fromUnixTime(uint64_t(t) * TIME_SCALE);
}
