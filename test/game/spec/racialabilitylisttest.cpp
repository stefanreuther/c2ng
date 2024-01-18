/**
  *  \file test/game/spec/racialabilitylisttest.cpp
  *  \brief Test for game::spec::RacialAbilityList
  */

#include "game/spec/racialabilitylist.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/string/string.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    using game::config::HostConfiguration;
    using game::config::ConfigurationOption;
    using game::spec::RacialAbilityList;
    using game::PlayerSet_t;

    const RacialAbilityList::Ability* search(const RacialAbilityList& list, String_t needle)
    {
        for (size_t i = 0, n = list.size(); i < n; ++i) {
            if (const RacialAbilityList::Ability* a = list.get(i)) {
                if (afl::string::strLCase(a->name).find(afl::string::strLCase(needle)) != String_t::npos) {
                    return a;
                }
            }
        }
        return 0;
    }
}


/** Test addConfigRacialAbilities().
    This mainly verifies that the various classifications (increase/reduce) work as intended,
    it does not test all individual options. */
AFL_TEST("game.spec.RacialAbilityList:addConfigRacialAbilities", a)
{
    // Prepare a configuration
    HostConfiguration config;
    const ConfigurationOption::Source src = ConfigurationOption::Game;
    config.setOption("MaxPlanetaryIncome", "1000,2000,1000", src);  // generates 'increase' for Lizards
    config.setOption("RaceMiningRate", "70,100,100", src);          // generates 'reduced' for Feds
    config.setOption("StructureDecayPerTurn", "0,0,4,0,0", src);    // generates unclassified for Bird
    config.setOption("ProductionRate", "1,2,3,4,5,6,7,8", src);     // generates no message (too many values)
    config.setOption("ColonistTaxRate", "100", src);                // generates no message (only one value)
    config.setOption("StarbaseCost", "100t, 90t, 100t", src);       // generates 'reduced' for Lizard
    config.setOption("BaseFighterCost", "110t, 100t, 100t", src);   // generates 'increased' for Fed
    config.setOption("ShipFighterCost", "100s, 90$, 100s", src);    // generates 'reduced' for Lizard (90$ is less than 100s)

    // Build object to test
    afl::string::NullTranslator tx;
    game::spec::RacialAbilityList list;
    list.addConfigRacialAbilities(config, util::NumberFormatter(true, true), tx);

    // Test it
    const RacialAbilityList::Ability* ra;
    ra = search(list, "Increased MaxPlanetaryIncome (2x)");
    a.check("01. search", ra);
    a.checkEqual("02. players", ra->players, PlayerSet_t() + 2);

    ra = search(list, "Reduced RaceMiningRate (70%)");
    a.check("11. search", ra);
    a.checkEqual("12. players", ra->players, PlayerSet_t() + 1);

    ra = search(list, "StructureDecayPerTurn");
    a.check("21. search", ra);
    a.checkEqual("22. players", ra->players, PlayerSet_t() + 3);

    ra = search(list, "ProductionRate");
    a.checkNull("31. search", ra);

    ra = search(list, "ColonistTaxRate");
    a.checkNull("41. search", ra);

    ra = search(list, "Reduced StarbaseCost");
    a.check("51. search", ra);
    a.checkEqual("52. players", ra->players, PlayerSet_t() + 2);

    ra = search(list, "Increased BaseFighterCost");
    a.check("61. search", ra);
    a.checkEqual("62. players", ra->players, PlayerSet_t() + 1);

    ra = search(list, "Reduced ShipFighterCost");
    a.check("71. search", ra);
    a.checkEqual("72. players", ra->players, PlayerSet_t() + 2);
}

/** Test categories.
    This covers iteration and stringification of categories. */
AFL_TEST("game.spec.RacialAbilityList:addConfigRacialAbilities:categories", a)
{
    // Build object to test
    HostConfiguration config;
    config.setOption("SensorRange", "100,200", ConfigurationOption::Game);   // default config does not generate a Sensor element
    afl::string::NullTranslator tx;
    game::spec::RacialAbilityList list;
    list.addConfigRacialAbilities(config, util::NumberFormatter(true, true), tx);

    // Iterate through abilities, check that all categories stringify sensibly
    int n = 0;
    for (RacialAbilityList::Iterator_t i = list.begin(); i != list.end(); ++i) {
        a.check("01. toString", !toString(i->category, tx).empty());
        ++n;
    }

    // Must have had a sensible number of elements
    a.checkGreaterThan("11. count", n, 10);
}

/** Test addShipRacialAbilities(). */
AFL_TEST("game.spec.RacialAbilityList:addShipRacialAbilities", a)
{
    // Add a racial ability
    game::spec::ShipList list;
    game::spec::BasicHullFunction* hf = list.basicHullFunctions().addFunction(9, "Eat");
    hf->setDescription("Eat stuff");
    hf->setExplanation("Drink, too");
    list.racialAbilities().change(list.modifiedHullFunctions().getFunctionIdFromHostId(9), game::PlayerSet_t(7), game::PlayerSet_t());

    // Present as RacialAbilityList
    game::spec::RacialAbilityList testee;
    testee.addShipRacialAbilities(list);

    // Verify
    a.checkEqual("01. size", testee.size(), 1U);
    a.checkNonNull("02. get", testee.get(0));
    a.checkEqual("03. name", testee.get(0)->name, "Eat stuff");
    a.checkDifferent("04. explanation", testee.get(0)->explanation.find("Drink, too"), String_t::npos);
}

/** Test filterPlayers(). */
AFL_TEST("game.spec.RacialAbilityList:filterPlayers", a)
{
    // Prepare a configuration
    HostConfiguration config;
    const ConfigurationOption::Source src = ConfigurationOption::Game;
    config.setOption("MaxPlanetaryIncome", "1000,2000,1000", src);  // generates 'increase' for Lizards
    config.setOption("RaceMiningRate", "70,100,100", src);          // generates 'reduced' for Feds

    // Build object to test
    afl::string::NullTranslator tx;
    game::spec::RacialAbilityList list;
    list.addConfigRacialAbilities(config, util::NumberFormatter(true, true), tx);

    // Test it
    a.check("01", search(list, "Increased MaxPlanetaryIncome (2x)"));
    a.check("02", search(list, "Reduced RaceMiningRate (70%)"));

    // Filter for feds
    list.filterPlayers(PlayerSet_t(1));
    a.check("11", !search(list, "Increased MaxPlanetaryIncome (2x)"));
    a.check("12", search(list, "Reduced RaceMiningRate (70%)"));
}

/** Test origin stringification. */
AFL_TEST("game.spec.RacialAbilityList:toString", a)
{
    afl::string::NullTranslator tx;
    a.check("01", !toString(RacialAbilityList::FromHullFunction, tx).empty());
    a.check("02", !toString(RacialAbilityList::FromConfiguration, tx).empty());
}
