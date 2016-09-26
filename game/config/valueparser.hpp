/**
  *  \file game/config/valueparser.hpp
  *  \brief Interface game::config::ValueParser
  */
#ifndef C2NG_GAME_CONFIG_VALUEPARSER_HPP
#define C2NG_GAME_CONFIG_VALUEPARSER_HPP

#include "afl/string/string.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"

namespace game { namespace config {

    /** Value parser, base class.
        Descendants of this class parse strings into integers according to particular rules. */
    class ValueParser {
     public:
        /** Virtual destructor. */
        virtual ~ValueParser();

        /** Parse a single element into an integer.
            This is the inverse to toString().
            \param value String to parse
            \return result value
            \throw std::range_error if value is invalid */
        virtual int32_t parse(String_t value) const = 0;

        /** Format a single integer to a string.
            This is the inverse to parse().
            \param value Value
            \return string */
        virtual String_t toString(int32_t value) const = 0;

        /** Parse comma-separated list into array.
            The array is completely filled with parsed values.
            If the string contains fewer elements than the array, the last element is repeated.
            If the string contains more elements than the array, excess elements are ignored.

            This is the inverse to toStringArray().

            \param value [in] String value
            \param array [out] Array */
        void parseArray(String_t value, afl::base::Memory<int32_t> array) const;

        /** Convert array to string.
            Produces a comma-separated list of values.

            This is the inverse to parseArray().

            \param array [in] Array
            \return String value */
        String_t toStringArray(afl::base::Memory<const int32_t> array) const;
    };

} }

#endif
