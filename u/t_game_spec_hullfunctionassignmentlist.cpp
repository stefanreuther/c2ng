/**
  *  \file u/t_game_spec_hullfunctionassignmentlist.cpp
  *  \brief Test for game::spec::HullFunctionAssignmentList
  */

#include "game/spec/hullfunctionassignmentlist.hpp"

#include "t_game_spec.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/basichullfunctionlist.hpp"

using game::spec::ModifiedHullFunctionList;

/** Accessor tests. */
void
TestGameSpecHullFunctionAssignmentList::testIt()
{
    using game::spec::HullFunctionAssignmentList;

    HullFunctionAssignmentList testee;

    // Add some functions
    // - player 1 does 42
    testee.change(ModifiedHullFunctionList::Function_t(42), game::PlayerSet_t(1), game::PlayerSet_t());
    // - everyone does 77
    testee.change(ModifiedHullFunctionList::Function_t(77), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS), game::PlayerSet_t());
    // - wait, 2 does not do 77
    testee.change(ModifiedHullFunctionList::Function_t(77), game::PlayerSet_t(), game::PlayerSet_t(2));

    // Verify iteration
    {
        bool found42 = false;
        bool found77 = false;
        for (size_t i = 0; i < testee.getNumEntries(); ++i) {
            const HullFunctionAssignmentList::Entry* p = testee.getEntryByIndex(i);
            TS_ASSERT(p != 0);
            if (p->m_function == 42) {
                found42 = true;
            }
            if (p->m_function == 77) {
                found77 = true;
            }
        }
        TS_ASSERT(found42);
        TS_ASSERT(found77);
    }

    // Verify lookup
    TS_ASSERT(testee.findEntry(ModifiedHullFunctionList::Function_t(42)) != 0);
    TS_ASSERT(testee.findEntry(ModifiedHullFunctionList::Function_t(77)) != 0);
    TS_ASSERT(testee.findEntry(ModifiedHullFunctionList::Function_t(99)) == 0);

    TS_ASSERT(testee.getEntryByIndex(testee.getNumEntries()) == 0);

    // Lookup
    for (int player = 1; player <= 3; ++player) {
        // Query the list
        ModifiedHullFunctionList modList;
        game::config::HostConfiguration config;
        config[config.AllowFedCombatBonus].set(true);
        game::spec::Hull hull(2);
        game::spec::HullFunctionList result;
        testee.getAll(result, modList, config, hull, game::PlayerSet_t(player), game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS), game::spec::HullFunction::AssignedToHull);

        bool found42 = false;
        bool found77 = false;
        bool foundFullWeaponry = false;
        for (size_t i = 0, n = result.size(); i < n; ++i) {
            TS_ASSERT_EQUALS(result[i].getKind(), game::spec::HullFunction::AssignedToHull);
            switch (result[i].getBasicFunctionId()) {
             case 42: found42 = true; break;
             case 77: found77 = true; break;
             case game::spec::BasicHullFunction::FullWeaponry: foundFullWeaponry = true; break;
            }
        }

        switch (player) {
         case 1:
            // Player 1 has all functions (explicitly set and implicitly given)
            TS_ASSERT(found42);
            TS_ASSERT(found77);
            TS_ASSERT(foundFullWeaponry);
            break;
         case 2:
            // Player 2 has no functions
            TS_ASSERT(!found42);
            TS_ASSERT(!found77);
            TS_ASSERT(!foundFullWeaponry);
            break;
         case 3:
            // Player 3 has function 77 (given to all but 2)
            TS_ASSERT(!found42);
            TS_ASSERT(found77);
            TS_ASSERT(!foundFullWeaponry);
            break;
        }

        // Query the player set
        game::spec::BasicHullFunctionList basicList;
        TS_ASSERT_EQUALS(testee.getPlayersThatCan(42, modList, basicList, config, hull, game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS), true), game::PlayerSet_t(1));
    }
}

/** Test getPlayersThatCan with an implied function. */
void
TestGameSpecHullFunctionAssignmentList::testGetPlayerImplied()
{
    // Lists
    ModifiedHullFunctionList modList;
    game::spec::BasicHullFunctionList basicList;
    game::config::HostConfiguration config;
    game::spec::HullFunctionAssignmentList testee;

    // Add a function: Tow implies This
    game::spec::BasicHullFunction* towFunction = basicList.addFunction(game::spec::BasicHullFunction::Tow, "Tow");
    towFunction->setImpliedFunctionId(44);
    basicList.addFunction(44, "This");

    // Make a hull with two engines
    game::spec::Hull hull(3);
    hull.setNumEngines(2);

    const game::ExperienceLevelSet_t allLevels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);

    // Check
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(44, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));

    // Remove Tow for feds
    testee.change(game::spec::BasicHullFunction::Tow, game::PlayerSet_t(), game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(44, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 1);

    // Change hull so that implied-tow no longer applies
    config[config.AllowOneEngineTowing].set(false);
    hull.setNumEngines(1);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(44, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t());
}

/** Test behaviour of merged implied function. */
void
TestGameSpecHullFunctionAssignmentList::testMerged()
{
    // Lists
    ModifiedHullFunctionList modList;
    game::spec::BasicHullFunctionList basicList;
    game::config::HostConfiguration config;
    game::spec::HullFunctionAssignmentList testee;
    game::spec::Hull hull(3);

    // Configure
    const int fn = game::spec::BasicHullFunction::PlanetImmunity;
    config[config.PlanetsAttackKlingons].set(false);
    config[config.PlanetsAttackRebels].set(false);
    testee.change(ModifiedHullFunctionList::Function_t(fn), game::PlayerSet_t(3), game::PlayerSet_t());

    const game::ExperienceLevelSet_t allLevels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);

    // Verify
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(fn, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 3 + 4 + 10);
}

/** Test all defaulted functions. */
void
TestGameSpecHullFunctionAssignmentList::testDefaulted()
{
    ModifiedHullFunctionList modList;
    game::spec::BasicHullFunctionList basicList;
    game::config::HostConfiguration config;
    game::spec::HullFunctionAssignmentList testee;
    game::spec::Hull hull(3);
    const game::ExperienceLevelSet_t allLevels = game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS);

    config[config.AllowOneEngineTowing].set(true);
    config[config.AllowFedCombatBonus].set(true);
    config[config.AllowPrivateerTowCapture].set(true);
    config[config.AllowCrystalTowCapture].set(true);
    config[config.PlanetsAttackRebels].set(false);
    config[config.PlanetsAttackKlingons].set(false);
    config[config.AntiCloakImmunity].set("true,true,true,false");

    // Verify
    using game::spec::BasicHullFunction;
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::Tow,               modList, basicList, config, hull, allLevels, true), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::Boarding,          modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 5 + 7);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::AntiCloakImmunity, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 1 + 2 + 3);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity,    modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 4 + 10);
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::FullWeaponry,      modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 1);

    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::Tow,               modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::Boarding,          modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::AntiCloakImmunity, modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity,    modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    TS_ASSERT_EQUALS(testee.getPlayersThatCan(BasicHullFunction::FullWeaponry,      modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
}

/** Test remove(). */
void
TestGameSpecHullFunctionAssignmentList::testRemove()
{
    game::spec::HullFunctionAssignmentList testee;

    // Add something
    testee.change(ModifiedHullFunctionList::Function_t(100), game::PlayerSet_t(1), game::PlayerSet_t());
    testee.change(ModifiedHullFunctionList::Function_t(101), game::PlayerSet_t(2), game::PlayerSet_t());

    size_t n = testee.getNumEntries();
    TS_ASSERT(n >= 2);

    // Remove
    testee.removeEntry(ModifiedHullFunctionList::Function_t(100));
    size_t n2 = testee.getNumEntries();
    TS_ASSERT_EQUALS(n, n2+1);
    TS_ASSERT(n2 >= 1);

    // Remove same again [no change]
    testee.removeEntry(ModifiedHullFunctionList::Function_t(100));
    TS_ASSERT_EQUALS(testee.getNumEntries(), n2);

    // Out-of-range access
    TS_ASSERT(testee.getEntryByIndex(n2) == 0);
    TS_ASSERT(testee.getEntryByIndex(n) == 0);

    TS_ASSERT(testee.getEntryByIndex(n2-1) != 0);
}

/** Test sequence of add/remove.
    change() is defined as add-then-remove.
    That is, if a player is contained in add and remove, it ultimately ends in remove. */
void
TestGameSpecHullFunctionAssignmentList::testSequence()
{
    using game::spec::BasicHullFunction;

    game::spec::HullFunctionAssignmentList testee;

    // Modify something from the variable-default set
    // - in one action
    testee.change(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity), game::PlayerSet_t(1) + 2, game::PlayerSet_t(2) + 3);

    // - in two actions
    testee.change(ModifiedHullFunctionList::Function_t(BasicHullFunction::Tow),            game::PlayerSet_t(1) + 2, game::PlayerSet_t());
    testee.change(ModifiedHullFunctionList::Function_t(BasicHullFunction::Tow),            game::PlayerSet_t(),      game::PlayerSet_t(2) + 3);

    // Modify something outside the variable-default set
    // - in one action
    testee.change(ModifiedHullFunctionList::Function_t(100),                          game::PlayerSet_t(1) + 2, game::PlayerSet_t(2) + 3);

    // - in two actions
    testee.change(ModifiedHullFunctionList::Function_t(101),                          game::PlayerSet_t(1) + 2, game::PlayerSet_t());
    testee.change(ModifiedHullFunctionList::Function_t(101),                          game::PlayerSet_t(),      game::PlayerSet_t(2) + 3);

    // Verify. All four must be "+1", "-23".
    const game::spec::HullFunctionAssignmentList::Entry* p = testee.findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::PlanetImmunity));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t(2) + 3);

    p = testee.findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::Tow));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t(2) + 3);

    p = testee.findEntry(ModifiedHullFunctionList::Function_t(100));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t(2) + 3);

    p = testee.findEntry(ModifiedHullFunctionList::Function_t(101));
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->m_addedPlayers, game::PlayerSet_t(1));
    TS_ASSERT_EQUALS(p->m_removedPlayers, game::PlayerSet_t(2) + 3);
}

