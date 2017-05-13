/**
  *  \file u/t_game_spec_shiplist.cpp
  *  \brief Test for game::spec::ShipList
  */

#include "game/spec/shiplist.hpp"

#include "t_game_spec.hpp"

using game::spec::ModifiedHullFunctionList;
using game::spec::HullFunction;

/** Basic accessor test. */
void
TestGameSpecShipList::testIt()
{
    // Testee
    game::spec::ShipList sl;
    const game::spec::ShipList& csl = sl;

    // Verify components: const and non-const must be the same object; tables must be empty
    TS_ASSERT_EQUALS(&sl.beams(), &csl.beams());
    TS_ASSERT(csl.beams().findNext(0) == 0);

    TS_ASSERT_EQUALS(&sl.engines(), &csl.engines());
    TS_ASSERT(csl.engines().findNext(0) == 0);

    TS_ASSERT_EQUALS(&sl.launchers(), &csl.launchers());
    TS_ASSERT(csl.launchers().findNext(0) == 0);

    TS_ASSERT_EQUALS(&sl.hulls(), &csl.hulls());
    TS_ASSERT(csl.hulls().findNext(0) == 0);

    // Verify hull function stuff: const and non-const must be the same object; state must imply emptiness
    // (we cannot directly verify emptiness.)
    TS_ASSERT_EQUALS(&sl.basicHullFunctions(), &csl.basicHullFunctions());
    TS_ASSERT(csl.basicHullFunctions().getFunctionById(0) == 0);

    TS_ASSERT_EQUALS(&sl.modifiedHullFunctions(), &csl.modifiedHullFunctions());
    TS_ASSERT(csl.modifiedHullFunctions().getFunctionIdFromHostId(42) == 42);

    TS_ASSERT_EQUALS(&sl.racialAbilities(), &csl.racialAbilities());
    // TS_ASSERT_EQUALS(csl.racialAbilities().getNumEntries(), 0U); <- cannot check this; HullAssignmentList is preloaded with default no-op entries

    TS_ASSERT_EQUALS(&sl.hullAssignments(), &csl.hullAssignments());
    TS_ASSERT_EQUALS(csl.hullAssignments().getMaxIndex(game::config::HostConfiguration(), 1), 0);

    TS_ASSERT_EQUALS(&sl.componentNamer(), &csl.componentNamer());
    TS_ASSERT_EQUALS(csl.componentNamer().getShortName(game::spec::ComponentNameProvider::Hull, 15, "SMALL DEEP SPACE FREIGHTER", ""), "SMALL DEEP SPACE FREIGHTER");

    TS_ASSERT_EQUALS(&sl.friendlyCodes(), &csl.friendlyCodes());
    TS_ASSERT_EQUALS(csl.friendlyCodes().size(), 0U);

    TS_ASSERT_EQUALS(&sl.missions(), &csl.missions());
    TS_ASSERT_EQUALS(csl.missions().size(), 0U);
}

/** Test racial abilities, simple case.
    Racial abilities created by configuration must be identified as such. */
void
TestGameSpecShipList::testRacialAbilities()
{
    game::spec::ShipList testee;

    // Create some hulls
    testee.hulls().create(1);
    testee.hulls().create(2);
    testee.hulls().create(3);

    // Create host configuration
    game::config::HostConfiguration config;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity));

    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t());
}

/** Test racial abilities, simple case with hull function.
    Racial abilities created by configuration must be identified as such, even when a ship has it as a real ability. */
void
TestGameSpecShipList::testRacialAbilitiesSSD()
{
    game::spec::ShipList testee;

    // Create some hulls, one of which has PlanetImmunity
    testee.hulls().create(1);
    testee.hulls().create(2)->changeHullFunction(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS), game::PlayerSet_t(), true);
    testee.hulls().create(3);

    // Create host configuration
    game::config::HostConfiguration config;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity));

    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t());
}

/** Test racial abilities, sparse hull array.
    This is essentially the same as testRacialAbilities(), but the original implementation failed to work on a sparse hull list. */
void
TestGameSpecShipList::testRacialAbilitiesSparse()
{
    game::spec::ShipList testee;

    // Create some hulls
    testee.hulls().create(10);
    testee.hulls().create(20);
    testee.hulls().create(30);

    // Create host configuration
    game::config::HostConfiguration config;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity));

    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t());
}

/** Test racial abilities, one hull.
    Tests a border case.
    One hull means every ability is a racial ability. */
void
TestGameSpecShipList::testRacialAbilitiesOne()
{
    game::spec::ShipList testee;

    // Create one hull that can cloak
    testee.hulls().create(1)->changeHullFunction(ModifiedHullFunctionList::Function_t(HullFunction::Cloak), game::PlayerSet_t(9), game::PlayerSet_t(), true);

    // Create host configuration
    game::config::HostConfiguration config;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // PlanetImmunity must now be registered as racial abilities for 4+10
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity));

    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t());

    // Likewise, Cloak must be a racial ability
    p = testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::Cloak));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t(9));
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t());
}

/** Test racial abilities, no hulls.
    Tests a border case.
    No hulls means no abilities. */
void
TestGameSpecShipList::testRacialAbilitiesEmpty()
{
    game::spec::ShipList testee;

    // Create host configuration
    game::config::HostConfiguration config;
    config.setDefaultValues();
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);

    // Do it
    testee.findRacialAbilities(config);

    // No racial abilities
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::PlanetImmunity));

    TS_ASSERT(p != 0);
    TS_ASSERT(p->m_addedPlayers.empty());
    TS_ASSERT(p->m_removedPlayers.empty());
}

/** Test racial abilities, failure case.
    Ship abilities do not trigger racial ability detection. */
void
TestGameSpecShipList::testRacialAbilitiesFail()
{
    game::spec::ShipList testee;

    // Create one hull that can cloak WHEN ASSIGNED TO THE SHIP
    testee.hulls().create(1)->changeHullFunction(ModifiedHullFunctionList::Function_t(HullFunction::Cloak), game::PlayerSet_t(9), game::PlayerSet_t(), false);

    // Do it
    testee.findRacialAbilities(game::config::HostConfiguration());

    // Must not find Cloak.
    const game::spec::HullFunctionAssignmentList::Entry* p =
        testee.racialAbilities().findEntry(ModifiedHullFunctionList::Function_t(HullFunction::Cloak));
    TS_ASSERT(p == 0);
}

/** Test querying of hull functions. */
void
TestGameSpecShipList::testGetHullFunctions()
{
    game::spec::ShipList testee;

    const game::PlayerSet_t allPlayers = game::PlayerSet_t::allUpTo(game::MAX_PLAYERS);
    const game::PlayerSet_t noPlayers  = game::PlayerSet_t();
    const game::ExperienceLevelSet_t allLevels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);

    // Create a modified hull function.
    int modCloak = testee.modifiedHullFunctions().getFunctionIdFromDefinition(HullFunction(HullFunction::Cloak, game::ExperienceLevelSet_t(3)));

    // Create some hulls.
    testee.hulls().create(1)->changeHullFunction(ModifiedHullFunctionList::Function_t(HullFunction::Cloak), allPlayers, noPlayers, true);
    testee.hulls().create(2)->changeHullFunction(modCloak,                                                  allPlayers, noPlayers, true);
    testee.hulls().create(3)->changeHullFunction(ModifiedHullFunctionList::Function_t(HullFunction::Cloak), allPlayers, noPlayers, false);

    // Create a racial ability. Do NOT call findRacialAbilities().
    testee.racialAbilities().change(ModifiedHullFunctionList::Function_t(HullFunction::Bioscan), game::PlayerSet_t(2), noPlayers);

    // Create a configuration
    game::config::HostConfiguration config;
    config[config.PlanetsAttackRebels].set(false);
    config[config.PlanetsAttackKlingons].set(false);
    config[config.AllowFedCombatBonus].set(false);
    config[config.AllowPrivateerTowCapture].set(false);
    config[config.AllowCrystalTowCapture].set(false);
    config[config.AntiCloakImmunity].set(false);

    // Check getPlayersThatCan().
    // - Cloak: available to 1 (general ability) and 2 (because the modified level is a subset of allLevels)
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 1, config, allLevels), allPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 2, config, allLevels), allPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 3, config, allLevels), noPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 4, config, allLevels), noPlayers);

    // - Cloak, level 1: only available to 1 (general ability), not to 2 (mismatching level)
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 1, config, game::ExperienceLevelSet_t(2)), allPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 2, config, game::ExperienceLevelSet_t(2)), noPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 3, config, game::ExperienceLevelSet_t(2)), noPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 4, config, game::ExperienceLevelSet_t(2)), noPlayers);

    // - Cloak, level 1: available to 1 (general ability), and 2 (matching level)
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 1, config, game::ExperienceLevelSet_t(3)), allPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 2, config, game::ExperienceLevelSet_t(3)), allPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 3, config, game::ExperienceLevelSet_t(3)), noPlayers);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Cloak, 4, config, game::ExperienceLevelSet_t(3)), noPlayers);

    // - Bioscan: available to all existing hulls for player 2 (racial ability)
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Bioscan, 1, config, allLevels), game::PlayerSet_t(2));
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Bioscan, 2, config, allLevels), game::PlayerSet_t(2));
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Bioscan, 3, config, allLevels), game::PlayerSet_t(2));
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::Bioscan, 4, config, allLevels), noPlayers);

    // - PlanetImmunity: available to all existing hulls for player 4+10 (implied)
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::PlanetImmunity, 1, config, allLevels), game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::PlanetImmunity, 2, config, allLevels), game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::PlanetImmunity, 3, config, allLevels), game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(HullFunction::PlanetImmunity, 4, config, allLevels), noPlayers);

    // Check enumerateHullFunctions
    // - Hull 1
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 1, config, allPlayers, allLevels, true, true);
        list.sortForNewShip(allPlayers);

        // Result should be: Cloak [H], PlanetImmunity [H], Bioscan [R]
        TS_ASSERT_EQUALS(list.size(), 3U);
        TS_ASSERT_EQUALS(list[0].getBasicFunctionId(), HullFunction::Cloak);
        TS_ASSERT_EQUALS(list[0].getPlayers(), allPlayers);
        TS_ASSERT_EQUALS(list[0].getKind(), HullFunction::AssignedToHull);

        TS_ASSERT_EQUALS(list[1].getBasicFunctionId(), HullFunction::PlanetImmunity);
        TS_ASSERT_EQUALS(list[1].getPlayers(), game::PlayerSet_t() + 4 + 10);
        TS_ASSERT_EQUALS(list[1].getKind(), HullFunction::AssignedToHull);

        TS_ASSERT_EQUALS(list[2].getBasicFunctionId(), HullFunction::Bioscan);
        TS_ASSERT_EQUALS(list[2].getPlayers(), game::PlayerSet_t(2));
        TS_ASSERT_EQUALS(list[2].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 2
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 2, config, allPlayers, allLevels, true, true);
        list.sortForNewShip(allPlayers);

        // Result should be: PlanetImmunity [H], Cloak [H], Bioscan [R]
        TS_ASSERT_EQUALS(list.size(), 3U);
        TS_ASSERT_EQUALS(list[0].getBasicFunctionId(), HullFunction::PlanetImmunity);
        TS_ASSERT_EQUALS(list[0].getPlayers(), game::PlayerSet_t() + 4 + 10);
        TS_ASSERT_EQUALS(list[0].getKind(), HullFunction::AssignedToHull);
        TS_ASSERT_EQUALS(list[0].getLevels(), allLevels);

        TS_ASSERT_EQUALS(list[1].getBasicFunctionId(), HullFunction::Cloak);
        TS_ASSERT_EQUALS(list[1].getPlayers(), allPlayers);
        TS_ASSERT_EQUALS(list[1].getKind(), HullFunction::AssignedToHull);
        TS_ASSERT_EQUALS(list[1].getLevels(), game::ExperienceLevelSet_t(3));

        TS_ASSERT_EQUALS(list[2].getBasicFunctionId(), HullFunction::Bioscan);
        TS_ASSERT_EQUALS(list[2].getPlayers(), game::PlayerSet_t(2));
        TS_ASSERT_EQUALS(list[2].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 3
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, allPlayers, allLevels, true, true);
        list.sortForNewShip(allPlayers);

        // Result should be: PlanetImmunity [H], Cloak [S], Bioscan [R]
        TS_ASSERT_EQUALS(list.size(), 3U);
        TS_ASSERT_EQUALS(list[0].getBasicFunctionId(), HullFunction::Cloak);
        TS_ASSERT_EQUALS(list[0].getPlayers(), allPlayers);
        TS_ASSERT_EQUALS(list[0].getKind(), HullFunction::AssignedToShip);
        TS_ASSERT_EQUALS(list[0].getLevels(), allLevels);

        TS_ASSERT_EQUALS(list[1].getBasicFunctionId(), HullFunction::PlanetImmunity);
        TS_ASSERT_EQUALS(list[1].getPlayers(), game::PlayerSet_t() + 4 + 10);
        TS_ASSERT_EQUALS(list[1].getKind(), HullFunction::AssignedToHull);
        TS_ASSERT_EQUALS(list[1].getLevels(), allLevels);

        TS_ASSERT_EQUALS(list[2].getBasicFunctionId(), HullFunction::Bioscan);
        TS_ASSERT_EQUALS(list[2].getPlayers(), game::PlayerSet_t(2));
        TS_ASSERT_EQUALS(list[2].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 3, limited
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, game::PlayerSet_t(2), allLevels, false, false);
        list.sortForNewShip(allPlayers);

        // Should be empty: PlanetImmunity not in race selection, Cloak deselected by includeNewShip=false, Bioscan deselected by includeRacialAbilities=false
        TS_ASSERT_EQUALS(list.size(), 0U);
    }
    // - Hull 3, limited
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, game::PlayerSet_t(2), allLevels, false, true);
        list.sortForNewShip(allPlayers);

        // Only Bioscan, everything else filtered
        TS_ASSERT_EQUALS(list.size(), 1U);
        TS_ASSERT_EQUALS(list[0].getBasicFunctionId(), HullFunction::Bioscan);
        TS_ASSERT_EQUALS(list[0].getPlayers(), game::PlayerSet_t(2));
        TS_ASSERT_EQUALS(list[0].getKind(), HullFunction::AssignedToRace);
    }
    // - Hull 3, limited
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 3, config, game::PlayerSet_t(3), allLevels, false, true);
        list.sortForNewShip(allPlayers);

        // Should be empty; only racial abilities selected but filtered by player
        TS_ASSERT_EQUALS(list.size(), 0U);
    }
    // - Hull 4 (nonexistant)
    {
        game::spec::HullFunctionList list;
        testee.enumerateHullFunctions(list, 4, config, allPlayers, allLevels, true, true);
        TS_ASSERT_EQUALS(list.size(), 0U);
    }
}

