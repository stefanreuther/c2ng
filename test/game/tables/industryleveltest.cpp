/**
  *  \file test/game/tables/industryleveltest.cpp
  *  \brief Test for game::tables::IndustryLevel
  */

#include "game/tables/industrylevel.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/types.hpp"

AFL_TEST("game.tables.IndustryLevel", a)
{
    afl::string::NullTranslator tx;
    game::tables::IndustryLevel testee(tx);

    // Enum values
    a.checkEqual("01", testee(game::MinimalIndustry), "minimal");
    a.checkEqual("02", testee(game::LightIndustry), "light");
    a.checkEqual("03", testee(game::ModerateIndustry), "moderate");
    a.checkEqual("04", testee(game::SubstantialIndustry), "substantial");
    a.checkEqual("05", testee(game::HeavyIndustry), "heavy");

    // Outside ranges
    a.checkEqual("11", testee(game::MinimalIndustry-1), "minimal");
    a.checkEqual("12", testee(game::MinimalIndustry-100), "minimal");
    a.checkEqual("13", testee(game::HeavyIndustry+100), "heavy");
    a.checkEqual("14", testee(game::HeavyIndustry+1), "heavy");

    // Loop: 5 levels
    int n;
    int count = 0;
    for (bool v = testee.getFirstKey(n); v; v = testee.getNextKey(n)) {
        ++count;
    }
    a.checkEqual("21", count, 5);
}
