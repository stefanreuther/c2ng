/**
  *  \file u/t_game_spec_racialabilitylist.cpp
  *  \brief Test for game::spec::RacialAbilityList
  */

#include "game/spec/racialabilitylist.hpp"

#include "t_game_spec.hpp"
#include "afl/string/string.hpp"
#include "afl/string/nulltranslator.hpp"

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
void
TestGameSpecRacialAbilityList::testConfigAbilities()
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
    const RacialAbilityList::Ability* a;
    a = search(list, "Increased MaxPlanetaryIncome (2x)");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->players, PlayerSet_t() + 2);

    a = search(list, "Reduced RaceMiningRate (70%)");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->players, PlayerSet_t() + 1);

    a = search(list, "StructureDecayPerTurn");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->players, PlayerSet_t() + 3);

    a = search(list, "ProductionRate");
    TS_ASSERT(a == 0);

    a = search(list, "ColonistTaxRate");
    TS_ASSERT(a == 0);

    a = search(list, "Reduced StarbaseCost");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->players, PlayerSet_t() + 2);

    a = search(list, "Increased BaseFighterCost");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->players, PlayerSet_t() + 1);

    a = search(list, "Reduced ShipFighterCost");
    TS_ASSERT(a);
    TS_ASSERT_EQUALS(a->players, PlayerSet_t() + 2);
}

/** Test categories.
    This covers iteration and stringification of categories. */
void
TestGameSpecRacialAbilityList::testCategories()
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
        TS_ASSERT(!toString(i->category, tx).empty());
        ++n;
    }

    // Must have had a sensible number of elements
    TS_ASSERT(n > 10);
}

/** Test addShipRacialAbilities(). */
void
TestGameSpecRacialAbilityList::testShip()
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
    TS_ASSERT_EQUALS(testee.size(), 1U);
    TS_ASSERT(testee.get(0) != 0);
    TS_ASSERT_EQUALS(testee.get(0)->name, "Eat stuff");
    TS_ASSERT(testee.get(0)->explanation.find("Drink, too") != String_t::npos);
}

/** Test filterPlayers(). */
void
TestGameSpecRacialAbilityList::testFilter()
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
    TS_ASSERT(search(list, "Increased MaxPlanetaryIncome (2x)"));
    TS_ASSERT(search(list, "Reduced RaceMiningRate (70%)"));

    // Filter for feds
    list.filterPlayers(PlayerSet_t(1));
    TS_ASSERT(!search(list, "Increased MaxPlanetaryIncome (2x)"));
    TS_ASSERT(search(list, "Reduced RaceMiningRate (70%)"));
}

/** Test origin stringification. */
void
TestGameSpecRacialAbilityList::testOrigin()
{
    afl::string::NullTranslator tx;
    TS_ASSERT(!toString(RacialAbilityList::FromHullFunction, tx).empty());
    TS_ASSERT(!toString(RacialAbilityList::FromConfiguration, tx).empty());
}

