/**
  *  \file test/game/spec/info/typestest.cpp
  *  \brief Test for game::spec::info::Types
  */

#include "game/spec/info/types.hpp"

#include "afl/test/testrunner.hpp"
#include "game/limits.hpp"

namespace gsi = game::spec::info;

// Test game::spec::info::Attribute.
AFL_TEST("game.spec.info.Types:Attribute", a)
{
    gsi::Attribute att("n", "v");
    a.checkEqual("01. name",  att.name, "n");
    a.checkEqual("02. value", att.value, "v");
}

// Test game::spec::info::Ability.
AFL_TEST("game.spec.info.Types:Ability", a)
{
    gsi::Ability ab("n", "p", gsi::AbilityFlags_t());
    a.checkEqual("01. info",        ab.info, "n");
    a.checkEqual("02. pictureName", ab.pictureName, "p");
    a.checkEqual("03. flags",       ab.flags.empty(), true);
}

// Test game::spec::info::PageContent.
AFL_TEST("game.spec.info.Types:PageContent", a)
{
    gsi::PageContent pc;
    a.checkEqual("01. title",       pc.title, "");
    a.checkEqual("02. pictureName", pc.pictureName, "");
    a.check("03. attributes",       pc.attributes.empty());
    a.check("04. pageLinks",        pc.pageLinks.empty());
    a.check("05. abilities",        pc.abilities.empty());
    a.check("06. players",          pc.players.empty());
}

// Test game::spec::info::ListEntry.
AFL_TEST("game.spec.info.Types:ListEntry", a)
{
    gsi::ListEntry e("n", 99);
    a.checkEqual("01. name", e.name, "n");
    a.checkEqual("02. id",   e.id, 99);
}

// Test game::spec::info::ListContent.
AFL_TEST("game.spec.info.Types:ListContent", a)
{
    gsi::ListContent c;
    a.check("01. content", c.content.empty());
}

// Test game::spec::info::FilterElement.
AFL_TEST("game.spec.info.Types:FilterElement", a)
{
    gsi::FilterElement ele(gsi::Range_Id, 42, gsi::IntRange_t(1, 500));
    a.checkEqual("01. att",       ele.att, gsi::Range_Id);
    a.checkEqual("02. value",     ele.value, 42);
    a.checkEqual("03. range.min", ele.range.min(), 1);
    a.checkEqual("04. range.max", ele.range.max(), 500);
}

// Test game::spec::info::FilterInfo.
AFL_TEST("game.spec.info.Types:FilterInfo", a)
{
    gsi::FilterInfo info("na", "va", gsi::EditValuePlayer, gsi::IntRange_t(1, 12), gsi::FilterElement(gsi::Value_Player, 4, gsi::IntRange_t()));
    a.checkEqual("01. name",         info.name, "na");
    a.checkEqual("02. value",        info.value, "va");
    a.checkEqual("03. mode",         info.mode, gsi::EditValuePlayer);
    a.checkEqual("04. maxRange.max", info.maxRange.min(), 1);
    a.checkEqual("05. maxRange.min", info.maxRange.max(), 12);
    a.checkEqual("06. elem.att",     info.elem.att, gsi::Value_Player);
    a.checkEqual("07. elem.value",   info.elem.value, 4);
    a.checkEqual("08. active",       info.active, true);
    a.check     ("09. elem.range",   info.elem.range.empty());
}
