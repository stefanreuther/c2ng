/**
  *  \file util/numberformatter.cpp
  */

#include "util/numberformatter.hpp"
#include "afl/string/format.hpp"

util::NumberFormatter::NumberFormatter(bool useThousandsSeparator, bool useClans)
    : m_useThousandsSeparator(useThousandsSeparator),
      m_useClans(useClans)
{ }

String_t
util::NumberFormatter::formatNumber(int32_t n) const
{
    // ex numToString
    String_t result = afl::string::Format("%d", n);
    if (m_useThousandsSeparator) {
        // The limit is to avoid placing a thousands-separator as "-,234"
        size_t i = result.size();
        size_t limit = (i > 0 && (result[0] < '0' || result[0] > '9')) ? 4 : 3;
        while (i > limit) {
            i -= 3;
            result.insert(i, ",");
        }
    }
    return result;
}

String_t
util::NumberFormatter::formatPopulation(int32_t n) const
{
    // ex clansToString
    return (m_useClans
            ? formatNumber(n) + "c"
            : formatNumber(100*n));
}
