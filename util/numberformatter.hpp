/**
  *  \file util/numberformatter.hpp
  *  \brief Class util::NumberFormatter
  */
#ifndef C2NG_UTIL_NUMBERFORMATTER_HPP
#define C2NG_UTIL_NUMBERFORMATTER_HPP

#include "afl/string/string.hpp"
#include "afl/base/inlineoptional.hpp"

namespace util {

    /** Number formatter.
        Allows formatting numbers according to user configuration.
        This is a value class that can be passed between components (in particular, from game to UI thread). */
    class NumberFormatter {
     public:
        /** Constructor.
            \param useThousandsSeparator Enable thousands-separators
            \param useClans Format clans as clans (default: population) */
        NumberFormatter(bool useThousandsSeparator, bool useClans);

        /** Format a number.
            \param n Number
            \return Formatted number, using user's settings for Display_ThousandsSep. */
        String_t formatNumber(int32_t n) const;

        /** Format a difference.
            Like formatNumber(), but always includes a "+" or "-" if the number is nonzero.
            \param n Number
            \return Formatted number, using user's settings for Display_ThousandsSep. */
        String_t formatDifference(int32_t n) const;

        /** Format an optional number.
            \param value Number
            \return Formatted value, using user's settings for Display_ThousandsSep; empty if parameter was unset. */
        template<class StorageType, StorageType NullValue, class UserType>
        String_t formatNumber(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const;

        /** Format a number of clans.
            \param n Number
            \return Formatted number, using user's settings for Display_ThousandsSep, Display_Clans. */
        String_t formatPopulation(int32_t n) const;

        /** Format an optional number of clans.
            \param value Number
            \return Formatted value, using user's settings for Display_ThousandsSep, Display_Clans; empty if parameter was unset. */
        template<class StorageType, StorageType NullValue, class UserType>
        String_t formatPopulation(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const;

     private:
        bool m_useThousandsSeparator;
        bool m_useClans;
    };

}

template<class StorageType, StorageType NullValue, class UserType>
String_t
util::NumberFormatter::formatNumber(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const
{
    UserType i;
    if (value.get(i)) {
        return formatNumber(i);
    } else {
        return String_t();
    }
}

template<class StorageType, StorageType NullValue, class UserType>
String_t
util::NumberFormatter::formatPopulation(afl::base::InlineOptional<StorageType,NullValue,UserType> value) const
{
    UserType i;
    if (value.get(i)) {
        return formatPopulation(i);
    } else {
        return String_t();
    }
}


#endif
