/**
  *  \file interpreter/values.hpp
  */
#ifndef C2NG_INTERPRETER_VALUES_HPP
#define C2NG_INTERPRETER_VALUES_HPP

#include "afl/data/value.hpp"
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/base/inlineoptional.hpp"
#include "afl/base/optional.hpp"

namespace interpreter {

    afl::data::Value* makeBooleanValue(int value);
    afl::data::Value* makeIntegerValue(int32_t value);
    afl::data::Value* makeFloatValue(double value);
    afl::data::Value* makeStringValue(String_t str);
    afl::data::Value* makeStringValue(const char* str);

    template<class StorageType, StorageType NullValue, class UserType>
    afl::data::Value* makeOptionalIntegerValue(afl::base::InlineOptional<StorageType,NullValue,UserType> value);

    afl::data::Value* makeOptionalStringValue(const afl::base::Optional<String_t>& value);

    int getBooleanValue(const afl::data::Value* value);

    String_t toString(const afl::data::Value* value, bool readable);

    String_t quoteString(const String_t& value);
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
