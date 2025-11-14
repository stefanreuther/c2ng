/**
  *  \file test/game/sim/shiptest.cpp
  *  \brief Test for game::sim::Ship
  */

#include "game/sim/ship.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/configuration.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"

#include "objecttest.hpp"

/** Test getter/setter. */
AFL_TEST("game.sim.Ship:basics", a)
{
    game::sim::Ship t;
    game::spec::ShipList sl;

    // Initial state
    a.checkEqual("01. getCrew",           t.getCrew(), 10);
    a.checkEqual("02. getHullType",       t.getHullType(), 0);
    a.checkEqual("03. getMass",           t.getMass(), 100);
    a.checkEqual("04. getBeamType",       t.getBeamType(), 0);
    a.checkEqual("05. getNumBeams",       t.getNumBeams(), 0);
    a.checkEqual("06. getTorpedoType",    t.getTorpedoType(), 0);
    a.checkEqual("07. getNumLaunchers",   t.getNumLaunchers(), 0);
    a.checkEqual("08. getNumBays",        t.getNumBays(), 0);
    a.checkEqual("09. getAmmo",           t.getAmmo(), 0);
    a.checkEqual("10. getEngineType",     t.getEngineType(), 1);
    a.checkEqual("11. getAggressiveness", t.getAggressiveness(), game::sim::Ship::agg_Passive);
    a.checkEqual("12. getInterceptId",    t.getInterceptId(), 0);

    a.check     ("21. isCustomShip",      t.isCustomShip());
    a.checkEqual("22. getNumBeamsRange",  t.getNumBeamsRange(sl).min(), 0);
    a.check     ("23. getNumBeamsRange", 20 <= t.getNumBeamsRange(sl).max());
    a.checkEqual("24. getNumLaunchersRange", t.getNumLaunchersRange(sl).min(), 0);
    a.check     ("25. getNumLaunchersRange", 20 <= t.getNumLaunchersRange(sl).max());
    a.checkEqual("24. getNumBaysRange",   t.getNumBaysRange(sl).min(), 0);
    a.check     ("27. getNumBaysRange",   20 <= t.getNumBaysRange(sl).max());

    // Set/get
    t.markClean();
    t.setCrew(42);
    a.checkEqual("31. getCrew", t.getCrew(), 42);
    a.check("32. isDirty", t.isDirty());

    t.markClean();
    t.setHullTypeOnly(33);
    a.checkEqual("41. getHullType", t.getHullType(), 33);
    a.check("42. isDirty", t.isDirty());
    a.check("43. isCustomShip", !t.isCustomShip());

    t.markClean();
    t.setMass(130);
    a.checkEqual("51. getMass", t.getMass(), 130);
    a.check("52. isDirty", t.isDirty());

    t.markClean();
    t.setBeamType(3);
    a.checkEqual("61. getBeamType", t.getBeamType(), 3);
    a.check("62. isDirty", t.isDirty());

    t.markClean();
    t.setNumBeams(9);
    a.checkEqual("71. getNumBeams", t.getNumBeams(), 9);
    a.check("72. isDirty", t.isDirty());

    t.markClean();                             // repeated -> no change signal
    t.setNumBeams(9);
    a.checkEqual("81. getNumBeams", t.getNumBeams(), 9);
    a.check("82. isDirty", !t.isDirty());

    t.markClean();
    t.setTorpedoType(4);
    a.checkEqual("91. getTorpedoType", t.getTorpedoType(), 4);
    a.check("92. isDirty", t.isDirty());

    t.markClean();
    t.setNumLaunchers(8);
    a.checkEqual("101. getNumLaunchers", t.getNumLaunchers(), 8);
    a.check("102. isDirty", t.isDirty());

    t.markClean();
    t.setNumBays(12);
    a.checkEqual("111. getNumBays", t.getNumBays(), 12);
    a.check("112. isDirty", t.isDirty());

    t.markClean();
    t.setAmmo(80);
    a.checkEqual("121. getAmmo", t.getAmmo(), 80);
    a.check("122. isDirty", t.isDirty());

    t.markClean();
    t.setEngineType(9);
    a.checkEqual("131. getEngineType", t.getEngineType(), 9);
    a.check("132. isDirty", t.isDirty());

    t.markClean();
    t.setAggressiveness(7);
    a.checkEqual("141. getAggressiveness", t.getAggressiveness(), 7);
    a.check("142. isDirty", t.isDirty());

    t.markClean();
    t.setInterceptId(815);
    a.checkEqual("151. getInterceptId", t.getInterceptId(), 815);
    a.check("152. isDirty", t.isDirty());

    verifyObject(a, t);
}

/** Test name functions. */
AFL_TEST("game.sim.Ship:name", a)
{
    afl::string::NullTranslator tx;
    game::sim::Ship t;
    t.setId(77);

    t.setDefaultName(tx);
    a.check("01", t.hasDefaultName(tx));

    t.setId(42);
    a.check("11", !t.hasDefaultName(tx));

    t.setDefaultName(tx);
    a.check("21", t.hasDefaultName(tx));
}

/** Test hull type / ship list interaction. */
AFL_TEST("game.sim.Ship:shiplist", a)
{
    // Make a ship list
    game::spec::ShipList list;
    {
        game::spec::Hull* h = list.hulls().create(1);
        h->setMaxFuel(100);
        h->setMaxCrew(50);
        h->setNumEngines(2);
        h->setMaxCargo(80);
        h->setNumBays(5);
        h->setMaxLaunchers(0);
        h->setMaxBeams(15);
        h->setMass(2000);
    }
    {
        game::spec::Hull* h = list.hulls().create(2);
        h->setMaxFuel(200);
        h->setMaxCrew(75);
        h->setNumEngines(3);
        h->setMaxCargo(120);
        h->setNumBays(0);
        h->setMaxLaunchers(10);
        h->setMaxBeams(5);
        h->setMass(3000);
    }
    for (int i = 1; i <= 5; ++i) {
        list.beams().create(i);
    }
    for (int i = 1; i <= 7; ++i) {
        list.launchers().create(i);
    }
    for (int i = 1; i <= 7; ++i) {
        list.engines().create(i)->cost().set(game::spec::Cost::Money, 100*i);
    }

    // Test
    game::sim::Ship testee;
    testee.setHullType(2, list);
    a.checkEqual("01. getHullType",          testee.getHullType(), 2);
    a.checkEqual("02. getAmmo",              testee.getAmmo(), 120);
    a.checkEqual("03. getNumBays",           testee.getNumBays(), 0);
    a.checkEqual("04. getNumLaunchers",      testee.getNumLaunchers(), 10);
    a.checkEqual("05. getNumBeams",          testee.getNumBeams(), 5);
    a.checkEqual("06. getTorpedoType",       testee.getTorpedoType(), 7);
    a.checkEqual("07. getBeamType",          testee.getBeamType(), 5);
    a.checkEqual("08. getMass",              testee.getMass(), 3000);
    a.check     ("09. isMatchingShipList",   testee.isMatchingShipList(list));
    a.checkEqual("10. getNumBeamsRange",     testee.getNumBeamsRange(list).min(), 0);
    a.checkEqual("11. getNumBeamsRange",     testee.getNumBeamsRange(list).max(), 5);
    a.checkEqual("12. getNumLaunchersRange", testee.getNumLaunchersRange(list).min(), 0);
    a.checkEqual("13. getNumLaunchersRange", testee.getNumLaunchersRange(list).max(), 10);
    a.checkEqual("14. getNumBaysRange",      testee.getNumBaysRange(list).min(), 0);
    a.checkEqual("15. getNumBaysRange",      testee.getNumBaysRange(list).max(), 0);
    a.checkEqual("16. getEngineType",        testee.getEngineType(), 1);

    // Derived attributes
    {
        afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
        game::config::HostConfiguration& config = *rconfig;
        game::vcr::flak::Configuration flakConfiguration;
        game::sim::Configuration opts;
        opts.setEngineShieldBonus(20);

        a.checkEqual("21. getEffectiveMass",           testee.getEffectiveMass(opts, list, config), 3020);                               // +20 from ESB
        a.checkEqual("22. getDefaultFlakRating",       testee.getDefaultFlakRating(flakConfiguration, opts, list, config), 3115);
        a.checkEqual("23. getDefaultFlakCompensation", testee.getDefaultFlakCompensation(flakConfiguration, opts, list, config), 500);   // actually, 1050, but limited by CompensationLimit

        // Alternative FLAK configuration
        flakConfiguration.RatingMassScale = 0;
        flakConfiguration.CompensationLimit = 9999;
        flakConfiguration.CompensationMass100KTScale = 30;
        a.checkEqual("31. getDefaultFlakRating",       testee.getDefaultFlakRating(flakConfiguration, opts, list, config), 95);
        a.checkEqual("32. getDefaultFlakCompensation", testee.getDefaultFlakCompensation(flakConfiguration, opts, list, config), 1956);  // +906 from CompensationMass100KTScale

        // Alternative sim configuration
        opts.setEngineShieldBonus(0);
        a.checkEqual("41. getDefaultFlakRating",       testee.getDefaultFlakRating(flakConfiguration, opts, list, config), 95);
        a.checkEqual("42. getDefaultFlakCompensation", testee.getDefaultFlakCompensation(flakConfiguration, opts, list, config), 1950);  // -20*0.3 from ESB
    }

    // Vary attributes
    testee.setNumBeams(3);
    a.check("51. isMatchingShipList", testee.isMatchingShipList(list));
    testee.setNumBeams(6);
    a.check("52. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setNumBeams(5);
    a.check("53. isMatchingShipList", testee.isMatchingShipList(list));

    testee.setNumLaunchers(9);
    a.check("61. isMatchingShipList", testee.isMatchingShipList(list));
    testee.setNumLaunchers(11);
    a.check("62. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setNumLaunchers(10);
    a.check("63. isMatchingShipList", testee.isMatchingShipList(list));

    testee.setAmmo(1);
    a.check("71. isMatchingShipList", testee.isMatchingShipList(list));
    testee.setAmmo(121);
    a.check("72. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setAmmo(120);
    a.check("73. isMatchingShipList", testee.isMatchingShipList(list));

    testee.setTorpedoType(0);
    testee.setNumLaunchers(0);
    testee.setNumBays(1);
    a.check("81. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setNumBays(0);
    a.check("82. isMatchingShipList", testee.isMatchingShipList(list));

    // Change hull type to other existing hull
    testee.setHullType(1, list);
    a.checkEqual("91. getHullType",           testee.getHullType(), 1);
    a.checkEqual("92. getAmmo",               testee.getAmmo(), 80);
    a.checkEqual("93. getNumBays",            testee.getNumBays(), 5);
    a.checkEqual("94. getNumLaunchers",       testee.getNumLaunchers(), 0);
    a.checkEqual("95. getNumBeams",           testee.getNumBeams(), 15);
    a.checkEqual("96. getTorpedoType",        testee.getTorpedoType(), 0);
    a.checkEqual("97. getBeamType",           testee.getBeamType(), 5);
    a.checkEqual("98. getMass",               testee.getMass(), 2000);
    a.check     ("99. isMatchingShipList",    testee.isMatchingShipList(list));
    a.checkEqual("100. getNumBeamsRange",     testee.getNumBeamsRange(list).min(), 0);
    a.checkEqual("101. getNumBeamsRange",     testee.getNumBeamsRange(list).max(), 15);
    a.checkEqual("102. getNumLaunchersRange", testee.getNumLaunchersRange(list).min(), 0);
    a.checkEqual("103. getNumLaunchersRange", testee.getNumLaunchersRange(list).max(), 0);
    a.checkEqual("104. getNumBaysRange",      testee.getNumBaysRange(list).min(), 5);
    a.checkEqual("105. getNumBaysRange",      testee.getNumBaysRange(list).max(), 5);

    // Vary attributes
    testee.setNumBays(10);
    a.check("111. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setNumBays(0);
    testee.setNumLaunchers(1);
    testee.setTorpedoType(1);
    a.check("112. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setNumBays(1);
    testee.setNumLaunchers(0);
    testee.setTorpedoType(0);
    a.check("113. isMatchingShipList", !testee.isMatchingShipList(list));
    testee.setNumBays(5);
    a.check("114. isMatchingShipList", testee.isMatchingShipList(list));

    // Change to nonexistant hull
    testee.setHullType(3, list);
    a.checkEqual("121. getHullType",          testee.getHullType(), 3);
    a.check     ("122. isMatchingShipList",  !testee.isMatchingShipList(list));
    a.checkEqual("123. getNumBeamsRange",     testee.getNumBeamsRange(list).min(), 0);
    a.checkEqual("124. getNumBeamsRange",     testee.getNumBeamsRange(list).max(), 0);
    a.checkEqual("125. getNumLaunchersRange", testee.getNumLaunchersRange(list).min(), 0);
    a.checkEqual("126. getNumLaunchersRange", testee.getNumLaunchersRange(list).max(), 0);
    a.checkEqual("127. getNumBaysRange",      testee.getNumBaysRange(list).min(), 0);
    a.checkEqual("128. getNumBaysRange",      testee.getNumBaysRange(list).max(), 0);

    // Change to custom ship
    testee.setHullType(0, list);
    a.checkEqual("131. getHullType", testee.getHullType(), 0);
    a.check("132. isMatchingShipList", testee.isMatchingShipList(list));
}

/** Test ship abilities. */
AFL_TEST("game.sim.Ship:abilities", a)
{
    // Make a ship list
    game::spec::ShipList list;
    {
        game::spec::Hull* h = list.hulls().create(1);
        h->changeHullFunction(game::spec::ModifiedHullFunctionList::Function_t(game::spec::BasicHullFunction::Commander),
                              game::PlayerSet_t(9),
                              game::PlayerSet_t(),
                              true);
    }

    // Configuration
    afl::base::Ref<game::config::HostConfiguration> rconfig = game::config::HostConfiguration::create();
    game::config::HostConfiguration& config = *rconfig;
    game::sim::Configuration opts;

    game::sim::Configuration nuOpts;
    nuOpts.setMode(game::sim::Configuration::VcrNuHost, 0, config);

    // Test
    game::sim::Ship testee;
    testee.setHullType(1, list);

    // Player 1: FullWeaponry
    testee.setOwner(1);
    a.check("01. hasAnyNonstandardAbility", !testee.hasAnyNonstandardAbility());
    a.check("02. FullWeaponryAbility",       testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    a.check("03. PlanetImmunityAbility",    !testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    a.check("04. TripleBeamKillAbility",    !testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    a.check("05. CommanderAbility",         !testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    a.check("06. ElusiveAbility",           !testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    a.checkEqual("11", testee.getAbilities(opts, list, config), game::sim::Abilities_t() + game::sim::FullWeaponryAbility);

    // Player 4: PlanetImmunity
                                             testee.setOwner(4);
    a.check("21. hasAnyNonstandardAbility", !testee.hasAnyNonstandardAbility());
    a.check("22. FullWeaponryAbility",      !testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    a.check("23. PlanetImmunityAbility",     testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    a.check("24. TripleBeamKillAbility",    !testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    a.check("25. CommanderAbility",         !testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    a.check("26. ElusiveAbility",           !testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    a.check("31. DoubleBeamChargeAbility",  !testee.hasAbility(game::sim::DoubleBeamChargeAbility, opts, list, config));
    a.check("32. DoubleBeamChargeAbility",   testee.hasAbility(game::sim::DoubleBeamChargeAbility, nuOpts, list, config));

    a.checkEqual("41. getAbilities", testee.getAbilities(opts,   list, config), game::sim::Abilities_t() + game::sim::PlanetImmunityAbility);
    a.checkEqual("42. getAbilities", testee.getAbilities(nuOpts, list, config), game::sim::Abilities_t() + game::sim::PlanetImmunityAbility + game::sim::DoubleBeamChargeAbility);

    // Player 5: TripleBeamKill
    testee.setOwner(5);
    a.check("51. hasAnyNonstandardAbility", !testee.hasAnyNonstandardAbility());
    a.check("52. FullWeaponryAbility",      !testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    a.check("53. PlanetImmunityAbility",    !testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    a.check("54. TripleBeamKillAbility",     testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    a.check("55. CommanderAbility",         !testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    a.check("56. ElusiveAbility",           !testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    a.checkEqual("61. getAbilities", testee.getAbilities(opts, list, config), game::sim::Abilities_t() + game::sim::TripleBeamKillAbility);

    // Player 9: Commander
    testee.setOwner(9);
    a.check("71. hasAnyNonstandardAbility", !testee.hasAnyNonstandardAbility());
    a.check("72. FullWeaponryAbility",      !testee.hasAbility(game::sim::FullWeaponryAbility,   opts, list, config));
    a.check("73. PlanetImmunityAbility",    !testee.hasAbility(game::sim::PlanetImmunityAbility, opts, list, config));
    a.check("74. TripleBeamKillAbility",    !testee.hasAbility(game::sim::TripleBeamKillAbility, opts, list, config));
    a.check("75. CommanderAbility",          testee.hasAbility(game::sim::CommanderAbility,      opts, list, config));
    a.check("76. ElusiveAbility",           !testee.hasAbility(game::sim::ElusiveAbility,        opts, list, config));

    a.checkEqual("81. getAbilities", testee.getAbilities(opts, list, config), game::sim::Abilities_t() + game::sim::CommanderAbility);
}

AFL_TEST("game.sim.Ship:isPrimaryEnemy", a)
{
    using game::sim::Ship;
    a.checkEqual("01", Ship::isPrimaryEnemy(0), false);
    a.checkEqual("02", Ship::isPrimaryEnemy(Ship::agg_Kill), false);
    a.checkEqual("03", Ship::isPrimaryEnemy(Ship::agg_NoFuel), false);
    a.checkEqual("04", Ship::isPrimaryEnemy(Ship::agg_Passive), false);

    a.checkEqual("11", Ship::isPrimaryEnemy(1), true);
    a.checkEqual("12", Ship::isPrimaryEnemy(11), true);
    a.checkEqual("13", Ship::isPrimaryEnemy(12), true);
}
