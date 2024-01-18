/**
  *  \file test/game/spec/info/filtertest.cpp
  *  \brief Test for game::spec::info::Filter
  */

#include "game/spec/info/filter.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/info/browser.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "game/test/root.hpp"

namespace gsi = game::spec::info;

namespace {
    struct TestHarness {
        gsi::NullPictureNamer picNamer;
        afl::base::Ref<game::Root> root;
        game::spec::ShipList shipList;
        afl::string::NullTranslator tx;
        gsi::Browser browser;

        TestHarness()
            : picNamer(),
              root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)))),
              shipList(),
              tx(),
              browser(picNamer, *root, shipList, 3, tx)
            { }
    };
}


/** Verify initial state. */
AFL_TEST("game.spec.info.Filter:init", a)
{
    gsi::Filter testee;

    a.checkEqual("01. size",            testee.size(), 0U);
    a.check     ("02. empty",           testee.begin() == testee.end());
    a.checkEqual("03. getPlayerFilter", testee.getPlayerFilter(), 0);
    a.checkEqual("04. getNameFilter",   testee.getNameFilter(), "");
}

/** Test describe(FilterElement). */
AFL_TEST("game.spec.info.Filter:describe", a)
{
    TestHarness h;
    h.shipList.hulls().create(12)->setName("AWESOME CRUISER");
    h.shipList.basicHullFunctions().addFunction(9, "Jump");
    h.root->playerList().create(4)->setName(game::Player::ShortName, "The Frogs");
    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(3);

    gsi::Filter testee;

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_Mass, 42, gsi::IntRange_t(4, 10)), h.browser);
        a.checkEqual("01. name",         i.name, "Mass");
        a.checkEqual("02. value",        i.value, "4 to 10");
        a.checkEqual("03. mode",         i.mode, gsi::EditRange);
        a.checkEqual("04. maxRange.min", i.maxRange.min(), 0);
        a.checkEqual("05. maxRange.max", i.maxRange.max(), 20000);
        a.checkEqual("06. att",          i.elem.att, gsi::Range_Mass);
        a.checkEqual("07. value",        i.elem.value, 42);
        a.checkEqual("08. range.min",    i.elem.range.min(), 4);
        a.checkEqual("09. range.max",    i.elem.range.max(), 10);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_CostD, 23, gsi::IntRange_t(42, 20000)), h.browser);
        a.checkEqual("11. name",         i.name, "Duranium cost");
        a.checkEqual("12. value",        i.value, "42 or more");
        a.checkEqual("13. mode",         i.mode, gsi::EditRange);
        a.checkEqual("14. maxRange.min", i.maxRange.min(), 0);
        a.checkEqual("15. maxRange.max", i.maxRange.max(), 20000);
        a.checkEqual("16. att",          i.elem.att, gsi::Range_CostD);
        a.checkEqual("17. value",        i.elem.value, 23);
        a.checkEqual("18. range.min",    i.elem.range.min(), 42);
        a.checkEqual("19. range.max",    i.elem.range.max(), 20000);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_Tech, 23, gsi::IntRange_t(1, 3)), h.browser);
        a.checkEqual("21. name",         i.name, "Tech level");
        a.checkEqual("22. value",        i.value, "up to 3");
        a.checkEqual("23. mode",         i.mode, gsi::EditRange);
        a.checkEqual("24. maxRange.min", i.maxRange.min(), 1);
        a.checkEqual("25. maxRange.max", i.maxRange.max(), 10);
        a.checkEqual("26. att",          i.elem.att, gsi::Range_Tech);
        a.checkEqual("27. value",        i.elem.value, 23);
        a.checkEqual("28. range.min",    i.elem.range.min(), 1);
        a.checkEqual("29. range.max",    i.elem.range.max(), 3);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_IsArmed, 9, gsi::IntRange_t(1, 1)), h.browser);
        a.checkEqual("31. name",         i.name, "Armed");
        a.checkEqual("32. value",        i.value, "yes");
        a.checkEqual("33. mode",         i.mode, gsi::SetValueRange);
        // No test on maxRange; not relevant for SetValueRange
        a.checkEqual("34. att",          i.elem.att, gsi::Range_IsArmed);
        // value/range set to fixed values for SetValueRange
        a.checkEqual("35. value",        i.elem.value, 0);
        a.checkEqual("36. range.min",    i.elem.range.min(), 0);
        a.checkEqual("37. range.max",    i.elem.range.max(), 0);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Value_Player, 4, gsi::IntRange_t()), h.browser);
        a.checkEqual("41. name",         i.name, "Player");
        a.checkEqual("42. value",        i.value, "The Frogs");
        a.checkEqual("43. mode",         i.mode, gsi::EditValuePlayer);
        a.checkEqual("44. maxRange.min", i.maxRange.min(), 1);
        a.checkEqual("45. maxRange.max", i.maxRange.max(), 4);
        a.checkEqual("46. att",          i.elem.att, gsi::Value_Player);
        a.checkEqual("47. value",        i.elem.value, 4);
        // No test on elem.range
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Value_Hull, 12, gsi::IntRange_t()), h.browser);
        a.checkEqual("51. name",         i.name, "Hull");
        a.checkEqual("52. value",        i.value, "AWESOME CRUISER");
        a.checkEqual("53. mode",         i.mode, gsi::EditValueHull);
        a.checkEqual("54. maxRange.min", i.maxRange.min(), 1);
        a.checkEqual("55. maxRange.max", i.maxRange.max(), 12);
        a.checkEqual("56. att",          i.elem.att, gsi::Value_Hull);
        a.checkEqual("57. value",        i.elem.value, 12);
        // No test on elem.range
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser);
        a.checkEqual("61. name",         i.name, "Has");
        a.checkEqual("62. value",        i.value, "Jump (level 0)");
        a.checkEqual("63. mode",         i.mode, gsi::EditRangeLevel);
        a.checkEqual("64. maxRange.min", i.maxRange.min(), 0);
        a.checkEqual("65. maxRange.max", i.maxRange.max(), 3);
        a.checkEqual("66. att",          i.elem.att, gsi::ValueRange_ShipAbility);
        a.checkEqual("67. value",        i.elem.value, 9);
        a.checkEqual("68. range.min",    i.elem.range.min(), 0);
        a.checkEqual("69. range.max",    i.elem.range.max(), 0);
    }
}

AFL_TEST("game.spec.info.Filter:describe:2", a)
{
    TestHarness h;
    gsi::Filter testee;
    h.shipList.basicHullFunctions().addFunction(9, "Jump");

    // Value formatting
    a.checkEqual("01", testee.describe(gsi::FilterElement(gsi::Value_Category, game::spec::RacialAbilityList::Economy, gsi::IntRange_t()), h.browser).value, "Economy");
    a.checkEqual("02", testee.describe(gsi::FilterElement(gsi::Value_Origin,  game::spec::RacialAbilityList::FromConfiguration, gsi::IntRange_t()), h.browser).value, "Host configuration");
    a.checkEqual("03", testee.describe(gsi::FilterElement(gsi::Range_IsArmed, 0, gsi::IntRange_t::fromValue(2)), h.browser).value, "2");
    a.checkEqual("04", testee.describe(gsi::FilterElement(gsi::Range_IsArmed, 0, gsi::IntRange_t::fromValue(0)), h.browser).value, "no");
    a.checkEqual("05", testee.describe(gsi::FilterElement(gsi::Range_IsDeathRay, 0, gsi::IntRange_t::fromValue(0)), h.browser).value, "normal");
    a.checkEqual("06", testee.describe(gsi::FilterElement(gsi::Range_IsDeathRay, 0, gsi::IntRange_t::fromValue(1)), h.browser).value, "death ray");
    a.checkEqual("07", testee.describe(gsi::FilterElement(gsi::Range_IsDeathRay, 0, gsi::IntRange_t()), h.browser).value, "none");

    // Other specialties
    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(0);
    a.checkEqual("11", testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).value, "Jump");
    a.checkEqual("12", testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).mode, gsi::NotEditable);

    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(2);
    a.checkEqual("21", testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).value, "Jump (level 0)");
    a.checkEqual("22", testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).mode, gsi::EditRangeLevel);
}

AFL_TEST("game.spec.info.Filter:modify", a)
{
    gsi::Filter testee;

    // Add one element
    testee.add(gsi::FilterElement(gsi::Value_Player, 3, gsi::IntRange_t()));
    a.checkEqual("01. size",            testee.size(), 1U);
    a.checkEqual("02. att",             testee.begin()->att, gsi::Value_Player);
    a.checkEqual("03. value",           testee.begin()->value, 3);
    a.checkEqual("04. getPlayerFilter", testee.getPlayerFilter(), 3);

    // Add a second element
    testee.add(gsi::FilterElement(gsi::Range_NumBays, 0, gsi::IntRange_t(2, 4)));
    a.checkEqual("11. size", testee.size(), 2U);

    // Add duplicate -> no change in size and order
    testee.add(gsi::FilterElement(gsi::Value_Player, 5, gsi::IntRange_t()));
    a.checkEqual("21. size",            testee.size(), 2U);
    a.checkEqual("22. att",             testee.begin()->att, gsi::Value_Player);
    a.checkEqual("23. value",           testee.begin()->value, 5);
    a.checkEqual("24. getPlayerFilter", testee.getPlayerFilter(), 5);

    // Environment only required for formatting
    TestHarness h;
    h.root->playerList().create(3)->setName(game::Player::ShortName, "The Vorticons");
    h.root->playerList().create(5)->setName(game::Player::ShortName, "The Q");

    // Describe
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        a.checkEqual("31. size",  result.size(), 2U);
        a.checkEqual("32. name",  result[0].name, "Player");
        a.checkEqual("33. value", result[0].value, "The Q");
        a.checkEqual("34. name",  result[1].name, "Fighter Bays");
        a.checkEqual("35. value", result[1].value, "2 to 4");
    }

    // Add name filter -> not shown in size(), but in describe()
    testee.setNameFilter("dread");
    a.checkEqual("41. size", testee.size(), 2U);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        a.checkEqual("42. size",  result.size(), 3U);
        a.checkEqual("43. name",  result[0].name, "Player");
        a.checkEqual("44. value", result[0].value, "The Q");
        a.checkEqual("45. name",  result[1].name, "Fighter Bays");
        a.checkEqual("46. value", result[1].value, "2 to 4");
        a.checkEqual("47. name",  result[2].name, "Name");
        a.checkEqual("48. value", result[2].value, "dread");
        a.checkEqual("49. mode",  result[2].mode, gsi::EditString);
    }

    // Modification
    testee.setRange(1, gsi::IntRange_t::fromValue(10));
    testee.setValue(0, 3);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        a.checkEqual("51. size", result.size(), 3U);
        a.checkEqual("52. name",  result[0].name, "Player");
        a.checkEqual("53. value", result[0].value, "The Vorticons");
        a.checkEqual("54. name",  result[1].name, "Fighter Bays");
        a.checkEqual("55. value", result[1].value, "10");
        a.checkEqual("56. name",  result[2].name, "Name");
        a.checkEqual("57. value", result[2].value, "dread");
    }

    // Erase
    testee.erase(0);
    a.checkEqual("61. getPlayerFilter", testee.getPlayerFilter(), 0);
    a.checkEqual("62. size", testee.size(), 1U);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        a.checkEqual("63. size", result.size(), 2U);
        a.checkEqual("64. name",  result[0].name, "Fighter Bays");
        a.checkEqual("65. value", result[0].value, "10");
        a.checkEqual("66. name",  result[1].name, "Name");
        a.checkEqual("67. value", result[1].value, "dread");
    }

    // Erase
    testee.erase(1);
    a.checkEqual("71. size", testee.size(), 1U);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        a.checkEqual("72. size", result.size(), 1U);
    }
}
