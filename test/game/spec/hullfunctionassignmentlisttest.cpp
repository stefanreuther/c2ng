/**
  *  \file test/game/spec/hullfunctionassignmentlisttest.cpp
  *  \brief Test for game::spec::HullFunctionAssignmentList
  */

#include "game/spec/hullfunctionassignmentlist.hpp"

#include "afl/test/testrunner.hpp"
#include "game/spec/basichullfunctionlist.hpp"
#include "game/spec/hull.hpp"

using game::spec::ModifiedHullFunctionList;

namespace {
    const game::spec::HullFunction* findEntry(const game::spec::HullFunctionList& list, int fcn)
    {
        for (game::spec::HullFunctionList::Iterator_t it = list.begin(); it != list.end(); ++it) {
            if (it->getBasicFunctionId() == fcn) {
                return &*it;
            }
        }
        return 0;
    }
}


/** Accessor tests. */
AFL_TEST("game.spec.HullFunctionAssignmentList:basics", a)
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
            a.checkNonNull("01. getEntryByIndex", p);
            if (p->m_function == 42) {
                found42 = true;
            }
            if (p->m_function == 77) {
                found77 = true;
            }
        }
        a.check("02. found 42", found42);
        a.check("03. found 77", found77);
    }

    // Verify lookup
    a.checkNonNull("11. findEntry", testee.findEntry(ModifiedHullFunctionList::Function_t(42)));
    a.checkNonNull("12. findEntry", testee.findEntry(ModifiedHullFunctionList::Function_t(77)));
    a.checkNull("13. findEntry", testee.findEntry(ModifiedHullFunctionList::Function_t(99)));

    a.checkNull("21. getEntryByIndex", testee.getEntryByIndex(testee.getNumEntries()));

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
            a.checkEqual("31. getKind", result[i].getKind(), game::spec::HullFunction::AssignedToHull);
            switch (result[i].getBasicFunctionId()) {
             case 42: found42 = true; break;
             case 77: found77 = true; break;
             case game::spec::BasicHullFunction::FullWeaponry: foundFullWeaponry = true; break;
            }
        }

        switch (player) {
         case 1:
            // Player 1 has all functions (explicitly set and implicitly given)
            a.check("41. found42", found42);
            a.check("42. found77", found77);
            a.check("43. foundFullWeaponry", foundFullWeaponry);
            break;
         case 2:
            // Player 2 has no functions
            a.check("44. found42", !found42);
            a.check("45. found77", !found77);
            a.check("46. foundFullWeaponry", !foundFullWeaponry);
            break;
         case 3:
            // Player 3 has function 77 (given to all but 2)
            a.check("47. found42", !found42);
            a.check("48. found77", found77);
            a.check("49. foundFullWeaponry", !foundFullWeaponry);
            break;
        }

        // Query the player set
        game::spec::BasicHullFunctionList basicList;
        a.checkEqual("51. getPlayersThatCan", testee.getPlayersThatCan(42, modList, basicList, config, hull, game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS), true), game::PlayerSet_t(1));
    }
}

/** Test getPlayersThatCan with an implied function. */
AFL_TEST("game.spec.HullFunctionAssignmentList:getPlayersThatCan:implied", a)
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
    a.checkEqual("01", testee.getPlayersThatCan(44, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));

    // Remove Tow for feds
    testee.change(game::spec::BasicHullFunction::Tow, game::PlayerSet_t(), game::PlayerSet_t(1));
    a.checkEqual("11", testee.getPlayersThatCan(44, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 1);

    // Change hull so that implied-tow no longer applies
    config[config.AllowOneEngineTowing].set(false);
    hull.setNumEngines(1);
    a.checkEqual("21", testee.getPlayersThatCan(44, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t());
}

/** Test behaviour of merged implied function. */
AFL_TEST("game.spec.HullFunctionAssignmentList:getPlayersThatCan:implied-merged", a)
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
    a.checkEqual("01", testee.getPlayersThatCan(fn, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 3 + 4 + 10);
}

/** Test all defaulted functions. */
AFL_TEST("game.spec.HullFunctionAssignmentList:getPlayersThatCan:defaulted", a)
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
    a.checkEqual("01", testee.getPlayersThatCan(BasicHullFunction::Tow,               modList, basicList, config, hull, allLevels, true), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS));
    a.checkEqual("02", testee.getPlayersThatCan(BasicHullFunction::Boarding,          modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 5 + 7);
    a.checkEqual("03", testee.getPlayersThatCan(BasicHullFunction::AntiCloakImmunity, modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 1 + 2 + 3);
    a.checkEqual("04", testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity,    modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 4 + 10);
    a.checkEqual("05", testee.getPlayersThatCan(BasicHullFunction::FullWeaponry,      modList, basicList, config, hull, allLevels, true), game::PlayerSet_t() + 1);

    a.checkEqual("11", testee.getPlayersThatCan(BasicHullFunction::Tow,               modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    a.checkEqual("12", testee.getPlayersThatCan(BasicHullFunction::Boarding,          modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    a.checkEqual("13", testee.getPlayersThatCan(BasicHullFunction::AntiCloakImmunity, modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    a.checkEqual("14", testee.getPlayersThatCan(BasicHullFunction::PlanetImmunity,    modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
    a.checkEqual("15", testee.getPlayersThatCan(BasicHullFunction::FullWeaponry,      modList, basicList, config, hull, allLevels, false), game::PlayerSet_t());
}

/** Test remove(). */
AFL_TEST("game.spec.HullFunctionAssignmentList:removeEntry", a)
{
    game::spec::HullFunctionAssignmentList testee;

    // Add something
    testee.change(ModifiedHullFunctionList::Function_t(100), game::PlayerSet_t(1), game::PlayerSet_t());
    testee.change(ModifiedHullFunctionList::Function_t(101), game::PlayerSet_t(2), game::PlayerSet_t());

    size_t n = testee.getNumEntries();
    a.checkGreaterEqual("01. getNumEntries", n, 2U);

    // Remove
    testee.removeEntry(ModifiedHullFunctionList::Function_t(100));
    size_t n2 = testee.getNumEntries();
    a.checkEqual("11. getNumEntries", n, n2+1);
    a.checkGreaterEqual("12. getNumEntries", n2, 1U);

    // Remove same again [no change]
    testee.removeEntry(ModifiedHullFunctionList::Function_t(100));
    a.checkEqual("21. getNumEntries", testee.getNumEntries(), n2);

    // Out-of-range access
    a.checkNull("31. getEntryByIndex", testee.getEntryByIndex(n2));
    a.checkNull("32. getEntryByIndex", testee.getEntryByIndex(n));

    a.checkNonNull("41. getEntryByIndex", testee.getEntryByIndex(n2-1));
}

/** Test sequence of add/remove.
    change() is defined as add-then-remove.
    That is, if a player is contained in add and remove, it ultimately ends in remove. */
AFL_TEST("game.spec.HullFunctionAssignmentList:add+remove", a)
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
    a.checkNonNull("01. findEntry", p);
    a.checkEqual("02. m_addedPlayers", p->m_addedPlayers, game::PlayerSet_t(1));
    a.checkEqual("03. m_removedPlayers", p->m_removedPlayers, game::PlayerSet_t(2) + 3);

    p = testee.findEntry(ModifiedHullFunctionList::Function_t(BasicHullFunction::Tow));
    a.checkNonNull("11. findEntry", p);
    a.checkEqual("12. m_addedPlayers", p->m_addedPlayers, game::PlayerSet_t(1));
    a.checkEqual("13. m_removedPlayers", p->m_removedPlayers, game::PlayerSet_t(2) + 3);

    p = testee.findEntry(ModifiedHullFunctionList::Function_t(100));
    a.checkNonNull("21. findEntry", p);
    a.checkEqual("22. m_addedPlayers", p->m_addedPlayers, game::PlayerSet_t(1));
    a.checkEqual("23. m_removedPlayers", p->m_removedPlayers, game::PlayerSet_t(2) + 3);

    p = testee.findEntry(ModifiedHullFunctionList::Function_t(101));
    a.checkNonNull("31. findEntry", p);
    a.checkEqual("32. m_addedPlayers", p->m_addedPlayers, game::PlayerSet_t(1));
    a.checkEqual("33. m_removedPlayers", p->m_removedPlayers, game::PlayerSet_t(2) + 3);
}

AFL_TEST("game.spec.HullFunctionAssignmentList:filter", a)
{
    // Add some functions
    game::spec::HullFunctionAssignmentList testee;
    testee.change(ModifiedHullFunctionList::Function_t(100), game::PlayerSet_t::allUpTo(20), game::PlayerSet_t());
    testee.change(ModifiedHullFunctionList::Function_t(101), game::PlayerSet_t(5), game::PlayerSet_t());
    testee.change(ModifiedHullFunctionList::Function_t(102), game::PlayerSet_t(7), game::PlayerSet_t());

    // Query, limited to one player
    game::spec::HullFunctionList out;
    game::spec::ModifiedHullFunctionList definitions;
    game::config::HostConfiguration config;
    game::spec::Hull hull(33);
    testee.getAll(out, definitions, config, hull, game::PlayerSet_t(7), game::ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS), game::spec::HullFunction::AssignedToHull);

    // Validate
    const game::spec::HullFunction* p = findEntry(out, ModifiedHullFunctionList::Function_t(100));
    a.checkNonNull("01. findEntry", p);
    a.checkEqual("02. getPlayers", p->getPlayers(), game::PlayerSet_t::allUpTo(20));

    p = findEntry(out, ModifiedHullFunctionList::Function_t(101));
    a.checkNull("11. findEntry", p);

    p = findEntry(out, ModifiedHullFunctionList::Function_t(102));
    a.checkNonNull("21. findEntry", p);
    a.checkEqual("22. getPlayers", p->getPlayers(), game::PlayerSet_t(7));
}
