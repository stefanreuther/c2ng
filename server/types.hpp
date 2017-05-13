/**
  *  \file server/types.hpp
  */
#ifndef C2NG_SERVER_TYPES_HPP
#define C2NG_SERVER_TYPES_HPP

#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/time.hpp"

namespace server {

    typedef afl::data::Value Value_t;
    typedef int32_t Time_t;

    int32_t toInteger(const Value_t* v);
    String_t toString(const Value_t* v);

    Value_t* makeIntegerValue(int32_t val);
    Value_t* makeStringValue(const String_t& str);

    Time_t packTime(afl::sys::Time t);
    afl::sys::Time unpackTime(Time_t t);

}

#endif
