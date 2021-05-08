/**
  *  \file u/t_game_spec_info_info.cpp
  *  \brief Test for game::spec::info::Info
  */

#include "game/spec/info/info.hpp"

#include "t_game_spec_info.hpp"
#include "game/test/root.hpp"
#include "game/spec/info/nullpicturenamer.hpp"
#include "afl/string/nulltranslator.hpp"

namespace gsi = game::spec::info;
using game::Id_t;
using game::config::HostConfiguration;
using game::spec::Cost;

namespace {
    struct TestHarness {
        game::spec::ShipList shipList;
        game::test::Root root;
        afl::string::NullTranslator tx;
        gsi::NullPictureNamer picNamer;

        TestHarness()
            : shipList(),
              root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))),
              tx(),
              picNamer()
            { }
    };

    /* Disable all host config options that would assign automatic hull functions. */
    void disableAutomaticHullFunctions(TestHarness& h)
    {
        HostConfiguration& c = h.root.hostConfiguration();

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
void
TestGameSpecInfoInfo::testDescribeHull()
{
    const Id_t HULL_NR = 44;
    TestHarness h;
    makeHull(h, HULL_NR);
    disableAutomaticHullFunctions(h);
    h.shipList.hullAssignments().add(2, 3, HULL_NR);
    h.shipList.hullAssignments().add(5, 9, HULL_NR);

    gsi::PageContent c;
    gsi::describeHull(c, HULL_NR, h.shipList, true, h.picNamer, h.root, 2, h.tx);

    TS_ASSERT_EQUALS(c.title, "BR4 CLASS GUNSHIP");
    TS_ASSERT_EQUALS(c.pictureName, "");                // would be set by PictureNamer
    TS_ASSERT_EQUALS(toString(c.attributes),
                     "Mass:55 kt\n"
                     "Cargo:20 kt\n"
                     "Fuel:80 kt\n"
                     "Engines:1\n"
                     "Crew:55\n"
                     "Weapons:5 beams\n"
                     "Mine hit damage:181%\n"
                     "Cost:60 mc, 17 T, 12 D, 35 M\n"
                     "Tech level:1\n");
    TS_ASSERT_EQUALS(c.pageLinks, gsi::Pages_t());
    TS_ASSERT_EQUALS(toString(c.abilities),
                     "Cloaking Device\n");
    TS_ASSERT_EQUALS(c.players, game::PlayerSet_t() + 2 + 5);
}

/** Test describeEngine().
    This is mainly a regression test for ports. */
void
TestGameSpecInfoInfo::testDescribeEngine()
{
    const Id_t ENGINE_NR = 6;
    TestHarness h;
    makeEngine(h, ENGINE_NR);

    gsi::PageContent c;
    gsi::describeEngine(c, ENGINE_NR, h.shipList, true, h.picNamer, h.root, 2, h.tx);

    TS_ASSERT_EQUALS(c.title, "HeavyNova Drive 6");
    TS_ASSERT_EQUALS(c.pictureName, "");                // would be set by PictureNamer
    TS_ASSERT_EQUALS(toString(c.attributes),
                     "Max Efficient Warp:6\n"
                     "Cost:53 mc, 3 T/D, 15 M\n"
                     "Tech level:6\n");
    TS_ASSERT_EQUALS(c.pageLinks, gsi::Pages_t());
    TS_ASSERT_EQUALS(c.abilities.size(), 0U);
    TS_ASSERT_EQUALS(c.players, game::PlayerSet_t());
}

/** Test describeBeam().
    This is mainly a regression test for ports. */
void
TestGameSpecInfoInfo::testDescribeBeam()
{
    const Id_t BEAM_NR = 4;
    TestHarness h;
    makeBeam(h, BEAM_NR);

    gsi::PageContent c;
    gsi::describeBeam(c, BEAM_NR, h.shipList, true, h.picNamer, h.root, 2, h.tx);

    TS_ASSERT_EQUALS(c.title, "Blaster");
    TS_ASSERT_EQUALS(c.pictureName, "");                // would be set by PictureNamer
    TS_ASSERT_EQUALS(toString(c.attributes),
                     "Type:normal\n"
                     "Kill:10\n"
                     "Destroy:25\n"
                     "Recharge time:150s\n"
                     "Hit:100%\n"
                     "Sweep:64 mines, 48 webs\n"
                     "Mass:4 kt\n"
                     "Cost:10 mc, 1 T/M, 12 D\n"
                     "Tech level:3\n");
    TS_ASSERT_EQUALS(c.pageLinks, gsi::Pages_t());
    TS_ASSERT_EQUALS(c.abilities.size(), 0U);
    TS_ASSERT_EQUALS(c.players, game::PlayerSet_t());
}

/** Test describeTorpedo().
    This is mainly a regression test for ports. */
void
TestGameSpecInfoInfo::testDescribeTorp()
{
    const Id_t LAUNCHER_NR = 9;
    TestHarness h;
    makeLauncher(h, LAUNCHER_NR);

    gsi::PageContent c;
    gsi::describeTorpedo(c, LAUNCHER_NR, h.shipList, true, h.picNamer, h.root, 2, h.tx);

    TS_ASSERT_EQUALS(c.title, "Mark 7 Photon");
    TS_ASSERT_EQUALS(c.pictureName, "");                // would be set by PictureNamer
    TS_ASSERT_EQUALS(toString(c.attributes),
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
    TS_ASSERT_EQUALS(c.pageLinks, gsi::Pages_t());
    TS_ASSERT_EQUALS(c.abilities.size(), 0U);
    TS_ASSERT_EQUALS(c.players, game::PlayerSet_t());
}

/** Test describeFighter().
    This is mainly a regression test for ports. */
void
TestGameSpecInfoInfo::testDescribeFighter()
{
    TestHarness h;

    gsi::PageContent c;
    gsi::describeFighter(c, 7, h.shipList, true, h.picNamer, h.root, h.tx);

    TS_ASSERT_EQUALS(c.title, "Player 7 fighter");
    TS_ASSERT_EQUALS(c.pictureName, "");                // would be set by PictureNamer
    TS_ASSERT_EQUALS(toString(c.attributes),
                     "Type:fighter\n"
                     "Kill:2\n"
                     "Destroy:2\n"
                     "Recharge:21\xE2\x80\x93""36\n"
                     "Strikes:7\n"
                     "Fighter Cost:100 mc, 3 T, 2 M\n");
    TS_ASSERT_EQUALS(c.pageLinks, gsi::Pages_t());
    TS_ASSERT_EQUALS(c.abilities.size(), 0U);
    TS_ASSERT_EQUALS(c.players, game::PlayerSet_t());
}

/** Test getHullAttribute(). */
void
TestGameSpecInfoInfo::testGetHullAttribute()
{
    const Id_t HULL_NR = 120;
    TestHarness h;
    game::spec::Hull& hull = makeHull(h, HULL_NR);

    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_CostD).orElse(-1),        12);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_CostM).orElse(-1),        35);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_CostMC).orElse(-1),       60);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_CostT).orElse(-1),        17);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_IsArmed).orElse(-1),      1);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_Mass).orElse(-1),         55);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_MaxBeams).orElse(-1),     5);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_MaxCargo).orElse(-1),     20);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_MaxCrew).orElse(-1),      55);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_MaxFuel).orElse(-1),      80);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_MaxLaunchers).orElse(-1), 0);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_NumBays).orElse(-1),      0);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_NumEngines).orElse(-1),   1);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_Id).orElse(-1),           HULL_NR);
    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_Tech).orElse(-1),         1);

    TS_ASSERT_EQUALS(getHullAttribute(hull, gsi::Range_DamagePower).isValid(), false);
}

/** Test getEngineAttribute(). */
void
TestGameSpecInfoInfo::testGetEngineAttribute()
{
    const Id_t ENGINE_NR = 4;
    TestHarness h;
    game::spec::Engine& e = makeEngine(h, ENGINE_NR);

    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_CostD).orElse(-1),        3);
    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_CostM).orElse(-1),        15);
    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_CostMC).orElse(-1),       53);
    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_CostT).orElse(-1),        3);
    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_MaxEfficientWarp).orElse(-1), 6);
    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_Id).orElse(-1),           ENGINE_NR);
    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_Tech).orElse(-1),         6);

    TS_ASSERT_EQUALS(getEngineAttribute(e, gsi::Range_DamagePower).isValid(), false);
}

/** Test getBeamAttribute(). */
void
TestGameSpecInfoInfo::testGetBeamAttribute()
{
    const Id_t BEAM_NR = 2;
    const int VIEWPOINT = 4;
    TestHarness h;
    game::spec::Beam& b = makeBeam(h, BEAM_NR);

    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_CostD,        h.root, VIEWPOINT).orElse(-1), 12);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_CostM,        h.root, VIEWPOINT).orElse(-1), 1);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_CostMC,       h.root, VIEWPOINT).orElse(-1), 10);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_CostT,        h.root, VIEWPOINT).orElse(-1), 1);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_DamagePower,  h.root, VIEWPOINT).orElse(-1), 25);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_HitOdds,      h.root, VIEWPOINT).orElse(-1), 100);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_KillPower,    h.root, VIEWPOINT).orElse(-1), 10);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_Mass,         h.root, VIEWPOINT).orElse(-1), 4);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_RechargeTime, h.root, VIEWPOINT).orElse(-1), 150);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_Id,           h.root, VIEWPOINT).orElse(-1), BEAM_NR);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_IsDeathRay,   h.root, VIEWPOINT).orElse(-1), 0);
    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_Tech,         h.root, VIEWPOINT).orElse(-1), 3);

    TS_ASSERT_EQUALS(getBeamAttribute(b, gsi::Range_MaxCrew,      h.root, VIEWPOINT).isValid(), false);
}

/** Test getTorpedoAttribute(). */
void
TestGameSpecInfoInfo::testGetTorpAttribute()
{
    const Id_t LAUNCHER_NR = 10;
    const int VIEWPOINT = 4;
    TestHarness h;
    game::spec::TorpedoLauncher& tl = makeLauncher(h, LAUNCHER_NR);

    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_CostD,        h.root, VIEWPOINT).orElse(-1), 3);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_CostM,        h.root, VIEWPOINT).orElse(-1), 8);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_CostMC,       h.root, VIEWPOINT).orElse(-1), 120);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_CostT,        h.root, VIEWPOINT).orElse(-1), 1);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_DamagePower,  h.root, VIEWPOINT).orElse(-1), 96);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_HitOdds,      h.root, VIEWPOINT).orElse(-1), 65);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_KillPower,    h.root, VIEWPOINT).orElse(-1), 50);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_Mass,         h.root, VIEWPOINT).orElse(-1), 3);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_RechargeTime, h.root, VIEWPOINT).orElse(-1), 44);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_Id,           h.root, VIEWPOINT).orElse(-1), LAUNCHER_NR);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_IsDeathRay,   h.root, VIEWPOINT).orElse(-1), 0);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_Tech,         h.root, VIEWPOINT).orElse(-1), 8);
    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_TorpCost,     h.root, VIEWPOINT).orElse(-1), 36);

    TS_ASSERT_EQUALS(getTorpedoAttribute(tl, gsi::Range_MaxCrew,      h.root, VIEWPOINT).isValid(), false);
}

/** Test getFighterAttribute(). */
void
TestGameSpecInfoInfo::testGetFighterAttribute()
{
    TestHarness h;
    game::spec::Fighter ftr(3, h.root.hostConfiguration(), h.root.playerList(), h.tx);

    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_CostD,        h.root).orElse(-1), 0);
    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_CostM,        h.root).orElse(-1), 2);
    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_CostMC,       h.root).orElse(-1), 100);
    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_CostT,        h.root).orElse(-1), 3);
    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_DamagePower,  h.root).orElse(-1), 2);
    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_KillPower,    h.root).orElse(-1), 2);
    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_RechargeTime, h.root).orElse(-1), 21);

    TS_ASSERT_EQUALS(getFighterAttribute(ftr, gsi::Range_MaxCrew,      h.root).isValid(), false);
}

