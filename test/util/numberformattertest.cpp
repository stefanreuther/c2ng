/**
  *  \file test/util/numberformattertest.cpp
  *  \brief Test for util::NumberFormatter
  */

#include "util/numberformatter.hpp"

#include "afl/test/testrunner.hpp"
#include "game/types.hpp"

// Defaults: thousands separators, but no clans
AFL_TEST("util.NumberFormatter:default", a)
{
    const util::NumberFormatter testee(true, false);
    a.checkEqual("01", testee.formatNumber(1), "1");
    a.checkEqual("02", testee.formatNumber(1000), "1,000");
    a.checkEqual("03", testee.formatNumber(-1000), "-1,000");
    a.checkEqual("04", testee.formatNumber(1000000), "1,000,000");
    a.checkEqual("05", testee.formatNumber(-100000), "-100,000");
    a.checkEqual("06", testee.formatPopulation(33), "3,300");
    a.checkEqual("07", testee.formatPopulation(334455), "33,445,500");
    a.checkEqual("08", testee.formatNumber(game::IntegerProperty_t(2000)), "2,000");
    a.checkEqual("09", testee.formatNumber(game::IntegerProperty_t()), "");
    a.checkEqual("10", testee.formatPopulation(game::IntegerProperty_t(2000)), "200,000");
    a.checkEqual("11", testee.formatPopulation(game::IntegerProperty_t()), "");
    a.checkEqual("12", testee.formatDifference(0), "0");
    a.checkEqual("13", testee.formatDifference(1000), "+1,000");
    a.checkEqual("14", testee.formatDifference(-1000), "-1,000");
}

// No thousands separators
AFL_TEST("util.NumberFormatter:thousands-separator", a)
{
    const util::NumberFormatter testee(false, false);
    a.checkEqual("01", testee.formatNumber(1), "1");
    a.checkEqual("02", testee.formatNumber(1000), "1000");
    a.checkEqual("03", testee.formatNumber(-1000), "-1000");
    a.checkEqual("04", testee.formatNumber(1000000), "1000000");
    a.checkEqual("05", testee.formatNumber(-100000), "-100000");
    a.checkEqual("06", testee.formatPopulation(33), "3300");
    a.checkEqual("07", testee.formatPopulation(334455), "33445500");
    a.checkEqual("08", testee.formatNumber(game::IntegerProperty_t(2000)), "2000");
    a.checkEqual("09", testee.formatNumber(game::IntegerProperty_t()), "");
    a.checkEqual("10", testee.formatPopulation(game::IntegerProperty_t(2000)), "200000");
    a.checkEqual("11", testee.formatPopulation(game::IntegerProperty_t()), "");
}

// Clans
AFL_TEST("util.NumberFormatter:clans", a)
{
    const util::NumberFormatter testee(true, true);
    a.checkEqual("01", testee.formatPopulation(33), "33c");
    a.checkEqual("02", testee.formatPopulation(334455), "334,455c");
    a.checkEqual("03", testee.formatPopulation(game::IntegerProperty_t(2000)), "2,000c");
    a.checkEqual("04", testee.formatPopulation(game::IntegerProperty_t()), "");
}
