/**
  *  \file u/t_game_spec_info_types.cpp
  *  \brief Test for game::spec::info::Types
  */

#include "game/spec/info/types.hpp"

#include "t_game_spec_info.hpp"
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
    gsi::Ability ab("n", "p", gsi::AbilityFlags_t());
    TS_ASSERT_EQUALS(ab.info, "n");
    TS_ASSERT_EQUALS(ab.pictureName, "p");
    TS_ASSERT_EQUALS(ab.flags.empty(), true);
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

