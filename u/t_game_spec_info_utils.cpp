/**
  *  \file u/t_game_spec_info_utils.cpp
  *  \brief Test for game::spec::info::Utils
  */

#include "game/spec/info/utils.hpp"

#include "t_game_spec_info.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/root.hpp"

namespace gsi = game::spec::info;

// Test game::spec::info::toString(FilterAttribute).
void
TestGameSpecInfoUtils::testFilterAttributeToString()
{
    // Specimen
    afl::string::NullTranslator tx;
    TS_ASSERT_EQUALS(gsi::toString(gsi::Range_MaxFuel, tx), "Fuel");
    TS_ASSERT_EQUALS(gsi::toString(gsi::Value_Origin, tx), "From");

    // General
    for (int i = 0; i <= gsi::ValueRange_ShipAbility; ++i) {
        TS_ASSERT(!gsi::toString(gsi::FilterAttribute(i), tx).empty());
    }
}

// Test game::spec::info::convertRangeToSet().
void
TestGameSpecInfoUtils::testConvertRangeToSet()
{
    TS_ASSERT_EQUALS(gsi::convertRangeToSet(gsi::IntRange_t()).toInteger(), 0U);
    TS_ASSERT_EQUALS(gsi::convertRangeToSet(gsi::IntRange_t(0, 4)).toInteger(), 0x1FU);
    TS_ASSERT_EQUALS(gsi::convertRangeToSet(gsi::IntRange_t(1, 4)).toInteger(), 0x1EU);
}

// Test game::spec::info::getLevelRange().
void
TestGameSpecInfoUtils::testGetLevelRange()
{
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 0, 0))));
    r->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(7);

    TS_ASSERT_EQUALS(gsi::getLevelRange(*r), gsi::IntRange_t(0, 7));
}

// Test game::spec::info::getHullRange().
void
TestGameSpecInfoUtils::testGetHullRange()
{
    game::spec::ShipList sl;
    sl.hulls().create(19);
    sl.hulls().create(2);

    TS_ASSERT_EQUALS(gsi::getHullRange(sl), gsi::IntRange_t(1, 19));
}

// Test game::spec::info::getPlayerRange().
void
TestGameSpecInfoUtils::testGetPlayerRange()
{
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 0, 0))));
    r->playerList().create(1);
    r->playerList().create(7);
    r->playerList().create(9);

    TS_ASSERT_EQUALS(gsi::getPlayerRange(*r), gsi::IntRange_t(1, 9));
}

// Test game::spec::info::getAttributeRange().
void
TestGameSpecInfoUtils::testAttributeRange()
{
    // Tech goes from 1..10
    TS_ASSERT_EQUALS(gsi::getAttributeRange(gsi::Range_Tech).min(), 1);
    TS_ASSERT_EQUALS(gsi::getAttributeRange(gsi::Range_Tech).max(), 10);

    // Cost goes from 0 to at least MAX_NUMBER
    TS_ASSERT_EQUALS(gsi::getAttributeRange(gsi::Range_CostD).min(), 0);
    TS_ASSERT(gsi::getAttributeRange(gsi::Range_CostD).max() >= game::MAX_NUMBER);
}

