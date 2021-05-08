/**
  *  \file util/numberformatter.cpp
  *  \brief Class util::NumberFormatter
  */

#include "util/numberformatter.hpp"
#include "afl/string/format.hpp"

namespace {
    String_t insertThousandsSeparator(String_t str, bool flag)
    {
        if (flag) {
            // The limit is to avoid placing a thousands-separator as "-,234"
            size_t i = str.size();
            size_t limit = (i > 0 && (str[0] < '0' || str[0] > '9')) ? 4 : 3;
            while (i > limit) {
                i -= 3;
                str.insert(i, ",");
            }
        }
        return str;
    }
}

util::NumberFormatter::NumberFormatter(bool useThousandsSeparator, bool useClans)
    : m_useThousandsSeparator(useThousandsSeparator),
      m_useClans(useClans)
{ }

String_t
util::NumberFormatter::formatNumber(int32_t n) const
{
    // ex numToString
    return insertThousandsSeparator(afl::string::Format("%d", n), m_useThousandsSeparator);
}

String_t
util::NumberFormatter::formatDifference(int32_t n) const
{
    if (n == 0) {
        return "0";
    } else {
        return insertThousandsSeparator(afl::string::Format("%+d", n), m_useThousandsSeparator);
    }
}

String_t
util::NumberFormatter::formatPopulation(int32_t n) const
{
    // ex clansToString
    return (m_useClans
            ? formatNumber(n) + "c"
            : formatNumber(100*n));
}
