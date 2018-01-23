/**
  *  \file interpreter/values.hpp
  *  \brief Interpreter Value Handling
  *
  *  This module provides a central variation point for values to be used by the interpreter.
  */
#ifndef C2NG_INTERPRETER_VALUES_HPP
#define C2NG_INTERPRETER_VALUES_HPP

#include "afl/base/inlineoptional.hpp"
#include "afl/base/optional.hpp"
#include "afl/base/types.hpp"
#include "afl/data/value.hpp"
#include "afl/string/string.hpp"

namespace interpreter {

    /** Make a tristate-boolean value from integer.
        \param value Negative for empty, zero for false, positive for true
        \return newly-created value (can be null) */
    afl::data::Value* makeBooleanValue(int value);

    /** Make integer value.
        \param value Integer value
        \return newly-created value */
    afl::data::Value* makeIntegerValue(int32_t value);

    /** Make size value.
        This is the same as makeIntegerValue, but will provide a reasonable fallback (INT_MAX) if the value is out of range.
        \param value Size value
        \return newly-created value */
    afl::data::Value* makeSizeValue(size_t value);

    /** Make float value.
        \param value Float value
        \return newly-created value */
    afl::data::Value* makeFloatValue(double value);

    /** Make string value.
        \param str String value
        \return newly-created value */
    afl::data::Value* makeStringValue(String_t str);

    /** Make string value.
        \param str String value
        \return newly-created value */
    afl::data::Value* makeStringValue(const char* str);

    /** Make optional integer value.
        Creates an integer value if the parameter is present; otherwise, returns null.
        \tparam StorageType,NullValue,UserType Parameters for the optional type
        \param value Value
        \return newly-created value (can be null) */
    template<class StorageType, StorageType NullValue, class UserType>
    afl::data::Value* makeOptionalIntegerValue(afl::base::InlineOptional<StorageType,NullValue,UserType> value);

    /** Make optional string value.
        Creates a string value if the parameter is present; otherwise, returns null.
        \param value Value
        \return newly-created value (can be null) */
    afl::data::Value* makeOptionalStringValue(const afl::base::Optional<String_t>& value);

    /** Get tristate-integer value from value.
        This is used whenever a value is used in a boolean context in a program.
        \retval -1 Input is EMPTY
        \retval 0  Input is False
        \retval +1 Input is True
        \change PCC2 <= 2.0.1 treats non-scalars as empty. We treat non-scalars as true. */
    int getBooleanValue(const afl::data::Value* value);

    /** Convert to string representation.
        This function implements stringification for simple types.
        For BaseValue descendants, calls their toString() method.
        This function is used to implement all sorts of stringification including the "Str()" function.
        \param value Value to stringify (can be null)
        \param readable True to (try to) produce something the parser can read; false for simpler/human-readable output
        \return result */
    String_t toString(const afl::data::Value* value, bool readable);

    /** Quote a string.
        Escapes the string by adding quotes and possibly backslashes
        such that the parser will read the original string again.
        \param value String
        \return quoted string */
    String_t quoteString(const String_t& value);

    /** Format a floating-point value.
        \param value Value
        \return formatted value */
    String_t formatFloat(double value);

}

template<class StorageType, StorageType NullValue, class UserType>
afl::data::Value*
interpreter::makeOptionalIntegerValue(afl::base::InlineOptional<StorageType,NullValue,UserType> value)
{
    UserType i;
    if (value.get(i)) {
        return makeIntegerValue(i);
    } else {
        return 0;
    }
}

#endif
