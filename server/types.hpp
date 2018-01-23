/**
  *  \file server/types.hpp
  *  \brief Convenience functions and types for server applications
  */
#ifndef C2NG_SERVER_TYPES_HPP
#define C2NG_SERVER_TYPES_HPP

#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/time.hpp"

namespace server {

    /** Shorthand for a value. */
    typedef afl::data::Value Value_t;

    /** Type for a time.
        We normally store minutes-since-epoch. */
    typedef int32_t Time_t;


    /** Interpret value as integer.
        \param v Value
        \return integer value
        \throw afl::except::InvalidDataException
        \see afl::data::Access::toInteger */
    int32_t toInteger(const Value_t* v);

    /** Interpret value as string.
        \param v Value
        \return string value
        \see afl::data::Access::toString */
    String_t toString(const Value_t* v);


    /** Make integer value.
        \param val Value
        \return newly-allocated value object */
    Value_t* makeIntegerValue(int32_t val);

    /** Make string value.
        \param str Value
        \return newly-allocated value object */
    Value_t* makeStringValue(const String_t& str);


    /** Convert system time into Time_t.
        \param t system time
        \return internal time value */
    Time_t packTime(afl::sys::Time t);

    /** Convert Time_t to system time.
        \param t internal time value
        \return system time */
    afl::sys::Time unpackTime(Time_t t);

}

#endif
