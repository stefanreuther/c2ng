/**
  *  \file test/game/spec/info/infotest.cpp
  *  \brief Test for game::spec::info::Info
  */

#include "game/spec/info/info.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

namespace gsi = game::spec::info;
using game::Id_t;
using game::config::HostConfiguration;
using game::spec::Cost;

namespace {
    struct TestHarness {
        game::spec::ShipList shipList;
        afl::base::Ref<game::Root> root;
        afl::string::NullTranslator tx;
        gsi::NullPictureNamer picNamer;

        TestHarness()
            : shipList(),
              root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)))),
              tx(),
              picNamer()
            { }
    };

    /* Disable all host config options that would assign automatic hull functions. */
    void disableAutomaticHullFunctions(TestHarness& h)
    {
        HostConfiguration& c = h.root->hostConfiguration();

        // To be able to disable automatic Tow ability
        c[HostConfiguration::AllowOneEngineTowing].set(0);

        // Disable Boarding
        c[HostConfiguration::AllowPrivateerTowCapture].set(0);
        c[HostConfiguration::AllowCrystalTowCapture].set(0);

        // Disable AntiCloakImmunity
        c[HostConfiguration::AntiCloakImmunity].set(0);

        // Disable PlanetImmunity
        c[HostConfiguration::PlanetsAttackKlingons].set(1);
        c[HostConfiguration::PlanetsAttackRebels].set(1);

        // Disable FullWeaponry
        c[HostConfiguration::AllowFedCombatBonus].set(0);
    }

    /* Create a hull. */
    game::spec::Hull& makeHull(TestHarness& h, Id_t id)
    {
        game::spec::Hull& hull = *h.shipList.hulls().create(id);
        hull.setName("BR4 CLASS GUNSHIP");
        hull.setExternalPictureNumber(74);
        hull.setInternalPictureNumber(74);
        hull.cost().set(Cost::Tritanium, 17);
        hull.cost().set(Cost::Duranium, 12);
        hull.cost().set(Cost::Molybdenum, 35);
        hull.cost().set(Cost::Money, 60);
        hull.setMaxFuel(80);
        hull.setMaxCrew(55);
        hull.setNumEngines(1);    // set to 1 so we don't get automatic Tow
        hull.setMass(55);
        hull.setTechLevel(1);
        hull.setMaxCargo(20);
        hull.setNumBays(0);
        hull.setMaxLaunchers(0);
        hull.setMaxBeams(5);

        // Hull functions. Give it at least a cloaking device
        const int FUNC_ID = 3;
        game::spec::BasicHullFunction* f = h.shipList.basicHullFunctions().addFunction(FUNC_ID, "Cloak");
        f->setDescription("Cloaking Device");
        hull.changeHullFunction(h.shipList.modifiedHullFunctions().getFunctionIdFromHostId(FUNC_ID), game::PlayerSet_t::allUpTo(12), game::PlayerSet_t(), true);

        return hull;
    }

    /* Create an engine */
    game::spec::Engine& makeEngine(TestHarness& h, Id_t id)
    {
        game::spec::Engine& e = *h.shipList.engines().create(id);
        e.setName("HeavyNova Drive 6");
        e.cost().set(Cost::Tritanium, 3);
        e.cost().set(Cost::Duranium, 3);
        e.cost().set(Cost::Molybdenum, 15);
        e.cost().set(Cost::Money, 53);
        e.setTechLevel(6);
        e.setFuelFactor(1, 100);
        e.setFuelFactor(2, 415);
        e.setFuelFactor(3, 940);
        e.setFuelFactor(4, 1700);
        e.setFuelFactor(5, 260);
        e.setFuelFactor(6, 3733);
        e.setFuelFactor(7, 12300);
        e.setFuelFactor(8, 21450);
        e.setFuelFactor(9, 72900);
        return e;
    }

    /* Create a beam */
    game::spec::Beam& makeBeam(TestHarness& h, Id_t id)
    {
        game::spec::Beam& b = *h.shipList.beams().create(id);
        b.setName("Blaster");
        b.cost().set(Cost::Tritanium, 1);
        b.cost().set(Cost::Duranium, 12);
        b.cost().set(Cost::Molybdenum, 1);
        b.cost().set(Cost::Money, 10);
        b.setMass(4);
        b.setTechLevel(3);
        b.setKillPower(10);
        b.setDamagePower(25);
        return b;
    }

    /* Create a torpedo launcher */
    game::spec::TorpedoLauncher& makeLauncher(TestHarness& h, Id_t id)
    {
        game::spec::TorpedoLauncher& tl = *h.shipList.launchers().create(id);
        tl.setName("Mark 7 Photon");
        tl.cost().set(Cost::Tritanium, 1);
        tl.cost().set(Cost::Duranium, 3);
        tl.cost().set(Cost::Molybdenum, 8);
        tl.cost().set(Cost::Money, 120);
        tl.setMass(3);
        tl.setTechLevel(8);
        tl.setKillPower(25);
        tl.setDamagePower(48);
        tl.torpedoCost().set(Cost::Tritanium, 1);
        tl.torpedoCost().set(Cost::Duranium, 1);
        tl.torpedoCost().set(Cost::Molybdenum, 1);
        tl.torpedoCost().set(Cost::Money, 36);
        return tl;
    }

    /* Convert Attributes_t to string */
    String_t toString(const gsi::Attributes_t& atts)
    {
        String_t result;
        for (size_t i = 0; i < atts.size(); ++i) {
            result += atts[i].name;
            result += ':';
            result += atts[i].value;
            result += '\n';
        }
        return result;
    }

    /* Convert Abilities_t to string */
    String_t toString(const gsi::Abilities_t& abs)
    {
        String_t result;
        for (size_t i = 0; i < abs.size(); ++i) {
            result += abs[i].info;
            result += '\n';
        }
        return result;
    }
}

/** Test describeHull().
    This is mainly a regression test for ports. */
AFL_TEST("game.spec.info.Info:describeHull", a)
{
    const Id_t HULL_NR = 44;
    TestHarness h;
    makeHull(h, HULL_NR);
    disableAutomaticHullFunctions(h);
    h.shipList.hullAssignments().add(2, 3, HULL_NR);
    h.shipList.hullAssignments().add(5, 9, HULL_NR);

    gsi::PageContent c;
    gsi::describeHull(c, HULL_NR, h.shipList, true, h.picNamer, *h.root, 2, h.tx);

    a.checkEqual("01. title", c.title, "BR4 CLASS GUNSHIP");
    a.checkEqual("02. pictureName", c.pictureName, "");                // would be set by PictureNamer
    a.checkEqual("03. attributes", toString(c.attributes),
                 "Mass:55 kt\n"
                 "Cargo:20 kt\n"
                 "Fuel:80 kt\n"
                 "Engines:1\n"
                 "Crew:55\n"
                 "Weapons:5 beams\n"
                 "Mine hit damage:181%\n"
                 "Cost:60 mc, 17 T, 12 D, 35 M\n"
                 "Tech level:1\n");
    a.checkEqual("04. pageLinks", c.pageLinks, gsi::Pages_t());
    a.checkEqual("05. abilities", toString(c.abilities),
                 "Cloaking Device\n");
    a.checkEqual("06. players", c.players, game::PlayerSet_t() + 2 + 5);
}

/** Test describeEngine().
    This is mainly a regression test for ports. */
AFL_TEST("game.spec.info.Info:describeEngine", a)
{
    const Id_t ENGINE_NR = 6;
    TestHarness h;
    makeEngine(h, ENGINE_NR);

    gsi::PageContent c;
    gsi::describeEngine(c, ENGINE_NR, h.shipList, true, h.picNamer, *h.root, 2, h.tx);

    a.checkEqual("01. title", c.title, "HeavyNova Drive 6");
    a.checkEqual("02. pictureName", c.pictureName, "");                // would be set by PictureNamer
    a.checkEqual("03. attributes", toString(c.attributes),
                 "Max Efficient Warp:6\n"
                 "Cost:53 mc, 3 T/D, 15 M\n"
                 "Tech level:6\n");
    a.checkEqual("04. pageLinks", c.pageLinks, gsi::Pages_t());
    a.checkEqual("05. abilities", c.abilities.size(), 0U);
    a.checkEqual("06. players", c.players, game::PlayerSet_t());
}

/** Test describeBeam().
    This is mainly a regression test for ports. */
AFL_TEST("game.spec.info.Info:describeBeam", a)
{
    const Id_t BEAM_NR = 4;
    TestHarness h;
    makeBeam(h, BEAM_NR);

    gsi::PageContent c;
    gsi::describeBeam(c, BEAM_NR, h.shipList, true, h.picNamer, *h.root, 2, h.tx);

    a.checkEqual("01. title", c.title, "Blaster");
    a.checkEqual("02. pictureName", c.pictureName, "");                // would be set by PictureNamer
    a.checkEqual("03. attributes", toString(c.attributes),
                 "Type:normal\n"
                 "Kill:10\n"
                 "Destroy:25\n"
                 "Recharge time:150s\n"
                 "Hit:100%\n"
                 "Sweep:64 mines, 48 webs\n"
                 "Mass:4 kt\n"
                 "Cost:10 mc, 1 T/M, 12 D\n"
                 "Tech level:3\n");
    a.checkEqual("04. pageLinks", c.pageLinks, gsi::Pages_t());
    a.checkEqual("05. abilities", c.abilities.size(), 0U);
    a.checkEqual("06. players", c.players, game::PlayerSet_t());
}

/** Test describeTorpedo().
    This is mainly a regression test for ports. */
AFL_TEST("game.spec.info.Info:describeTorpedo", a)
{
    const Id_t LAUNCHER_NR = 9;
    TestHarness h;
    makeLauncher(h, LAUNCHER_NR);

    gsi::PageContent c;
    gsi::describeTorpedo(c, LAUNCHER_NR, h.shipList, true, h.picNamer, *h.root, 2, h.tx);

    a.checkEqual("01. title", c.title, "Mark 7 Photon");
    a.checkEqual("02. pictureName", c.pictureName, "");                // would be set by PictureNamer
    a.checkEqual("03. attributes", toString(c.attributes),
                 "Type:normal\n"
                 "Kill:50\n"
                 "Destroy:96\n"
                 "Recharge time:44s\n"
                 "Hit:65%\n"
                 "Torp Cost:36 mc, 1 T/D/M\n"
                 "1000 mines:444 mc, 12 T/D/M\n"
                 "Launcher Mass:3 kt\n"
                 "Launcher Cost:120 mc, 1 T, 3 D, 8 M\n"
                 "Tech level:8\n");
    a.checkEqual("04. pageLinks", c.pageLinks, gsi::Pages_t());
    a.checkEqual("05. abilities", c.abilities.size(), 0U);
    a.checkEqual("06. players", c.players, game::PlayerSet_t());
}

/** Test describeFighter().
    This is mainly a regression test for ports. */
AFL_TEST("game.spec.info.Info:describeFighter", a)
{
    TestHarness h;

    gsi::PageContent c;
    gsi::describeFighter(c, 7, h.shipList, true, h.picNamer, *h.root, h.tx);

    a.checkEqual("01. title", c.title, "Player 7 fighter");
    a.checkEqual("02. pictureName", c.pictureName, "");                // would be set by PictureNamer
    a.checkEqual("03. attributes", toString(c.attributes),
                 "Type:fighter\n"
                 "Kill:2\n"
                 "Destroy:2\n"
                 "Recharge time:21\xE2\x80\x93""36s\n"
                 "Strikes:7\n"
                 "Fighter Cost:100 mc, 3 T, 2 M\n");
    a.checkEqual("04. pageLinks", c.pageLinks, gsi::Pages_t());
    a.checkEqual("05. abilities", c.abilities.size(), 0U);
    a.checkEqual("06. players", c.players, game::PlayerSet_t());
}

/** Test describeFighter() for Empire. */
AFL_TEST("game.spec.info.Info:describeFighter:8", a)
{
    TestHarness h;

    gsi::PageContent c;
    gsi::describeFighter(c, 8, h.shipList, true, h.picNamer, *h.root, h.tx);

    a.checkEqual("01. title", c.title, "Player 8 fighter");
    a.checkEqual("02. pictureName", c.pictureName, "");                // would be set by PictureNamer
    a.checkEqual("03. attributes", toString(c.attributes),
                 "Type:fighter\n"
                 "Kill:2\n"
                 "Destroy:2\n"
                 "Recharge time:21\xE2\x80\x93""36s\n"
                 "Strikes:7\n"
                 "Fighter Cost:100 mc, 3 T, 2 M\n"
                 "Auto-build:10 per turn for 3 T, 2 M each\n");
    a.checkEqual("04. pageLinks", c.pageLinks, gsi::Pages_t());
    a.checkEqual("05. abilities", c.abilities.size(), 0U);
    a.checkEqual("06. players", c.players, game::PlayerSet_t());
}

/** Test getHullAttribute(). */
AFL_TEST("game.spec.info.Info:getHullAttribute", a)
{
    const Id_t HULL_NR = 120;
    TestHarness h;
    game::spec::Hull& hull = makeHull(h, HULL_NR);

    a.checkEqual("01. Range_CostD",        getHullAttribute(hull, gsi::Range_CostD).orElse(-1),        12);
    a.checkEqual("02. Range_CostM",        getHullAttribute(hull, gsi::Range_CostM).orElse(-1),        35);
    a.checkEqual("03. Range_CostMC",       getHullAttribute(hull, gsi::Range_CostMC).orElse(-1),       60);
    a.checkEqual("04. Range_CostT",        getHullAttribute(hull, gsi::Range_CostT).orElse(-1),        17);
    a.checkEqual("05. Range_IsArmed",      getHullAttribute(hull, gsi::Range_IsArmed).orElse(-1),      1);
    a.checkEqual("06. Range_Mass",         getHullAttribute(hull, gsi::Range_Mass).orElse(-1),         55);
    a.checkEqual("07. Range_MaxBeams",     getHullAttribute(hull, gsi::Range_MaxBeams).orElse(-1),     5);
    a.checkEqual("08. Range_MaxCargo",     getHullAttribute(hull, gsi::Range_MaxCargo).orElse(-1),     20);
    a.checkEqual("09. Range_MaxCrew",      getHullAttribute(hull, gsi::Range_MaxCrew).orElse(-1),      55);
    a.checkEqual("10. Range_MaxFuel",      getHullAttribute(hull, gsi::Range_MaxFuel).orElse(-1),      80);
    a.checkEqual("11. Range_MaxLaunchers", getHullAttribute(hull, gsi::Range_MaxLaunchers).orElse(-1), 0);
    a.checkEqual("12. Range_NumBays",      getHullAttribute(hull, gsi::Range_NumBays).orElse(-1),      0);
    a.checkEqual("13. Range_NumEngines",   getHullAttribute(hull, gsi::Range_NumEngines).orElse(-1),   1);
    a.checkEqual("14. Range_Id",           getHullAttribute(hull, gsi::Range_Id).orElse(-1),           HULL_NR);
    a.checkEqual("15. Range_Tech",         getHullAttribute(hull, gsi::Range_Tech).orElse(-1),         1);

    a.checkEqual("21. Range_DamagePower",  getHullAttribute(hull, gsi::Range_DamagePower).isValid(), false);
}

/** Test getEngineAttribute(). */
AFL_TEST("game.spec.info.Info:getEngineAttribute", a)
{
    const Id_t ENGINE_NR = 4;
    TestHarness h;
    game::spec::Engine& e = makeEngine(h, ENGINE_NR);

    a.checkEqual("01. Range_CostD",            getEngineAttribute(e, gsi::Range_CostD).orElse(-1),        3);
    a.checkEqual("02. Range_CostM",            getEngineAttribute(e, gsi::Range_CostM).orElse(-1),        15);
    a.checkEqual("03. Range_CostMC",           getEngineAttribute(e, gsi::Range_CostMC).orElse(-1),       53);
    a.checkEqual("04. Range_CostT",            getEngineAttribute(e, gsi::Range_CostT).orElse(-1),        3);
    a.checkEqual("05. Range_MaxEfficientWarp", getEngineAttribute(e, gsi::Range_MaxEfficientWarp).orElse(-1), 6);
    a.checkEqual("06. Range_Id",               getEngineAttribute(e, gsi::Range_Id).orElse(-1),           ENGINE_NR);
    a.checkEqual("07. Range_Tech",             getEngineAttribute(e, gsi::Range_Tech).orElse(-1),         6);

    a.checkEqual("11. Range_DamagePower",      getEngineAttribute(e, gsi::Range_DamagePower).isValid(), false);
}

/** Test getBeamAttribute(). */
AFL_TEST("game.spec.info.Info:getBeamAttribute", a)
{
    const Id_t BEAM_NR = 2;
    const int VIEWPOINT = 4;
    TestHarness h;
    game::spec::Beam& b = makeBeam(h, BEAM_NR);

    a.checkEqual("01. Range_CostD",        getBeamAttribute(b, gsi::Range_CostD,        *h.root, VIEWPOINT).orElse(-1), 12);
    a.checkEqual("02. Range_CostM",        getBeamAttribute(b, gsi::Range_CostM,        *h.root, VIEWPOINT).orElse(-1), 1);
    a.checkEqual("03. Range_CostMC",       getBeamAttribute(b, gsi::Range_CostMC,       *h.root, VIEWPOINT).orElse(-1), 10);
    a.checkEqual("04. Range_CostT",        getBeamAttribute(b, gsi::Range_CostT,        *h.root, VIEWPOINT).orElse(-1), 1);
    a.checkEqual("05. Range_DamagePower",  getBeamAttribute(b, gsi::Range_DamagePower,  *h.root, VIEWPOINT).orElse(-1), 25);
    a.checkEqual("06. Range_HitOdds",      getBeamAttribute(b, gsi::Range_HitOdds,      *h.root, VIEWPOINT).orElse(-1), 100);
    a.checkEqual("07. Range_KillPower",    getBeamAttribute(b, gsi::Range_KillPower,    *h.root, VIEWPOINT).orElse(-1), 10);
    a.checkEqual("08. Range_Mass",         getBeamAttribute(b, gsi::Range_Mass,         *h.root, VIEWPOINT).orElse(-1), 4);
    a.checkEqual("09. Range_RechargeTime", getBeamAttribute(b, gsi::Range_RechargeTime, *h.root, VIEWPOINT).orElse(-1), 150);
    a.checkEqual("10. Range_Id",           getBeamAttribute(b, gsi::Range_Id,           *h.root, VIEWPOINT).orElse(-1), BEAM_NR);
    a.checkEqual("11. Range_IsDeathRay",   getBeamAttribute(b, gsi::Range_IsDeathRay,   *h.root, VIEWPOINT).orElse(-1), 0);
    a.checkEqual("12. Range_Tech",         getBeamAttribute(b, gsi::Range_Tech,         *h.root, VIEWPOINT).orElse(-1), 3);

    a.checkEqual("21. Range_MaxCrew",      getBeamAttribute(b, gsi::Range_MaxCrew,      *h.root, VIEWPOINT).isValid(), false);
}

/** Test getTorpedoAttribute(). */
AFL_TEST("game.spec.info.Info:getTorpedoAttribute", a)
{
    const Id_t LAUNCHER_NR = 10;
    const int VIEWPOINT = 4;
    TestHarness h;
    game::spec::TorpedoLauncher& tl = makeLauncher(h, LAUNCHER_NR);

    a.checkEqual("01. Range_CostD",        getTorpedoAttribute(tl, gsi::Range_CostD,        *h.root, VIEWPOINT).orElse(-1), 3);
    a.checkEqual("02. Range_CostM",        getTorpedoAttribute(tl, gsi::Range_CostM,        *h.root, VIEWPOINT).orElse(-1), 8);
    a.checkEqual("03. Range_CostMC",       getTorpedoAttribute(tl, gsi::Range_CostMC,       *h.root, VIEWPOINT).orElse(-1), 120);
    a.checkEqual("04. Range_CostT",        getTorpedoAttribute(tl, gsi::Range_CostT,        *h.root, VIEWPOINT).orElse(-1), 1);
    a.checkEqual("05. Range_DamagePower",  getTorpedoAttribute(tl, gsi::Range_DamagePower,  *h.root, VIEWPOINT).orElse(-1), 96);
    a.checkEqual("06. Range_HitOdds",      getTorpedoAttribute(tl, gsi::Range_HitOdds,      *h.root, VIEWPOINT).orElse(-1), 65);
    a.checkEqual("07. Range_KillPower",    getTorpedoAttribute(tl, gsi::Range_KillPower,    *h.root, VIEWPOINT).orElse(-1), 50);
    a.checkEqual("08. Range_Mass",         getTorpedoAttribute(tl, gsi::Range_Mass,         *h.root, VIEWPOINT).orElse(-1), 3);
    a.checkEqual("09. Range_RechargeTime", getTorpedoAttribute(tl, gsi::Range_RechargeTime, *h.root, VIEWPOINT).orElse(-1), 44);
    a.checkEqual("10. Range_Id",           getTorpedoAttribute(tl, gsi::Range_Id,           *h.root, VIEWPOINT).orElse(-1), LAUNCHER_NR);
    a.checkEqual("11. Range_IsDeathRay",   getTorpedoAttribute(tl, gsi::Range_IsDeathRay,   *h.root, VIEWPOINT).orElse(-1), 0);
    a.checkEqual("12. Range_Tech",         getTorpedoAttribute(tl, gsi::Range_Tech,         *h.root, VIEWPOINT).orElse(-1), 8);
    a.checkEqual("13. Range_TorpCost",     getTorpedoAttribute(tl, gsi::Range_TorpCost,     *h.root, VIEWPOINT).orElse(-1), 36);

    a.checkEqual("21. Range_MaxCrew",      getTorpedoAttribute(tl, gsi::Range_MaxCrew,      *h.root, VIEWPOINT).isValid(), false);
}

/** Test getFighterAttribute(). */
AFL_TEST("game.spec.info.Info:getFighterAttribute", a)
{
    TestHarness h;
    game::spec::Fighter ftr(3, h.root->hostConfiguration(), h.root->playerList(), h.tx);

    a.checkEqual("01. Range_CostD",        getFighterAttribute(ftr, gsi::Range_CostD,        *h.root).orElse(-1), 0);
    a.checkEqual("02. Range_CostM",        getFighterAttribute(ftr, gsi::Range_CostM,        *h.root).orElse(-1), 2);
    a.checkEqual("03. Range_CostMC",       getFighterAttribute(ftr, gsi::Range_CostMC,       *h.root).orElse(-1), 100);
    a.checkEqual("04. Range_CostT",        getFighterAttribute(ftr, gsi::Range_CostT,        *h.root).orElse(-1), 3);
    a.checkEqual("05. Range_DamagePower",  getFighterAttribute(ftr, gsi::Range_DamagePower,  *h.root).orElse(-1), 2);
    a.checkEqual("06. Range_KillPower",    getFighterAttribute(ftr, gsi::Range_KillPower,    *h.root).orElse(-1), 2);
    a.checkEqual("07. Range_RechargeTime", getFighterAttribute(ftr, gsi::Range_RechargeTime, *h.root).orElse(-1), 21);

    a.checkEqual("11. Range_MaxCrew",      getFighterAttribute(ftr, gsi::Range_MaxCrew,      *h.root).isValid(), false);
}

/** Test describeWeaponEffects(), Tim-Host version. */
AFL_TEST("game.spec.info.Info:describeWeaponEffects:host", a)
{
    // Environment
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,0))));
    afl::string::NullTranslator tx;

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(330, 50);
    q.setCrew(348);
    q.setOwner(11);

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, *root, tx);

    // Verify
    a.checkEqual("01. effectScale",    result.effectScale, 1);
    a.checkEqual("02. mass",           result.mass,        330);
    a.checkEqual("03. usedESBRate",    result.usedESBRate, 50);
    a.checkEqual("04. crew",           result.crew,        348);
    a.checkEqual("05. damageLimit",    result.damageLimit, 100);
    a.checkEqual("06. player",         result.player,      11);

    a.checkEqual("11. beamEffects",    result.beamEffects.size(), 10U);
    a.checkEqual("12. name",           result.beamEffects[0].name, "Laser");
    a.checkEqual("13. shieldEffect",   result.beamEffects[0].shieldEffect, 2);
    a.checkEqual("14. damageEffect",   result.beamEffects[0].damageEffect, 1);
    a.checkEqual("15. crewEffect",     result.beamEffects[0].crewEffect, 2);
    a.checkEqual("16. name",           result.beamEffects[9].name, "Heavy Phaser");
    a.checkEqual("17. shieldEffect",   result.beamEffects[9].shieldEffect, 12);
    a.checkEqual("18. damageEffect",   result.beamEffects[9].damageEffect, 4);
    a.checkEqual("19. crewEffect",     result.beamEffects[9].crewEffect, 8);

    a.checkEqual("21. torpedoEffects", result.torpedoEffects.size(), 10U);
    a.checkEqual("22. name",           result.torpedoEffects[0].name, "Mark 1 Photon");
    a.checkEqual("23. shieldEffect",   result.torpedoEffects[0].shieldEffect, 3);
    a.checkEqual("24. damageEffect",   result.torpedoEffects[0].damageEffect, 2);
    a.checkEqual("25. crewEffect",     result.torpedoEffects[0].crewEffect, 2);
    a.checkEqual("26. name",           result.torpedoEffects[9].name, "Mark 8 Photon");
    a.checkEqual("27. shieldEffect",   result.torpedoEffects[9].shieldEffect, 28);
    a.checkEqual("28. damageEffect",   result.torpedoEffects[9].damageEffect, 8);
    a.checkEqual("29. crewEffect",     result.torpedoEffects[9].crewEffect, 17);

    a.checkEqual("31. fighterEffects", result.fighterEffects.size(), 1U);
    a.checkEqual("32. name",           result.fighterEffects[0].name, "Fighter");
    a.checkEqual("33. shieldEffect",   result.fighterEffects[0].shieldEffect, 1);
    a.checkEqual("34. damageEffect",   result.fighterEffects[0].damageEffect, 1);
    a.checkEqual("35. crewEffect",     result.fighterEffects[0].crewEffect, 0);
}

/** Test describeWeaponEffects(), PHost Alternative Combat version. */
AFL_TEST("game.spec.info.Info:describeWeaponEffects:phost:alternative-combat", a)
{
    // Environment
    // (Pleiades 13, player 7, turn 74, ship 72)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root->hostConfiguration();
    config[HostConfiguration::AllowAlternativeCombat].set(1);
    config[HostConfiguration::CrewKillScaling].set(15);
    config[HostConfiguration::ShieldKillScaling].set(0);
    config[HostConfiguration::ShieldDamageScaling].set(40);
    config[HostConfiguration::HullDamageScaling].set(20);
    config[HostConfiguration::FighterBeamExplosive].set(9);
    config[HostConfiguration::FighterBeamKill].set(9);
    config[HostConfiguration::EModCrewKillScaling].set("-6,-9,-12,-15");
    config[HostConfiguration::EModHullDamageScaling].set("0");
    config[HostConfiguration::EModShieldDamageScaling].set("0");
    config[HostConfiguration::EModShieldKillScaling].set("0");

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(207, 23);
    q.setCrew(257);
    q.setOwner(7);

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, *root, tx);

    // Verify
    a.checkDifferent("01. effectScale", result.effectScale, 1);
    a.checkEqual("02. mass",            result.mass,        207);
    a.checkEqual("03. usedESBRate",     result.usedESBRate, 23);
    a.checkEqual("04. crew",            result.crew,        257);
    a.checkEqual("05. damageLimit",     result.damageLimit, 100);
    a.checkEqual("06. player",          result.player,      7);
    const double scale = 1.0 / result.effectScale;

    a.checkEqual("11. beamEffects",    result.beamEffects.size(), 10U);
    a.checkEqual("12. name",           result.beamEffects[0].name, "Laser Cannon");
    a.checkNear ("13. shieldEffect",   result.beamEffects[0].shieldEffect * scale, 1.35, 0.01);
    a.checkNear ("14. damageEffect",   result.beamEffects[0].damageEffect * scale, 0.67, 0.01);
    a.checkNear ("15. crewEffect",     result.beamEffects[0].crewEffect   * scale, 0.07, 0.01);
    a.checkEqual("16. name",           result.beamEffects[1].name, "Kill-O-Zap");
    a.checkEqual("17. shieldEffect",   result.beamEffects[1].shieldEffect, 0);
    a.checkEqual("18. damageEffect",   result.beamEffects[1].damageEffect, 0);
    a.checkNear ("19. crewEffect",     result.beamEffects[1].crewEffect   * scale, 1.08, 0.01);
    a.checkEqual("20. name",           result.beamEffects[9].name, "Multitraf Spiral");
    a.checkNear ("21. shieldEffect",   result.beamEffects[9].shieldEffect * scale, 15.38, 0.01);
    a.checkNear ("22. damageEffect",   result.beamEffects[9].damageEffect * scale,  7.69, 0.01);
    a.checkNear ("23. crewEffect",     result.beamEffects[9].crewEffect   * scale,  2.88, 0.01);

    a.checkEqual("31. torpedoEffects", result.torpedoEffects.size(), 10U);
    a.checkEqual("32. name",           result.torpedoEffects[0].name, "Space Rocket");
    a.checkNear ("33. shieldEffect",   result.torpedoEffects[0].shieldEffect * scale, 5.77, 0.01);
    a.checkNear ("34. damageEffect",   result.torpedoEffects[0].damageEffect * scale, 2.88, 0.01);
    a.checkNear ("35. crewEffect",     result.torpedoEffects[0].crewEffect   * scale, 0.36, 0.01);
    a.checkEqual("36. name",           result.torpedoEffects[1].name, "Paralyso-Matic Bomb");
    a.checkEqual("37. shieldEffect",   result.torpedoEffects[1].shieldEffect, 0);
    a.checkEqual("38. damageEffect",   result.torpedoEffects[1].damageEffect, 0);
    a.checkNear ("39. crewEffect",     result.torpedoEffects[1].crewEffect   * scale, 1.80, 0.01);
    a.checkEqual("40. name",           result.torpedoEffects[9].name, "Selphyr-Fataro-Dev.");
    a.checkNear ("41. shieldEffect",   result.torpedoEffects[9].shieldEffect * scale, 19.04, 0.01);
    a.checkNear ("42. damageEffect",   result.torpedoEffects[9].damageEffect * scale,  9.52, 0.01);
    a.checkNear ("43. crewEffect",     result.torpedoEffects[9].crewEffect   * scale,  2.88, 0.01);

    a.checkEqual("51. fighterEffects", result.fighterEffects.size(), 1U);
    a.checkEqual("52. name",           result.fighterEffects[0].name, "Fighter");
    a.checkNear ("53. shieldEffect",   result.fighterEffects[0].shieldEffect * scale, 1.73, 0.01);
    a.checkNear ("54. damageEffect",   result.fighterEffects[0].damageEffect * scale, 0.87, 0.01);
    a.checkNear ("55. crewEffect",     result.fighterEffects[0].crewEffect   * scale, 0.65, 0.01);
}

/** Test describeWeaponEffects(), PHost Non-Alternative-Combat version.
    This is the same as above, but with AC turned off; validated against PCC2. */
AFL_TEST("game.spec.info.Info:describeWeaponEffects:phost:standard-combat", a)
{
    // Environment
    // (Pleiades 13, player 7, turn 74, ship 72)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root->hostConfiguration();
    config[HostConfiguration::AllowAlternativeCombat].set(0);   // off!
    config[HostConfiguration::CrewKillScaling].set(15);
    config[HostConfiguration::ShieldKillScaling].set(0);
    config[HostConfiguration::ShieldDamageScaling].set(40);
    config[HostConfiguration::HullDamageScaling].set(20);
    config[HostConfiguration::FighterBeamExplosive].set(9);
    config[HostConfiguration::FighterBeamKill].set(9);
    config[HostConfiguration::EModCrewKillScaling].set("-6,-9,-12,-15");
    config[HostConfiguration::EModHullDamageScaling].set("0");
    config[HostConfiguration::EModShieldDamageScaling].set("0");
    config[HostConfiguration::EModShieldKillScaling].set("0");

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(207, 23);
    q.setCrew(257);
    q.setOwner(7);

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, *root, tx);

    // Verify
    a.checkEqual("01. effectScale", result.effectScale, 1);
    a.checkEqual("02. mass",        result.mass,        207);
    a.checkEqual("03. usedESBRate", result.usedESBRate, 23);
    a.checkEqual("04. crew",        result.crew,        257);
    a.checkEqual("05. damageLimit", result.damageLimit, 100);
    a.checkEqual("06. player",      result.player,      7);

    a.checkEqual("11. beamEffects",    result.beamEffects.size(), 10U);
    a.checkEqual("12. name",           result.beamEffects[0].name, "Laser Cannon");
    a.checkEqual("13. shieldEffect",   result.beamEffects[0].shieldEffect, 2);
    a.checkEqual("14. damageEffect",   result.beamEffects[0].damageEffect, 0);
    a.checkEqual("15. crewEffect",     result.beamEffects[0].crewEffect,   0);
    a.checkEqual("16. name",           result.beamEffects[1].name, "Kill-O-Zap");
    a.checkEqual("17. shieldEffect",   result.beamEffects[1].shieldEffect, 0);
    a.checkEqual("18. damageEffect",   result.beamEffects[1].damageEffect, 0);
    a.checkEqual("19. crewEffect",     result.beamEffects[1].crewEffect,   1);
    a.checkEqual("20. name",           result.beamEffects[9].name, "Multitraf Spiral");
    a.checkEqual("21. shieldEffect",   result.beamEffects[9].shieldEffect, 16);
    a.checkEqual("22. damageEffect",   result.beamEffects[9].damageEffect,  2);
    a.checkEqual("23. crewEffect",     result.beamEffects[9].crewEffect,    3);

    a.checkEqual("31. torpedoEffects", result.torpedoEffects.size(), 10U);
    a.checkEqual("32. name",           result.torpedoEffects[0].name, "Space Rocket");
    a.checkEqual("33. shieldEffect",   result.torpedoEffects[0].shieldEffect, 13);
    a.checkEqual("34. damageEffect",   result.torpedoEffects[0].damageEffect, 1);
    a.checkEqual("35. crewEffect",     result.torpedoEffects[0].crewEffect,   1);
    a.checkEqual("36. name",           result.torpedoEffects[1].name, "Paralyso-Matic Bomb");
    a.checkEqual("37. shieldEffect",   result.torpedoEffects[1].shieldEffect, 0);
    a.checkEqual("38. damageEffect",   result.torpedoEffects[1].damageEffect, 0);
    a.checkEqual("39. crewEffect",     result.torpedoEffects[1].crewEffect,   4);
    a.checkEqual("40. name",           result.torpedoEffects[9].name, "Selphyr-Fataro-Dev.");
    a.checkEqual("41. shieldEffect",   result.torpedoEffects[9].shieldEffect, 39);
    a.checkEqual("42. damageEffect",   result.torpedoEffects[9].damageEffect,  4);
    a.checkEqual("43. crewEffect",     result.torpedoEffects[9].crewEffect,    6);

    a.checkEqual("51. fighterEffects", result.fighterEffects.size(), 1U);
    a.checkEqual("52. name",           result.fighterEffects[0].name, "Fighter");
    a.checkEqual("53. shieldEffect",   result.fighterEffects[0].shieldEffect, 3);
    a.checkEqual("54. damageEffect",   result.fighterEffects[0].damageEffect, 0);
    a.checkEqual("55. crewEffect",     result.fighterEffects[0].crewEffect,   1);
}

/** Test describeWeaponEffects(), mixed fighter behaviour. */
AFL_TEST("game.spec.info.Info:describeWeaponEffects:phost:mixed-fighters", a)
{
    // Environment
    // (Pleiades 13, player 7, turn 74, ship 72)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root->hostConfiguration();
    config[HostConfiguration::AllowAlternativeCombat].set(1);
    config[HostConfiguration::CrewKillScaling].set(15);
    config[HostConfiguration::ShieldKillScaling].set(0);
    config[HostConfiguration::ShieldDamageScaling].set(40);
    config[HostConfiguration::HullDamageScaling].set(20);
    config[HostConfiguration::FighterBeamExplosive].set("9,9,9,10,9,9,10,9,10,6,9");
    config[HostConfiguration::FighterBeamKill].set("9,12,9,10,9,9,13,9,8,6,9");

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(207, 23);
    q.setCrew(257);
    q.setOwner(7);

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, *root, tx);

    // Verify
    a.checkDifferent("01. effectScale", result.effectScale, 1);
    a.checkEqual("02. mass",            result.mass,        207);
    a.checkEqual("03. usedESBRate",     result.usedESBRate, 23);
    a.checkEqual("04. crew",            result.crew,        257);
    a.checkEqual("05. damageLimit",     result.damageLimit, 100);
    a.checkEqual("06. player",          result.player,      7);
    const double scale = 1.0 / result.effectScale;

    // FighterBeamExplosive = 9,  9, 9, 10, 9, 9, 10, 9, 10, 6, 9
    // FighterBeamKill      = 9, 12, 9, 10, 9, 9, 13, 9,  8, 6, 9
    // -> Fed (9/9)           x      x      x  x      x         x
    // -> Liz (9/12)              x
    // -> Kli (10/10)                   x
    // -> Tho (10/13) (not listed!)                x
    // -> Rob (10/8)                                      x
    // -> Reb (6/6)                                          x
    a.checkEqual("11. fighterEffects", result.fighterEffects.size(), 5U);
    a.checkEqual("12. name",           result.fighterEffects[0].name, "Player 1 Fighter");
    a.checkNear ("13. shieldEffect",   result.fighterEffects[0].shieldEffect * scale, 1.73, 0.01);
    a.checkNear ("14. damageEffect",   result.fighterEffects[0].damageEffect * scale, 0.87, 0.01);
    a.checkNear ("15. crewEffect",     result.fighterEffects[0].crewEffect   * scale, 0.65, 0.01);
    a.checkEqual("16. name",           result.fighterEffects[1].name, "Player 2 Fighter");
    a.checkNear ("17. shieldEffect",   result.fighterEffects[1].shieldEffect * scale, 1.73, 0.01);
    a.checkNear ("18. damageEffect",   result.fighterEffects[1].damageEffect * scale, 0.87, 0.01);
    a.checkNear ("19. crewEffect",     result.fighterEffects[1].crewEffect   * scale, 0.87, 0.01);
    a.checkEqual("20. name",           result.fighterEffects[2].name, "Player 4 Fighter");
    a.checkNear ("21. shieldEffect",   result.fighterEffects[2].shieldEffect * scale, 1.92, 0.01);
    a.checkNear ("22. damageEffect",   result.fighterEffects[2].damageEffect * scale, 0.96, 0.01);
    a.checkNear ("23. crewEffect",     result.fighterEffects[2].crewEffect   * scale, 0.72, 0.01);
    a.checkEqual("24. name",           result.fighterEffects[3].name, "Player 9 Fighter");
    a.checkNear ("25. shieldEffect",   result.fighterEffects[3].shieldEffect * scale, 1.92, 0.01);
    a.checkNear ("26. damageEffect",   result.fighterEffects[3].damageEffect * scale, 0.96, 0.01);
    a.checkNear ("27. crewEffect",     result.fighterEffects[3].crewEffect   * scale, 0.58, 0.01);
    a.checkEqual("28. name",           result.fighterEffects[4].name, "Player 10 Fighter");
    a.checkNear ("29. shieldEffect",   result.fighterEffects[4].shieldEffect * scale, 1.15, 0.01);
    a.checkNear ("30. damageEffect",   result.fighterEffects[4].damageEffect * scale, 0.58, 0.01);
    a.checkNear ("31. crewEffect",     result.fighterEffects[4].crewEffect   * scale, 0.43, 0.01);
}

/** Test describeWeaponEffects(), experience behaviour. */
AFL_TEST("game.spec.info.Info:describeWeaponEffects:phost:alternative-combat:experience", a)
{
    // Environment (similar as testDescribeWeaponEffectsPHostAC)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root->hostConfiguration();
    config[HostConfiguration::AllowAlternativeCombat].set(1);
    config[HostConfiguration::CrewKillScaling].set(15);
    config[HostConfiguration::ShieldKillScaling].set(0);
    config[HostConfiguration::ShieldDamageScaling].set(40);
    config[HostConfiguration::HullDamageScaling].set(20);
    config[HostConfiguration::FighterBeamExplosive].set(9);
    config[HostConfiguration::FighterBeamKill].set(9);
    config[HostConfiguration::EModCrewKillScaling].set("-6,-9,-12,-15");
    config[HostConfiguration::EModHullDamageScaling].set("0");
    config[HostConfiguration::EModShieldDamageScaling].set("0");
    config[HostConfiguration::EModShieldKillScaling].set("0");

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(207, 23);
    q.setCrew(257);
    q.setOwner(7);
    q.setLevelDisplaySet(game::ExperienceLevelSet_t(3));

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, *root, tx);

    // Verify specimen
    const double scale = 1.0 /       result.effectScale;
    a.checkEqual("01. beamEffects",  result.beamEffects.size(), 10U);
    a.checkEqual("02. name",         result.beamEffects[9].name, "Multitraf Spiral");
    a.checkNear ("03. shieldEffect", result.beamEffects[9].shieldEffect * scale, 15.38, 0.01);
    a.checkNear ("04. damageEffect", result.beamEffects[9].damageEffect * scale,  7.69, 0.01);
    a.checkNear ("05. crewEffect",   result.beamEffects[9].crewEffect   * scale,  0.58, 0.01);  // CrewKillScaling reduced from 15 -> 3 (=factor 5), effect also reduced by factor 5
}

/** Test describeWeaponEffects(), experience behaviour, non-AC. */
AFL_TEST("game.spec.info.Info:describeWeaponEffects:phost:standard-combat:experience", a)
{
    // Environment (similar as testDescribeWeaponEffectsPHostNonAC)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root->hostConfiguration();
    config[HostConfiguration::AllowAlternativeCombat].set(0);   // off!
    config[HostConfiguration::CrewKillScaling].set(15);
    config[HostConfiguration::ShieldKillScaling].set(0);
    config[HostConfiguration::ShieldDamageScaling].set(40);
    config[HostConfiguration::HullDamageScaling].set(20);
    config[HostConfiguration::FighterBeamExplosive].set(9);
    config[HostConfiguration::FighterBeamKill].set(9);
    config[HostConfiguration::EModCrewKillScaling].set("-6,-9,-12,-15");
    config[HostConfiguration::EModHullDamageScaling].set("0");
    config[HostConfiguration::EModShieldDamageScaling].set("0");
    config[HostConfiguration::EModShieldKillScaling].set("0");

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(207, 23);
    q.setCrew(257);
    q.setOwner(7);
    q.setLevelDisplaySet(game::ExperienceLevelSet_t(3));

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, *root, tx);

    // Verify specimen
    a.checkEqual("01. effectScale",  result.effectScale, 1);
    a.checkEqual("02. beamEffects",  result.beamEffects.size(), 10U);
    a.checkEqual("03. name",         result.beamEffects[9].name, "Multitraf Spiral");
    a.checkEqual("04. shieldEffect", result.beamEffects[9].shieldEffect, 16);
    a.checkEqual("05. damageEffect", result.beamEffects[9].damageEffect,  2);
    a.checkEqual("06. crewEffect",   result.beamEffects[9].crewEffect,    1);
}

AFL_TEST("game.spec.info.Info:describeHullFunctions", a)
{
    using game::spec::BasicHullFunction;
    using game::spec::HullFunction;
    using game::ExperienceLevelSet_t;
    using game::PlayerSet_t;

    // Environment
    TestHarness h;
    game::spec::BasicHullFunctionList& b = h.shipList.basicHullFunctions();
    BasicHullFunction* fCloak = b.addFunction(16, "Cloak");
    fCloak->setDescription("cloaking device");
    fCloak->setExplanation("it cloaks");
    BasicHullFunction* fBoarding = b.addFunction(31, "Boarding");
    fBoarding->setDescription("tow-capture");
    fBoarding->setExplanation("it boards!");
    for (int i = 1; i <= 10; ++i) {
        h.root->playerList().create(i);
    }

    h.root->hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(5);
    h.root->hostConfiguration()[game::config::HostConfiguration::DamageLevelForCloakFail].set(10);

    // HullFunctionList
    game::spec::HullFunctionList hfList;
    HullFunction a1(16, ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS));
    a1.setPlayers(PlayerSet_t() + 5);
    a1.setKind(HullFunction::AssignedToHull);
    hfList.add(a1);
    HullFunction a2(31, ExperienceLevelSet_t() + 3);
    a2.setKind(HullFunction::AssignedToShip);
    hfList.add(a2);

    // describeHullFunctions()
    {
        gsi::Abilities_t out;
        describeHullFunctions(out, hfList, 0, h.shipList, h.picNamer, *h.root, h.tx);
        a.checkEqual("01. size", out.size(), 2U);
        a.checkEqual("02. info", out[0].info, "cloaking device (player 5)");
        a.checkEqual("03. info", out[1].info, "tow-capture (level 3; ship)");

        a.check("11. DamagedAbility", !out[0].flags.contains(game::spec::info::DamagedAbility));
        a.check("12. ForeignAbility", !out[0].flags.contains(game::spec::info::ForeignAbility));
        a.check("13. ReachableAbility", !out[0].flags.contains(game::spec::info::ReachableAbility));
        a.check("14. OutgrownAbility", !out[0].flags.contains(game::spec::info::OutgrownAbility));
    }

    // describeHullFunctions() with query
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::Abilities_t out;
        describeHullFunctions(out, hfList, &q, h.shipList, h.picNamer, *h.root, h.tx);
        a.checkEqual("21. size", out.size(), 2U);
        a.checkEqual("22. info", out[0].info, "cloaking device (player 5; damaged)");
        a.checkEqual("23. info", out[1].info, "tow-capture (level 3; ship)");

        a.check("31. DamagedAbility",    out[0].flags.contains(game::spec::info::DamagedAbility));
        a.check("32. ForeignAbility",    out[0].flags.contains(game::spec::info::ForeignAbility));
        a.check("33. ReachableAbility", !out[0].flags.contains(game::spec::info::ReachableAbility));
        a.check("34. OutgrownAbility",  !out[0].flags.contains(game::spec::info::OutgrownAbility));
    }

    // describeHullFunctionDetails()
    {
        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, 0, h.shipList, h.picNamer, false, *h.root, h.tx);
        a.checkEqual("41. size", out.size(), 2U);
        a.checkEqual("42. name",        out[0].name, "Cloak");
        a.checkEqual("43. description", out[0].description, "cloaking device");
        a.checkEqual("44. explanation", out[0].explanation, "it cloaks");
        // damageLimit not known (but might be someday)
        a.checkEqual("45. playerLimit", out[0].playerLimit, "player 5");
        a.checkEqual("46. levelLimit",  out[0].levelLimit, "");
        a.checkEqual("47. kind",        out[0].kind, game::spec::info::ClassAbility);

        a.checkEqual("51. name",        out[1].name, "Boarding");
        a.checkEqual("52. description", out[1].description, "tow-capture");
        a.checkEqual("53. explanation", out[1].explanation, "it boards!");
        a.checkEqual("54. damageLimit", out[1].damageLimit.isValid(), false);
        a.checkEqual("55. playerLimit", out[1].playerLimit, "");
        a.checkEqual("56. levelLimit",  out[1].levelLimit, "level 3");
        a.checkEqual("57. kind",        out[1].kind, game::spec::info::ShipAbility);
    }

    // describeHullFunctionDetails() with query
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, &q, h.shipList, h.picNamer, false, *h.root, h.tx);
        a.checkEqual("61. size", out.size(), 2U);
        a.checkEqual("62. name",              out[0].name, "Cloak");
        a.checkEqual("63. description",       out[0].description, "cloaking device");
        a.checkEqual("64. explanation",       out[0].explanation, "it cloaks");
        a.checkEqual("65. damageLimit",       out[0].damageLimit.orElse(-1), 10);
        a.checkEqual("66. playerLimit",       out[0].playerLimit, "player 5");
        a.checkEqual("67. levelLimit",        out[0].levelLimit, "");
        a.checkEqual("68. kind",              out[0].kind, game::spec::info::ClassAbility);
        a.check     ("69. DamagedAbility",    out[0].flags.contains(game::spec::info::DamagedAbility));
        a.checkEqual("70. minimumExperience", out[0].minimumExperience, 0);

        a.checkEqual("71. name",              out[1].name, "Boarding");
        a.checkEqual("72. description",       out[1].description, "tow-capture");
        a.checkEqual("73. explanation",       out[1].explanation, "it boards!");
        a.checkEqual("74. damageLimit",       out[1].damageLimit.isValid(), false);
        a.checkEqual("75. playerLimit",       out[1].playerLimit, "");
        a.checkEqual("76. levelLimit",        out[1].levelLimit, "level 3");
        a.checkEqual("77. kind",              out[1].kind, game::spec::info::ShipAbility);
        a.checkEqual("78. minimumExperience", out[1].minimumExperience, 3000);
    }
}

AFL_TEST("game.spec.info.Info:describeHullFunctions:picture", a)
{
    using game::spec::BasicHullFunction;
    using game::spec::HullFunction;
    using game::ExperienceLevelSet_t;
    using game::PlayerSet_t;

    // Environment
    TestHarness h;
    game::spec::BasicHullFunctionList& b = h.shipList.basicHullFunctions();
    b.addFunction(16, "Cloak")
        ->setPictureName("cloaker");
    for (int i = 1; i <= 10; ++i) {
        h.root->playerList().create(i);
    }
    h.root->hostConfiguration()[game::config::HostConfiguration::DamageLevelForCloakFail].set(10);

    // HullFunctionList
    game::spec::HullFunctionList hfList;
    HullFunction a1(16, ExperienceLevelSet_t::allUpTo(game::MAX_EXPERIENCE_LEVELS));
    a1.setPlayers(PlayerSet_t() + 5);
    a1.setKind(HullFunction::AssignedToHull);
    hfList.add(a1);

    // PictureNamer for testing
    class TestPicNamer : public game::spec::info::PictureNamer {
     public:
        virtual String_t getHullPicture(const game::spec::Hull& /*h*/) const
            { return String_t(); }
        virtual String_t getEnginePicture(const game::spec::Engine& /*e*/) const
            { return String_t(); }
        virtual String_t getBeamPicture(const game::spec::Beam& /*b*/) const
            { return String_t(); }
        virtual String_t getLauncherPicture(const game::spec::TorpedoLauncher& /*tl*/) const
            { return String_t(); }
        virtual String_t getAbilityPicture(const String_t& abilityName, game::spec::info::AbilityFlags_t flags) const
            {
                String_t result;
                if (flags.contains(game::spec::info::DamagedAbility)) {
                    result += "broken-";
                } else {
                    result += "good-";
                }
                result += abilityName;
                return result;
            }
        virtual String_t getPlayerPicture(const game::Player& /*pl*/) const
            { return String_t(); }
        virtual String_t getFighterPicture(int /*raceNr*/, int /*playerNr*/) const
            { return String_t(); }
        virtual String_t getVcrObjectPicture(bool /*isPlanet*/, int /*pictureNumber*/) const
            { return String_t(); }
    };
    TestPicNamer picNamer;

    // useNormalPictures=false
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, &q, h.shipList, picNamer, false, *h.root, h.tx);
        a.checkEqual("01. size", out.size(), 1U);
        a.checkEqual("02. name",              out[0].name, "Cloak");
        a.checkEqual("03. kind",              out[0].kind, game::spec::info::ClassAbility);
        a.check     ("04. DamagedAbility",    out[0].flags.contains(game::spec::info::DamagedAbility));
        a.checkEqual("05. pictureName",       out[0].pictureName, "broken-cloaker");
        a.checkEqual("06. minimumExperience", out[0].minimumExperience, 0);
    }

    // useNormalPictures=true
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, &q, h.shipList, picNamer, true, *h.root, h.tx);
        a.checkEqual("11. size", out.size(), 1U);
        a.checkEqual("12. name",              out[0].name, "Cloak");
        a.checkEqual("13. kind",              out[0].kind, game::spec::info::ClassAbility);
        a.check     ("14. DamagedAbility",    out[0].flags.contains(game::spec::info::DamagedAbility));
        a.checkEqual("15. pictureName",       out[0].pictureName, "good-cloaker");
        a.checkEqual("16. minimumExperience", out[0].minimumExperience, 0);
    }
}
