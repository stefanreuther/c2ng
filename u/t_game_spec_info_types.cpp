/**
  *  \file u/t_game_spec_info_types.cpp
  *  \brief Test for game::spec::info::Types
  */

#include "game/spec/info/types.hpp"

#include "t_game_spec_info.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/root.hpp"
#include "game/limits.hpp"

namespace gsi = game::spec::info;

// Test game::spec::info::Attribute.
void
TestGameSpecInfoTypes::testAttribute()
{
    gsi::Attribute att("n", "v");
    TS_ASSERT_EQUALS(att.name, "n");
    TS_ASSERT_EQUALS(att.value, "v");
}

// Test game::spec::info::Ability.
void
TestGameSpecInfoTypes::testAbility()
{
    gsi::Ability ab("n", "p");
    TS_ASSERT_EQUALS(ab.info, "n");
    TS_ASSERT_EQUALS(ab.pictureName, "p");
}

// Test game::spec::info::PageContent.
void
TestGameSpecInfoTypes::testPageContent()
{
    gsi::PageContent pc;
    TS_ASSERT_EQUALS(pc.title, "");
    TS_ASSERT_EQUALS(pc.pictureName, "");
    TS_ASSERT(pc.attributes.empty());
    TS_ASSERT(pc.pageLinks.empty());
    TS_ASSERT(pc.abilities.empty());
    TS_ASSERT(pc.players.empty());
}

// Test game::spec::info::ListEntry.
void
TestGameSpecInfoTypes::testListEntry()
{
    gsi::ListEntry e("n", 99);
    TS_ASSERT_EQUALS(e.name, "n");
    TS_ASSERT_EQUALS(e.id, 99);
}

// Test game::spec::info::ListContent.
void
TestGameSpecInfoTypes::testListContent()
{
    gsi::ListContent c;
    TS_ASSERT(c.content.empty());
}

// Test game::spec::info::FilterElement.
void
TestGameSpecInfoTypes::testFilterElement()
{
    gsi::FilterElement ele(gsi::Range_Id, 42, gsi::IntRange_t(1, 500));
    TS_ASSERT_EQUALS(ele.att, gsi::Range_Id);
    TS_ASSERT_EQUALS(ele.value, 42);
    TS_ASSERT_EQUALS(ele.range.min(), 1);
    TS_ASSERT_EQUALS(ele.range.max(), 500);
}

// Test game::spec::info::FilterInfo.
void
TestGameSpecInfoTypes::testFilterInfo()
{
    gsi::FilterInfo info("na", "va", gsi::EditValuePlayer, gsi::IntRange_t(1, 12), gsi::FilterElement(gsi::Value_Player, 4, gsi::IntRange_t()));
    TS_ASSERT_EQUALS(info.name, "na");
    TS_ASSERT_EQUALS(info.value, "va");
    TS_ASSERT_EQUALS(info.mode, gsi::EditValuePlayer);
    TS_ASSERT_EQUALS(info.maxRange.min(), 1);
    TS_ASSERT_EQUALS(info.maxRange.max(), 12);
    TS_ASSERT_EQUALS(info.elem.att, gsi::Value_Player);
    TS_ASSERT_EQUALS(info.elem.value, 4);
    TS_ASSERT_EQUALS(info.active, true);
    TS_ASSERT(info.elem.range.empty());
}

// Test game::spec::info::toString(FilterAttribute).
void
TestGameSpecInfoTypes::testFilterAttributeToString()
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
TestGameSpecInfoTypes::testConvertRangeToSet()
{
    TS_ASSERT_EQUALS(gsi::convertRangeToSet(gsi::IntRange_t()).toInteger(), 0U);
    TS_ASSERT_EQUALS(gsi::convertRangeToSet(gsi::IntRange_t(0, 4)).toInteger(), 0x1FU);
    TS_ASSERT_EQUALS(gsi::convertRangeToSet(gsi::IntRange_t(1, 4)).toInteger(), 0x1EU);
}

// Test game::spec::info::getLevelRange().
void
TestGameSpecInfoTypes::testGetLevelRange()
{
    game::test::Root r(game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 0, 0)));
    r.hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(7);

    TS_ASSERT_EQUALS(gsi::getLevelRange(r), gsi::IntRange_t(0, 7));
}

// Test game::spec::info::getHullRange().
void
TestGameSpecInfoTypes::testGetHullRange()
{
    game::spec::ShipList sl;
    sl.hulls().create(19);
    sl.hulls().create(2);

    TS_ASSERT_EQUALS(gsi::getHullRange(sl), gsi::IntRange_t(1, 19));
}

// Test game::spec::info::getPlayerRange().
void
TestGameSpecInfoTypes::testGetPlayerRange()
{
    game::test::Root r(game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 0, 0)));
    r.playerList().create(1);
    r.playerList().create(7);
    r.playerList().create(9);

    TS_ASSERT_EQUALS(gsi::getPlayerRange(r), gsi::IntRange_t(1, 9));
}

// Test game::spec::info::getAttributeRange().
void
TestGameSpecInfoTypes::testAttributeRange()
{
    // Tech goes from 1..10
    TS_ASSERT_EQUALS(gsi::getAttributeRange(gsi::Range_Tech).min(), 1);
    TS_ASSERT_EQUALS(gsi::getAttributeRange(gsi::Range_Tech).max(), 10);

    // Cost goes from 0 to at least MAX_NUMBER
    TS_ASSERT_EQUALS(gsi::getAttributeRange(gsi::Range_CostD).min(), 0);
    TS_ASSERT(gsi::getAttributeRange(gsi::Range_CostD).max() >= game::MAX_NUMBER);
}

