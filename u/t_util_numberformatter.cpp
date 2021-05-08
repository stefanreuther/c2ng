/**
  *  \file u/t_util_numberformatter.cpp
  *  \brief Test for util::NumberFormatter
  */

#include "util/numberformatter.hpp"

#include "t_util.hpp"
#include "game/types.hpp"

void
TestUtilNumberFormatter::testFormat()
{
    // Defaults: thousands separators, but no clans
    {
        const util::NumberFormatter testee(true, false);
        TS_ASSERT_EQUALS(testee.formatNumber(1), "1");
        TS_ASSERT_EQUALS(testee.formatNumber(1000), "1,000");
        TS_ASSERT_EQUALS(testee.formatNumber(-1000), "-1,000");
        TS_ASSERT_EQUALS(testee.formatNumber(1000000), "1,000,000");
        TS_ASSERT_EQUALS(testee.formatNumber(-100000), "-100,000");
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "3,300");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "33,445,500");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t(2000)), "2,000");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "200,000");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatDifference(0), "0");
        TS_ASSERT_EQUALS(testee.formatDifference(1000), "+1,000");
        TS_ASSERT_EQUALS(testee.formatDifference(-1000), "-1,000");
    }

    // No thousands separators
    {
        const util::NumberFormatter testee(false, false);
        TS_ASSERT_EQUALS(testee.formatNumber(1), "1");
        TS_ASSERT_EQUALS(testee.formatNumber(1000), "1000");
        TS_ASSERT_EQUALS(testee.formatNumber(-1000), "-1000");
        TS_ASSERT_EQUALS(testee.formatNumber(1000000), "1000000");
        TS_ASSERT_EQUALS(testee.formatNumber(-100000), "-100000");
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "3300");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "33445500");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t(2000)), "2000");
        TS_ASSERT_EQUALS(testee.formatNumber(game::IntegerProperty_t()), "");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "200000");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }

    // Clans
    {
        const util::NumberFormatter testee(true, true);
        TS_ASSERT_EQUALS(testee.formatPopulation(33), "33c");
        TS_ASSERT_EQUALS(testee.formatPopulation(334455), "334,455c");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t(2000)), "2,000c");
        TS_ASSERT_EQUALS(testee.formatPopulation(game::IntegerProperty_t()), "");
    }
}
