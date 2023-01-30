/**
  *  \file u/t_game_spec_info_filter.cpp
  *  \brief Test for game::spec::info::Filter
  */

#include "game/spec/info/filter.hpp"

#include "t_game_spec_info.hpp"
#include "afl/string/nulltranslator.hpp"
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
void
TestGameSpecInfoFilter::testInit()
{
    gsi::Filter testee;

    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee.begin(), testee.end());
    TS_ASSERT_EQUALS(testee.getPlayerFilter(), 0);
    TS_ASSERT_EQUALS(testee.getNameFilter(), "");
}

/** Test describe(FilterElement). */
void
TestGameSpecInfoFilter::testDescribeElement()
{
    TestHarness h;
    h.shipList.hulls().create(12)->setName("AWESOME CRUISER");
    h.shipList.basicHullFunctions().addFunction(9, "Jump");
    h.root->playerList().create(4)->setName(game::Player::ShortName, "The Frogs");
    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(3);

    gsi::Filter testee;

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_Mass, 42, gsi::IntRange_t(4, 10)), h.browser);
        TS_ASSERT_EQUALS(i.name, "Mass");
        TS_ASSERT_EQUALS(i.value, "4 to 10");
        TS_ASSERT_EQUALS(i.mode, gsi::EditRange);
        TS_ASSERT_EQUALS(i.maxRange.min(), 0);
        TS_ASSERT_EQUALS(i.maxRange.max(), 20000);
        TS_ASSERT_EQUALS(i.elem.att, gsi::Range_Mass);
        TS_ASSERT_EQUALS(i.elem.value, 42);
        TS_ASSERT_EQUALS(i.elem.range.min(), 4);
        TS_ASSERT_EQUALS(i.elem.range.max(), 10);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_CostD, 23, gsi::IntRange_t(42, 20000)), h.browser);
        TS_ASSERT_EQUALS(i.name, "Duranium cost");
        TS_ASSERT_EQUALS(i.value, "42 or more");
        TS_ASSERT_EQUALS(i.mode, gsi::EditRange);
        TS_ASSERT_EQUALS(i.maxRange.min(), 0);
        TS_ASSERT_EQUALS(i.maxRange.max(), 20000);
        TS_ASSERT_EQUALS(i.elem.att, gsi::Range_CostD);
        TS_ASSERT_EQUALS(i.elem.value, 23);
        TS_ASSERT_EQUALS(i.elem.range.min(), 42);
        TS_ASSERT_EQUALS(i.elem.range.max(), 20000);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_Tech, 23, gsi::IntRange_t(1, 3)), h.browser);
        TS_ASSERT_EQUALS(i.name, "Tech level");
        TS_ASSERT_EQUALS(i.value, "up to 3");
        TS_ASSERT_EQUALS(i.mode, gsi::EditRange);
        TS_ASSERT_EQUALS(i.maxRange.min(), 1);
        TS_ASSERT_EQUALS(i.maxRange.max(), 10);
        TS_ASSERT_EQUALS(i.elem.att, gsi::Range_Tech);
        TS_ASSERT_EQUALS(i.elem.value, 23);
        TS_ASSERT_EQUALS(i.elem.range.min(), 1);
        TS_ASSERT_EQUALS(i.elem.range.max(), 3);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Range_IsArmed, 9, gsi::IntRange_t(1, 1)), h.browser);
        TS_ASSERT_EQUALS(i.name, "Armed");
        TS_ASSERT_EQUALS(i.value, "yes");
        TS_ASSERT_EQUALS(i.mode, gsi::SetValueRange);
        // No test on maxRange; not relevant for SetValueRange
        TS_ASSERT_EQUALS(i.elem.att, gsi::Range_IsArmed);
        // value/range set to fixed values for SetValueRange
        TS_ASSERT_EQUALS(i.elem.value, 0);
        TS_ASSERT_EQUALS(i.elem.range.min(), 0);
        TS_ASSERT_EQUALS(i.elem.range.max(), 0);
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Value_Player, 4, gsi::IntRange_t()), h.browser);
        TS_ASSERT_EQUALS(i.name, "Player");
        TS_ASSERT_EQUALS(i.value, "The Frogs");
        TS_ASSERT_EQUALS(i.mode, gsi::EditValuePlayer);
        TS_ASSERT_EQUALS(i.maxRange.min(), 1);
        TS_ASSERT_EQUALS(i.maxRange.max(), 4);
        TS_ASSERT_EQUALS(i.elem.att, gsi::Value_Player);
        TS_ASSERT_EQUALS(i.elem.value, 4);
        // No test on elem.range
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::Value_Hull, 12, gsi::IntRange_t()), h.browser);
        TS_ASSERT_EQUALS(i.name, "Hull");
        TS_ASSERT_EQUALS(i.value, "AWESOME CRUISER");
        TS_ASSERT_EQUALS(i.mode, gsi::EditValueHull);
        TS_ASSERT_EQUALS(i.maxRange.min(), 1);
        TS_ASSERT_EQUALS(i.maxRange.max(), 12);
        TS_ASSERT_EQUALS(i.elem.att, gsi::Value_Hull);
        TS_ASSERT_EQUALS(i.elem.value, 12);
        // No test on elem.range
    }

    {
        gsi::FilterInfo i = testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser);
        TS_ASSERT_EQUALS(i.name, "Has");
        TS_ASSERT_EQUALS(i.value, "Jump (level 0)");
        TS_ASSERT_EQUALS(i.mode, gsi::EditRangeLevel);
        TS_ASSERT_EQUALS(i.maxRange.min(), 0);
        TS_ASSERT_EQUALS(i.maxRange.max(), 3);
        TS_ASSERT_EQUALS(i.elem.att, gsi::ValueRange_ShipAbility);
        TS_ASSERT_EQUALS(i.elem.value, 9);
        TS_ASSERT_EQUALS(i.elem.range.min(), 0);
        TS_ASSERT_EQUALS(i.elem.range.max(), 0);
    }
}

void
TestGameSpecInfoFilter::testDescribeElement2()
{
    TestHarness h;
    gsi::Filter testee;
    h.shipList.basicHullFunctions().addFunction(9, "Jump");

    // Value formatting
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Value_Category, game::spec::RacialAbilityList::Economy, gsi::IntRange_t()), h.browser).value, "Economy");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Value_Origin,  game::spec::RacialAbilityList::FromConfiguration, gsi::IntRange_t()), h.browser).value, "Host configuration");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Range_IsArmed, 0, gsi::IntRange_t::fromValue(2)), h.browser).value, "2");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Range_IsArmed, 0, gsi::IntRange_t::fromValue(0)), h.browser).value, "no");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Range_IsDeathRay, 0, gsi::IntRange_t::fromValue(0)), h.browser).value, "normal");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Range_IsDeathRay, 0, gsi::IntRange_t::fromValue(1)), h.browser).value, "death ray");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::Range_IsDeathRay, 0, gsi::IntRange_t()), h.browser).value, "none");

    // Other specialties
    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(0);
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).value, "Jump");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).mode, gsi::NotEditable);

    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(2);
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).value, "Jump (level 0)");
    TS_ASSERT_EQUALS(testee.describe(gsi::FilterElement(gsi::ValueRange_ShipAbility, 9, gsi::IntRange_t::fromValue(0)), h.browser).mode, gsi::EditRangeLevel);
}

void
TestGameSpecInfoFilter::testModify()
{
    gsi::Filter testee;

    // Add one element
    testee.add(gsi::FilterElement(gsi::Value_Player, 3, gsi::IntRange_t()));
    TS_ASSERT_EQUALS(testee.size(), 1U);
    TS_ASSERT_EQUALS(testee.begin()->att, gsi::Value_Player);
    TS_ASSERT_EQUALS(testee.begin()->value, 3);
    TS_ASSERT_EQUALS(testee.getPlayerFilter(), 3);

    // Add a second element
    testee.add(gsi::FilterElement(gsi::Range_NumBays, 0, gsi::IntRange_t(2, 4)));
    TS_ASSERT_EQUALS(testee.size(), 2U);

    // Add duplicate -> no change in size and order
    testee.add(gsi::FilterElement(gsi::Value_Player, 5, gsi::IntRange_t()));
    TS_ASSERT_EQUALS(testee.size(), 2U);
    TS_ASSERT_EQUALS(testee.begin()->att, gsi::Value_Player);
    TS_ASSERT_EQUALS(testee.begin()->value, 5);
    TS_ASSERT_EQUALS(testee.getPlayerFilter(), 5);

    // Environment only required for formatting
    TestHarness h;
    h.root->playerList().create(3)->setName(game::Player::ShortName, "The Vorticons");
    h.root->playerList().create(5)->setName(game::Player::ShortName, "The Q");

    // Describe
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].name, "Player");
        TS_ASSERT_EQUALS(result[0].value, "The Q");
        TS_ASSERT_EQUALS(result[1].name, "Fighter Bays");
        TS_ASSERT_EQUALS(result[1].value, "2 to 4");
    }

    // Add name filter -> not shown in size(), but in describe()
    testee.setNameFilter("dread");
    TS_ASSERT_EQUALS(testee.size(), 2U);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0].name, "Player");
        TS_ASSERT_EQUALS(result[0].value, "The Q");
        TS_ASSERT_EQUALS(result[1].name, "Fighter Bays");
        TS_ASSERT_EQUALS(result[1].value, "2 to 4");
        TS_ASSERT_EQUALS(result[2].name, "Name");
        TS_ASSERT_EQUALS(result[2].value, "dread");
        TS_ASSERT_EQUALS(result[2].mode, gsi::EditString);
    }

    // Modification
    testee.setRange(1, gsi::IntRange_t::fromValue(10));
    testee.setValue(0, 3);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        TS_ASSERT_EQUALS(result.size(), 3U);
        TS_ASSERT_EQUALS(result[0].name, "Player");
        TS_ASSERT_EQUALS(result[0].value, "The Vorticons");
        TS_ASSERT_EQUALS(result[1].name, "Fighter Bays");
        TS_ASSERT_EQUALS(result[1].value, "10");
        TS_ASSERT_EQUALS(result[2].name, "Name");
        TS_ASSERT_EQUALS(result[2].value, "dread");
    }

    // Erase
    testee.erase(0);
    TS_ASSERT_EQUALS(testee.getPlayerFilter(), 0);
    TS_ASSERT_EQUALS(testee.size(), 1U);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        TS_ASSERT_EQUALS(result.size(), 2U);
        TS_ASSERT_EQUALS(result[0].name, "Fighter Bays");
        TS_ASSERT_EQUALS(result[0].value, "10");
        TS_ASSERT_EQUALS(result[1].name, "Name");
        TS_ASSERT_EQUALS(result[1].value, "dread");
    }

    // Erase
    testee.erase(1);
    TS_ASSERT_EQUALS(testee.size(), 1U);
    {
        gsi::FilterInfos_t result;
        testee.describe(result, h.browser);
        TS_ASSERT_EQUALS(result.size(), 1U);
    }
}

