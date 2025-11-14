/**
  *  \file test/game/spec/shiplisttest.cpp
  *  \brief Test for game::spec::ShipList
  */

#include "game/spec/shiplist.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using game::PlayerSet_t;
using game::config::HostConfiguration;
using game::spec::BasicHullFunction;
using game::spec::HullFunction;
using game::spec::ModifiedHullFunctionList;

/** Basic accessor test. */
AFL_TEST("game.spec.ShipList:basics", a)
{
    // Testee
    game::spec::ShipList sl;
    const game::spec::ShipList& csl = sl;

    // Verify components: const and non-const must be the same object; tables must be empty
    a.checkEqual("01. beams", &sl.beams(), &csl.beams());
    a.checkNull("02. beams", csl.beams().findNext(0));

    a.checkEqual("11. engines", &sl.engines(), &csl.engines());
    a.checkNull("12. engines", csl.engines().findNext(0));

    a.checkEqual("21. launchers", &sl.launchers(), &csl.launchers());
    a.checkNull("22. launchers", csl.launchers().findNext(0));

    a.checkEqual("31. hulls", &sl.hulls(), &csl.hulls());
    a.checkNull("32. hulls", csl.hulls().findNext(0));

    // Verify hull function stuff: const and non-const must be the same object; state must imply emptiness
    // (we cannot directly verify emptiness.)
    a.checkEqual("41. basicHullFunctions", &sl.basicHullFunctions(), &csl.basicHullFunctions());
    a.checkNull("42. basicHullFunctions", csl.basicHullFunctions().getFunctionById(0));

    a.checkEqual("51. modifiedHullFunctions", &sl.modifiedHullFunctions(), &csl.modifiedHullFunctions());
    a.check("52. modifiedHullFunctions", csl.modifiedHullFunctions().getFunctionIdFromHostId(42) == 42);

    a.checkEqual("61. racialAbilities", &sl.racialAbilities(), &csl.racialAbilities());
    // a.checkEqual("62", csl.racialAbilities().getNumEntries(), 0U); <- cannot check this; HullAssignmentList is preloaded with default no-op entries

    Ref<HostConfiguration> rhostConfig = HostConfiguration::create();
    HostConfiguration& hostConfig = *rhostConfig;
    a.checkEqual("71. hullAssignments", &sl.hullAssignments(), &csl.hullAssignments());
    a.checkEqual("72. hullAssignments", csl.hullAssignments().getMaxIndex(hostConfig, 1), 0);

    a.checkEqual("81. componentNamer", &sl.componentNamer(), &csl.componentNamer());
    a.checkEqual("82. componentNamer", csl.componentNamer().getShortName(game::spec::ComponentNameProvider::Hull, 15, "SMALL DEEP SPACE FREIGHTER", ""), "SMALL DEEP SPACE FREIGHTER");

    a.checkEqual("91. friendlyCodes", &sl.friendlyCodes(), &csl.friendlyCodes());
    a.checkEqual("92. friendlyCodes", csl.friendlyCodes().size(), 0U);

    a.checkEqual("101. missions", &sl.missions(), &csl.missions());
    a.checkEqual("102. missions", csl.missions().size(), 0U);

    a.checkEqual("111. advantages", &sl.advantages(), &csl.advantages());
    a.checkEqual("112. advantages", sl.advantages().getNumAdvantages(), 0U);
}

/** Test racial abilities, simple case.
    Racial abilities created by configuration must be identified as such. */
AFL_TEST("game.spec.ShipList:findRacialAbilities", a)
{
    game::spec::ShipList testee;

    // Create some hulls
    testee.hulls().create(1);
    testee.hulls().create(2);
    testee.hulls().create(3);

    // Create host configuration
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity));

    a.checkNonNull("01. findEntry", p);
    a.checkEqual("02. m_addedPlayers", p->m_addedPlayers, PlayerSet_t() + 4 + 10);
    a.checkEqual("03. m_removedPlayers", p->m_removedPlayers, PlayerSet_t());
}

/** Test racial abilities, simple case with hull function.
    Racial abilities created by configuration must be identified as such, even when a ship has it as a real ability. */
AFL_TEST("game.spec.ShipList:findRacialAbilities:ssd", a)
{
    game::spec::ShipList testee;

    // Create some hulls, one of which has PlanetImmunity
    testee.hulls().create(1);
    testee.hulls().create(2)->changeHullFunction(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity), PlayerSet_t::allUpTo(game::MAX_PLAYERS), PlayerSet_t(), true);
    testee.hulls().create(3);

    // Create host configuration
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity));

    a.checkNonNull("01. findEntry", p);
    a.checkEqual("02. m_addedPlayers", p->m_addedPlayers, PlayerSet_t() + 4 + 10);
    a.checkEqual("03. m_removedPlayers", p->m_removedPlayers, PlayerSet_t());
}

/** Test racial abilities, sparse hull array.
    This is essentially the same as testRacialAbilities(), but the original implementation failed to work on a sparse hull list. */
AFL_TEST("game.spec.ShipList:findRacialAbilities:sparse", a)
{
    game::spec::ShipList testee;

    // Create some hulls
    testee.hulls().create(10);
    testee.hulls().create(20);
    testee.hulls().create(30);

    // Create host configuration
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity));

    a.checkNonNull("01. findEntry", p);
    a.checkEqual("02. m_addedPlayers", p->m_addedPlayers, PlayerSet_t() + 4 + 10);
    a.checkEqual("03. m_removedPlayers", p->m_removedPlayers, PlayerSet_t());
}

/** Test racial abilities, one hull.
    Tests a border case.
    One hull means every ability is a racial ability. */
AFL_TEST("game.spec.ShipList:findRacialAbilities:one-hull", a)
{
    game::spec::ShipList testee;

    // Create one hull that can cloak
    testee.hulls().create(1)->changeHullFunction(ModifiedHullFunctionList::Function_t(BasicHullFunction::Cloak), PlayerSet_t(9), PlayerSet_t(), true);

    // Create host configuration
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity));

    a.checkNonNull("01. findEntry", p);
    a.checkEqual("02. m_addedPlayers", p->m_addedPlayers, PlayerSet_t() + 4 + 10);
    a.checkEqual("03. m_removedPlayers", p->m_removedPlayers, PlayerSet_t());

    // Likewise, Cloak must be a racial ability
    p = testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::Cloak));
    a.checkNonNull("11. findEntry", p);
    a.checkEqual("12. m_addedPlayers", p->m_addedPlayers, PlayerSet_t(9));
    a.checkEqual("13. m_removedPlayers", p->m_removedPlayers, PlayerSet_t());
}

/** Test racial abilities, no hulls.
    Tests a border case.
    No hulls means no abilities. */
AFL_TEST("game.spec.ShipList:findRacialAbilities:no-hulls", a)
{
    game::spec::ShipList testee;

    // Create host configuration
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // No racial abilities
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity));

    a.checkNonNull("01. findEntry", p);
    a.check("02. m_addedPlayers", p->m_addedPlayers.empty());
    a.check("03. m_removedPlayers", p->m_removedPlayers.empty());
}

/** Test racial abilities, failure case.
    Ship abilities do not trigger racial ability detection. */
AFL_TEST("game.spec.ShipList:findRacialAbilities:failure", a)
{
    game::spec::ShipList testee;

    // Create one hull that can cloak WHEN ASSIGNED TO THE SHIP
    testee.hulls().create(1)->changeHullFunction(ModifiedHullFunctionList::Function_t(BasicHullFunction::Cloak), PlayerSet_t(9), PlayerSet_t(), false);

    // Do it
    Ref<HostConfiguration> rhostConfig = HostConfiguration::create();
    HostConfiguration& hostConfig = *rhostConfig;
    testee.findRacialAbilities(hostConfig);

    // Must not find Cloak.
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::Cloak));
    a.checkNull("01. findEntry", p);
}

/** Test querying of hull functions. */
AFL_TEST("game.spec.ShipList:hull-functions", a)
{
    game::spec::ShipList testee;

    const PlayerSet_t allPlayers = PlayerSet_t::allUpTo(game::MAX_PLAYERS);
    const PlayerSet_t noPlayers  = PlayerSet_t();
    const game::ExperienceLevelSet_t allLevels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);

    // Create a modified hull function.
    int modCloak = testee.modifiedHullFunctions().getFunctionIdFromDefinition(game::spec::HullFunction(BasicHullFunction::Cloak, game::ExperienceLevelSet_t(3)));

    // Create some hulls.
    testee.hulls().create(1)->changeHullFunction(ModifiedHullFunctionList::Function_t(BasicHullFunction::Cloak), allPlayers, noPlayers, true);
    testee.hulls().create(2)->changeHullFunction(modCloak,                                                  allPlayers, noPlayers, true);
    testee.hulls().create(3)->changeHullFunction(ModifiedHullFunctionList::Function_t(BasicHullFunction::Cloak), allPlayers, noPlayers, false);

    // Create a racial ability. Do NOT call findRacialAbilities().
    testee.racialAbilities().change(ModifiedHullFunctionList::Function_t(BasicHullFunction::Bioscan), PlayerSet_t(2), noPlayers);

    // Create a configuration
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[config.PlanetsAttackRebels].set(false);
    config[config.PlanetsAttackKlingons].set(false);
    config[config.AllowFedCombatBonus].set(false);
    config[config.AllowPrivateerTowCapture].set(false);
    config[config.AllowCrystalTowCapture].set(false);
    config[config.AntiCloakImmunity].set(false);

    // Check getPlayersThatCan().
    // - Cloak: available to 1 (general ability) and 2 (because the modified level is a subset of allLevels)
    a.checkEqual("01. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 1, config, allLevels), allPlayers);
    a.checkEqual("02. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 2, config, allLevels), allPlayers);
    a.checkEqual("03. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 3, config, allLevels), noPlayers);
    a.checkEqual("04. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 4, config, allLevels), noPlayers);

    // - Cloak, level 1: only available to 1 (general ability), not to 2 (mismatching level)
    a.checkEqual("11. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 1, config, game::ExperienceLevelSet_t(2)), allPlayers);
    a.checkEqual("12. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 2, config, game::ExperienceLevelSet_t(2)), noPlayers);
    a.checkEqual("13. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 3, config, game::ExperienceLevelSet_t(2)), noPlayers);
    a.checkEqual("14. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 4, config, game::ExperienceLevelSet_t(2)), noPlayers);

    // - Cloak, level 1: available to 1 (general ability), and 2 (matching level)
    a.checkEqual("21. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 1, config, game::ExperienceLevelSet_t(3)), allPlayers);
    a.checkEqual("22. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 2, config, game::ExperienceLevelSet_t(3)), allPlayers);
    a.checkEqual("23. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 3, config, game::ExperienceLevelSet_t(3)), noPlayers);
    a.checkEqual("24. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Cloak, 4, config, game::ExperienceLevelSet_t(3)), noPlayers);

    // - Bioscan: available to all existing hulls for player 2 (racial ability)
    a.checkEqual("31. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Bioscan, 1, config, allLevels), PlayerSet_t(2));
    a.checkEqual("32. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Bioscan, 2, config, allLevels), PlayerSet_t(2));
    a.checkEqual("33. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Bioscan, 3, config, allLevels), PlayerSet_t(2));
    a.checkEqual("34. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::Bioscan, 4, config, allLevels), noPlayers);

    // - PlanetImmunity: available to all existing hulls for player 4+10 (implied)
    a.checkEqual("41. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity, 1, config, allLevels), PlayerSet_t() + 4 + 10);
    a.checkEqual("42. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity, 2, config, allLevels), PlayerSet_t() + 4 + 10);
    a.checkEqual("43. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity, 3, config, allLevels), PlayerSet_t() + 4 + 10);
    a.checkEqual("44. getPlayersThatCan", testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity, 4, config, allLevels), noPlayers);

    // Check enumerateHullFunctions
    // - Hull 1
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 1, config, allPlayers, allLevels, true, true);
        list.sortForNewShip(allPlayers);

        // Result should be: Cloak [H], PlanetImmunity [H], Bioscan [R]
        a.checkEqual("51. size", list.size(), 3U);
        a.checkEqual("52. getBasicFunctionId", list[0].getBasicFunctionId(), BasicHullFunction::Cloak);
        a.checkEqual("53. getPlayers", list[0].getPlayers(), allPlayers);
        a.checkEqual("54. getKind", list[0].getKind(), HullFunction::AssignedToHull);

        a.checkEqual("61. getBasicFunctionId", list[1].getBasicFunctionId(), BasicHullFunction::PlanetImmunity);
        a.checkEqual("62. getPlayers", list[1].getPlayers(), PlayerSet_t() + 4 + 10);
        a.checkEqual("63. getKind", list[1].getKind(), HullFunction::AssignedToHull);

        a.checkEqual("71. getBasicFunctionId", list[2].getBasicFunctionId(), BasicHullFunction::Bioscan);
        a.checkEqual("72. getPlayers", list[2].getPlayers(), PlayerSet_t(2));
        a.checkEqual("73. getKind", list[2].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 2
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 2, config, allPlayers, allLevels, true, true);
        list.sortForNewShip(allPlayers);

        // Result should be: PlanetImmunity [H], Cloak [H], Bioscan [R]
        a.checkEqual("81. size", list.size(), 3U);
        a.checkEqual("82. getBasicFunctionId", list[0].getBasicFunctionId(), BasicHullFunction::PlanetImmunity);
        a.checkEqual("83. getPlayers", list[0].getPlayers(), PlayerSet_t() + 4 + 10);
        a.checkEqual("84. getKind", list[0].getKind(), HullFunction::AssignedToHull);
        a.checkEqual("85. getLevels", list[0].getLevels(), allLevels);

        a.checkEqual("91. getBasicFunctionId", list[1].getBasicFunctionId(), BasicHullFunction::Cloak);
        a.checkEqual("92. getPlayers", list[1].getPlayers(), allPlayers);
        a.checkEqual("93. getKind", list[1].getKind(), HullFunction::AssignedToHull);
        a.checkEqual("94. getLevels", list[1].getLevels(), game::ExperienceLevelSet_t(3));

        a.checkEqual("101. getBasicFunctionId", list[2].getBasicFunctionId(), BasicHullFunction::Bioscan);
        a.checkEqual("102. getPlayers", list[2].getPlayers(), PlayerSet_t(2));
        a.checkEqual("103. getKind", list[2].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 3
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, allPlayers, allLevels, true, true);
        list.sortForNewShip(allPlayers);

        // Result should be: PlanetImmunity [H], Cloak [S], Bioscan [R]
        a.checkEqual("111. size", list.size(), 3U);
        a.checkEqual("112. getBasicFunctionId", list[0].getBasicFunctionId(), BasicHullFunction::Cloak);
        a.checkEqual("113. getPlayers", list[0].getPlayers(), allPlayers);
        a.checkEqual("114. getKind", list[0].getKind(), HullFunction::AssignedToShip);
        a.checkEqual("115. getLevels", list[0].getLevels(), allLevels);

        a.checkEqual("121. getBasicFunctionId", list[1].getBasicFunctionId(), BasicHullFunction::PlanetImmunity);
        a.checkEqual("122. getPlayers", list[1].getPlayers(), PlayerSet_t() + 4 + 10);
        a.checkEqual("123. getKind", list[1].getKind(), HullFunction::AssignedToHull);
        a.checkEqual("124. getLevels", list[1].getLevels(), allLevels);

        a.checkEqual("131. getBasicFunctionId", list[2].getBasicFunctionId(), BasicHullFunction::Bioscan);
        a.checkEqual("132. getPlayers", list[2].getPlayers(), PlayerSet_t(2));
        a.checkEqual("133. getKind", list[2].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 3, limited
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, PlayerSet_t(2), allLevels, false, false);
        list.sortForNewShip(allPlayers);

        // Should be empty: PlanetImmunity not in race selection, Cloak deselected by includeNewShip=false, Bioscan deselected by includeRacialAbilities=false
        a.checkEqual("141. size", list.size(), 0U);
    }
    // - Hull 3, limited
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, PlayerSet_t(2), allLevels, false, true);
        list.sortForNewShip(allPlayers);

        // Only Bioscan, everything else filtered
        a.checkEqual("151. size", list.size(), 1U);
        a.checkEqual("152. getBasicFunctionId", list[0].getBasicFunctionId(), BasicHullFunction::Bioscan);
        a.checkEqual("153. getPlayers", list[0].getPlayers(), PlayerSet_t(2));
        a.checkEqual("154. getKind", list[0].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 3, limited
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, PlayerSet_t(3), allLevels, false, true);
        list.sortForNewShip(allPlayers);

        // Should be empty; only racial abilities selected but filtered by player
        a.checkEqual("161. size", list.size(), 0U);
    }
    // - Hull 4 (nonexistant)
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 4, config, allPlayers, allLevels, true, true);
        a.checkEqual("162. size", list.size(), 0U);
    }
}

/** Test racial abilities, many abilities.
    Tests many hulls, many abilities. */
AFL_TEST("game.spec.ShipList:findRacialAbilities:many", a)
{
    game::spec::ShipList testee;

    // Create 10 hulls with 5 functions each
    for (int i = 1; i <= 10; ++i) {
        game::spec::Hull* pHull = testee.hulls().create(i);
        a.checkNonNull("01", pHull);

        for (int f = 1; f <= 5; ++f) {
            pHull->changeHullFunction(ModifiedHullFunctionList::Function_t(f), PlayerSet_t::allUpTo(9), PlayerSet_t(), true);
        }
    }

    // Some more hulls that don't have functions; they don't even have the associated slot.
    for (int i = 11; i <= 15; ++i) {
        testee.hulls().create(i);
    }

    // Do it
    Ref<HostConfiguration> rhostConfig = HostConfiguration::create();
    HostConfiguration& hostConfig = *rhostConfig;
    testee.findRacialAbilities(hostConfig);

    // The functions must not be converted to racial abilities
    for (int f = 1; f <= 5; ++f) {
        const game::spec::HullFunctionAssignmentList::Entry* p =
            testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(f));
        a.checkNull("11", p);
    }
}

/** Test findSpecimenHullForFunction(). */
AFL_TEST("game.spec.ShipList:findSpecimenHullForFunction", a)
{
    game::spec::ShipList testee;

    const ModifiedHullFunctionList::Function_t f1 = testee.modifiedHullFunctions().getFunctionIdFromHostId(1);
    const ModifiedHullFunctionList::Function_t f2 = testee.modifiedHullFunctions().getFunctionIdFromHostId(2);
    const ModifiedHullFunctionList::Function_t f3 = testee.modifiedHullFunctions().getFunctionIdFromHostId(3);

    game::spec::Hull* nullh = 0;

    // Hull 1 has f1 for all players
    game::spec::Hull* h1 = testee.hulls().create(1);
    h1->changeHullFunction(f1, PlayerSet_t::allUpTo(10), PlayerSet_t(), true);

    // Hull 2 has f2 for all players
    game::spec::Hull* h2 = testee.hulls().create(2);
    h2->changeHullFunction(f2, PlayerSet_t::allUpTo(10), PlayerSet_t(), true);

    // Hull 3 has f3 for player 4 only
    game::spec::Hull* h3 = testee.hulls().create(3);
    h3->changeHullFunction(f3, PlayerSet_t(4), PlayerSet_t(), true);

    // Hull 4 has f2 for player 5 only
    game::spec::Hull* h4 = testee.hulls().create(4);
    h4->changeHullFunction(f2, PlayerSet_t(5), PlayerSet_t(), true);

    // Only player 5 can build hull 4
    testee.hullAssignments().add(5, 1, 4);

    // Tests follow:
    Ref<HostConfiguration> rhostConfig = HostConfiguration::create();
    HostConfiguration& hostConfig = *rhostConfig;

    // f1 -> hull 1 when searched for all or single player
    a.checkEqual("01", testee.findSpecimenHullForFunction(1, hostConfig, PlayerSet_t::allUpTo(10), PlayerSet_t(), true), h1);
    a.checkEqual("02", testee.findSpecimenHullForFunction(1, hostConfig, PlayerSet_t(3), PlayerSet_t(), true), h1);

    a.checkEqual("06", testee.findSpecimenHullForFunction(1, hostConfig, PlayerSet_t::allUpTo(10), PlayerSet_t(), false), h1);
    a.checkEqual("07", testee.findSpecimenHullForFunction(1, hostConfig, PlayerSet_t(3), PlayerSet_t(), false), h1);

    // f2 -> hull 2 when searched for all or single player except for player 5 (ambiguous)
    // But it's not ambiguous if we only check player 5's ships.
    a.checkEqual("11", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t::allUpTo(10), PlayerSet_t(), true), h2);
    a.checkEqual("12", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t(3), PlayerSet_t(), true), h2);
    a.checkEqual("13", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t(5), PlayerSet_t(), true), nullh);
    a.checkEqual("14", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t(5), PlayerSet_t(5), true), h4);

    a.checkEqual("16", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t::allUpTo(10), PlayerSet_t(), false), h2);
    a.checkEqual("17", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t(3), PlayerSet_t(), false), h2);
    a.checkEqual("18", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t(5), PlayerSet_t(), false), h2);
    a.checkEqual("19", testee.findSpecimenHullForFunction(2, hostConfig, PlayerSet_t(5), PlayerSet_t(5), false), h4);

    // f3 -> hull 2 only for player 4
    a.checkEqual("21", testee.findSpecimenHullForFunction(3, hostConfig, PlayerSet_t::allUpTo(10), PlayerSet_t(), true), nullh);
    a.checkEqual("22", testee.findSpecimenHullForFunction(3, hostConfig, PlayerSet_t(3), PlayerSet_t(), true), nullh);
    a.checkEqual("23", testee.findSpecimenHullForFunction(3, hostConfig, PlayerSet_t(4), PlayerSet_t(), true), h3);

    // f4 for nobody
    a.checkEqual("31", testee.findSpecimenHullForFunction(4, hostConfig, PlayerSet_t::allUpTo(10), PlayerSet_t(), true), nullh);
    a.checkEqual("32", testee.findSpecimenHullForFunction(4, hostConfig, PlayerSet_t(3), PlayerSet_t(), true), nullh);
    a.checkEqual("33", testee.findSpecimenHullForFunction(4, hostConfig, PlayerSet_t(4), PlayerSet_t(), true), nullh);
}

/** Test getComponent(). */
AFL_TEST("game.spec.ShipList:getComponent", a)
{
    game::spec::ShipList testee;
    game::spec::Hull* h            = testee.hulls().create(66);
    game::spec::Engine* e          = testee.engines().create(77);
    game::spec::Beam* b            = testee.beams().create(88);
    game::spec::TorpedoLauncher* t = testee.launchers().create(99);

    a.checkEqual("01", testee.getComponent(game::HullTech,    66), h);
    a.checkEqual("02", testee.getComponent(game::EngineTech,  77), e);
    a.checkEqual("03", testee.getComponent(game::BeamTech,    88), b);
    a.checkEqual("04", testee.getComponent(game::TorpedoTech, 99), t);

    a.checkNull("11", testee.getComponent(game::HullTech, 55)   );
    a.checkNull("12", testee.getComponent(game::EngineTech, 55) );
    a.checkNull("13", testee.getComponent(game::BeamTech, 55)   );
    a.checkNull("14", testee.getComponent(game::TorpedoTech, 55));
}
