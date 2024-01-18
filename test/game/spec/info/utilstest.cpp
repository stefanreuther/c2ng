/**
  *  \file test/game/spec/info/utilstest.cpp
  *  \brief Test for game::spec::info::Utils
  */

#include "game/spec/info/utils.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"

namespace gsi = game::spec::info;

// Test game::spec::info::toString(FilterAttribute).
AFL_TEST("game.spec.info.Utils:toString:FilterAttribute", a)
{
    // Specimen
    afl::string::NullTranslator tx;
    a.checkEqual("01", gsi::toString(gsi::Range_MaxFuel, tx), "Fuel");
    a.checkEqual("02", gsi::toString(gsi::Value_Origin, tx), "From");

    // General
    for (int i = 0; i <= gsi::ValueRange_ShipAbility; ++i) {
        a.check("11", !gsi::toString(gsi::FilterAttribute(i), tx).empty());
    }
}

// Test game::spec::info::convertRangeToSet().
AFL_TEST("game.spec.info.Utils:convertRangeToSet", a)
{
    a.checkEqual("01", gsi::convertRangeToSet(gsi::IntRange_t()).toInteger(), 0U);
    a.checkEqual("02", gsi::convertRangeToSet(gsi::IntRange_t(0, 4)).toInteger(), 0x1FU);
    a.checkEqual("03", gsi::convertRangeToSet(gsi::IntRange_t(1, 4)).toInteger(), 0x1EU);
}

// Test game::spec::info::getLevelRange().
AFL_TEST("game.spec.info.Utils:getLevelRange", a)
{
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 0, 0))));
    r->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(7);

    a.check("01", gsi::getLevelRange(*r) == gsi::IntRange_t(0, 7));
}

// Test game::spec::info::getHullRange().
AFL_TEST("game.spec.info.Utils:getHullRange", a)
{
    game::spec::ShipList sl;
    sl.hulls().create(19);
    sl.hulls().create(2);

    a.check("01", gsi::getHullRange(sl) == gsi::IntRange_t(1, 19));
}

// Test game::spec::info::getPlayerRange().
AFL_TEST("game.spec.info.Utils:getPlayerRange", a)
{
    afl::base::Ref<game::Root> r(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3, 0, 0))));
    r->playerList().create(1);
    r->playerList().create(7);
    r->playerList().create(9);

    a.check("01", gsi::getPlayerRange(*r) == gsi::IntRange_t(1, 9));
}

// Test game::spec::info::getAttributeRange().
AFL_TEST("game.spec.info.Utils:getAttributeRange", a)
{
    // Tech goes from 1..10
    a.checkEqual("01", gsi::getAttributeRange(gsi::Range_Tech).min(), 1);
    a.checkEqual("02", gsi::getAttributeRange(gsi::Range_Tech).max(), 10);

    // Cost goes from 0 to at least MAX_NUMBER
    a.checkEqual("11", gsi::getAttributeRange(gsi::Range_CostD).min(), 0);
    a.checkGreaterEqual("12", gsi::getAttributeRange(gsi::Range_CostD).max(), game::MAX_NUMBER);
}
