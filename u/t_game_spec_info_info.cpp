/**
  *  \file u/t_game_spec_info_info.cpp
  *  \brief Test for game::spec::info::Info
  */

#include "game/spec/info/info.hpp"

#include "t_game_spec_info.hpp"
#include "afl/string/nulltranslator.hpp"
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

/** Test describeWeaponEffects(), Tim-Host version. */
void
TestGameSpecInfoInfo::testDescribeWeaponEffectsTim()
{
    // Environment
    game::spec::ShipList shipList;
    game::test::initStandardBeams(shipList);
    game::test::initStandardTorpedoes(shipList);

    game::test::Root root(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,0)));
    afl::string::NullTranslator tx;

    // Ship query
    game::ShipQuery q;
    q.setCombatMass(330, 50);
    q.setCrew(348);
    q.setOwner(11);

    // Action
    game::spec::info::WeaponEffects result;
    describeWeaponEffects(result, q, shipList, root, tx);

    // Verify
    TS_ASSERT_EQUALS(result.effectScale, 1);
    TS_ASSERT_EQUALS(result.mass,        330);
    TS_ASSERT_EQUALS(result.usedESBRate, 50);
    TS_ASSERT_EQUALS(result.crew,        348);
    TS_ASSERT_EQUALS(result.damageLimit, 100);
    TS_ASSERT_EQUALS(result.player,      11);

    TS_ASSERT_EQUALS(result.beamEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.beamEffects[0].name, "Laser");
    TS_ASSERT_EQUALS(result.beamEffects[0].shieldEffect, 2);
    TS_ASSERT_EQUALS(result.beamEffects[0].damageEffect, 1);
    TS_ASSERT_EQUALS(result.beamEffects[0].crewEffect, 2);
    TS_ASSERT_EQUALS(result.beamEffects[9].name, "Heavy Phaser");
    TS_ASSERT_EQUALS(result.beamEffects[9].shieldEffect, 12);
    TS_ASSERT_EQUALS(result.beamEffects[9].damageEffect, 4);
    TS_ASSERT_EQUALS(result.beamEffects[9].crewEffect, 8);

    TS_ASSERT_EQUALS(result.torpedoEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].name, "Mark 1 Photon");
    TS_ASSERT_EQUALS(result.torpedoEffects[0].shieldEffect, 3);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].damageEffect, 2);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].crewEffect, 2);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].name, "Mark 8 Photon");
    TS_ASSERT_EQUALS(result.torpedoEffects[9].shieldEffect, 28);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].damageEffect, 8);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].crewEffect, 17);

    TS_ASSERT_EQUALS(result.fighterEffects.size(), 1U);
    TS_ASSERT_EQUALS(result.fighterEffects[0].name, "Fighter");
    TS_ASSERT_EQUALS(result.fighterEffects[0].shieldEffect, 1);
    TS_ASSERT_EQUALS(result.fighterEffects[0].damageEffect, 1);
    TS_ASSERT_EQUALS(result.fighterEffects[0].crewEffect, 0);
}

/** Test describeWeaponEffects(), PHost Alternative Combat version. */
void
TestGameSpecInfoInfo::testDescribeWeaponEffectsPHostAC()
{
    // Environment
    // (Pleiades 13, player 7, turn 74, ship 72)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    game::test::Root root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root.hostConfiguration();
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
    describeWeaponEffects(result, q, shipList, root, tx);

    // Verify
    TS_ASSERT_DIFFERS(result.effectScale, 1);
    TS_ASSERT_EQUALS(result.mass,        207);
    TS_ASSERT_EQUALS(result.usedESBRate, 23);
    TS_ASSERT_EQUALS(result.crew,        257);
    TS_ASSERT_EQUALS(result.damageLimit, 100);
    TS_ASSERT_EQUALS(result.player,      7);
    const double scale = 1.0 / result.effectScale;

    TS_ASSERT_EQUALS(result.beamEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.beamEffects[0].name, "Laser Cannon");
    TS_ASSERT_DELTA(result.beamEffects[0].shieldEffect * scale, 1.35, 0.01);
    TS_ASSERT_DELTA(result.beamEffects[0].damageEffect * scale, 0.67, 0.01);
    TS_ASSERT_DELTA(result.beamEffects[0].crewEffect   * scale, 0.07, 0.01);
    TS_ASSERT_EQUALS(result.beamEffects[1].name, "Kill-O-Zap");
    TS_ASSERT_EQUALS(result.beamEffects[1].shieldEffect, 0);
    TS_ASSERT_EQUALS(result.beamEffects[1].damageEffect, 0);
    TS_ASSERT_DELTA(result.beamEffects[1].crewEffect   * scale, 1.08, 0.01);
    TS_ASSERT_EQUALS(result.beamEffects[9].name, "Multitraf Spiral");
    TS_ASSERT_DELTA(result.beamEffects[9].shieldEffect * scale, 15.38, 0.01);
    TS_ASSERT_DELTA(result.beamEffects[9].damageEffect * scale,  7.69, 0.01);
    TS_ASSERT_DELTA(result.beamEffects[9].crewEffect   * scale,  2.88, 0.01);

    TS_ASSERT_EQUALS(result.torpedoEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].name, "Space Rocket");
    TS_ASSERT_DELTA(result.torpedoEffects[0].shieldEffect * scale, 5.77, 0.01);
    TS_ASSERT_DELTA(result.torpedoEffects[0].damageEffect * scale, 2.88, 0.01);
    TS_ASSERT_DELTA(result.torpedoEffects[0].crewEffect   * scale, 0.36, 0.01);
    TS_ASSERT_EQUALS(result.torpedoEffects[1].name, "Paralyso-Matic Bomb");
    TS_ASSERT_EQUALS(result.torpedoEffects[1].shieldEffect, 0);
    TS_ASSERT_EQUALS(result.torpedoEffects[1].damageEffect, 0);
    TS_ASSERT_DELTA(result.torpedoEffects[1].crewEffect   * scale, 1.80, 0.01);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].name, "Selphyr-Fataro-Dev.");
    TS_ASSERT_DELTA(result.torpedoEffects[9].shieldEffect * scale, 19.04, 0.01);
    TS_ASSERT_DELTA(result.torpedoEffects[9].damageEffect * scale,  9.52, 0.01);
    TS_ASSERT_DELTA(result.torpedoEffects[9].crewEffect   * scale,  2.88, 0.01);

    TS_ASSERT_EQUALS(result.fighterEffects.size(), 1U);
    TS_ASSERT_EQUALS(result.fighterEffects[0].name, "Fighter");
    TS_ASSERT_DELTA(result.fighterEffects[0].shieldEffect * scale, 1.73, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[0].damageEffect * scale, 0.87, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[0].crewEffect   * scale, 0.65, 0.01);
}

/** Test describeWeaponEffects(), PHost Non-Alternative-Combat version.
    This is the same as above, but with AC turned off; validated against PCC2. */
void
TestGameSpecInfoInfo::testDescribeWeaponEffectsPHostNonAC()
{
    // Environment
    // (Pleiades 13, player 7, turn 74, ship 72)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    game::test::Root root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root.hostConfiguration();
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
    describeWeaponEffects(result, q, shipList, root, tx);

    // Verify
    TS_ASSERT_EQUALS(result.effectScale, 1);
    TS_ASSERT_EQUALS(result.mass,        207);
    TS_ASSERT_EQUALS(result.usedESBRate, 23);
    TS_ASSERT_EQUALS(result.crew,        257);
    TS_ASSERT_EQUALS(result.damageLimit, 100);
    TS_ASSERT_EQUALS(result.player,      7);

    TS_ASSERT_EQUALS(result.beamEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.beamEffects[0].name, "Laser Cannon");
    TS_ASSERT_EQUALS(result.beamEffects[0].shieldEffect, 2);
    TS_ASSERT_EQUALS(result.beamEffects[0].damageEffect, 0);
    TS_ASSERT_EQUALS(result.beamEffects[0].crewEffect,   0);
    TS_ASSERT_EQUALS(result.beamEffects[1].name, "Kill-O-Zap");
    TS_ASSERT_EQUALS(result.beamEffects[1].shieldEffect, 0);
    TS_ASSERT_EQUALS(result.beamEffects[1].damageEffect, 0);
    TS_ASSERT_EQUALS(result.beamEffects[1].crewEffect,   1);
    TS_ASSERT_EQUALS(result.beamEffects[9].name, "Multitraf Spiral");
    TS_ASSERT_EQUALS(result.beamEffects[9].shieldEffect, 16);
    TS_ASSERT_EQUALS(result.beamEffects[9].damageEffect,  2);
    TS_ASSERT_EQUALS(result.beamEffects[9].crewEffect,    3);

    TS_ASSERT_EQUALS(result.torpedoEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].name, "Space Rocket");
    TS_ASSERT_EQUALS(result.torpedoEffects[0].shieldEffect, 13);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].damageEffect, 1);
    TS_ASSERT_EQUALS(result.torpedoEffects[0].crewEffect,   1);
    TS_ASSERT_EQUALS(result.torpedoEffects[1].name, "Paralyso-Matic Bomb");
    TS_ASSERT_EQUALS(result.torpedoEffects[1].shieldEffect, 0);
    TS_ASSERT_EQUALS(result.torpedoEffects[1].damageEffect, 0);
    TS_ASSERT_EQUALS(result.torpedoEffects[1].crewEffect,   4);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].name, "Selphyr-Fataro-Dev.");
    TS_ASSERT_EQUALS(result.torpedoEffects[9].shieldEffect, 39);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].damageEffect,  4);
    TS_ASSERT_EQUALS(result.torpedoEffects[9].crewEffect,    6);

    TS_ASSERT_EQUALS(result.fighterEffects.size(), 1U);
    TS_ASSERT_EQUALS(result.fighterEffects[0].name, "Fighter");
    TS_ASSERT_EQUALS(result.fighterEffects[0].shieldEffect, 3);
    TS_ASSERT_EQUALS(result.fighterEffects[0].damageEffect, 0);
    TS_ASSERT_EQUALS(result.fighterEffects[0].crewEffect,   1);
}

/** Test describeWeaponEffects(), mixed fighter behaviour. */
void
TestGameSpecInfoInfo::testDescribeWeaponEffectsPHostMixedFighters()
{
    // Environment
    // (Pleiades 13, player 7, turn 74, ship 72)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    game::test::Root root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root.hostConfiguration();
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
    describeWeaponEffects(result, q, shipList, root, tx);

    // Verify
    TS_ASSERT_DIFFERS(result.effectScale, 1);
    TS_ASSERT_EQUALS(result.mass,        207);
    TS_ASSERT_EQUALS(result.usedESBRate, 23);
    TS_ASSERT_EQUALS(result.crew,        257);
    TS_ASSERT_EQUALS(result.damageLimit, 100);
    TS_ASSERT_EQUALS(result.player,      7);
    const double scale = 1.0 / result.effectScale;

    // FighterBeamExplosive = 9,  9, 9, 10, 9, 9, 10, 9, 10, 6, 9
    // FighterBeamKill      = 9, 12, 9, 10, 9, 9, 13, 9,  8, 6, 9
    // -> Fed (9/9)           x      x      x  x      x         x
    // -> Liz (9/12)              x
    // -> Kli (10/10)                   x
    // -> Tho (10/13) (not listed!)                x
    // -> Rob (10/8)                                      x
    // -> Reb (6/6)                                          x
    TS_ASSERT_EQUALS(result.fighterEffects.size(), 5U);
    TS_ASSERT_EQUALS(result.fighterEffects[0].name, "Player 1 Fighter");
    TS_ASSERT_DELTA(result.fighterEffects[0].shieldEffect * scale, 1.73, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[0].damageEffect * scale, 0.87, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[0].crewEffect   * scale, 0.65, 0.01);
    TS_ASSERT_EQUALS(result.fighterEffects[1].name, "Player 2 Fighter");
    TS_ASSERT_DELTA(result.fighterEffects[1].shieldEffect * scale, 1.73, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[1].damageEffect * scale, 0.87, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[1].crewEffect   * scale, 0.87, 0.01);
    TS_ASSERT_EQUALS(result.fighterEffects[2].name, "Player 4 Fighter");
    TS_ASSERT_DELTA(result.fighterEffects[2].shieldEffect * scale, 1.92, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[2].damageEffect * scale, 0.96, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[2].crewEffect   * scale, 0.72, 0.01);
    TS_ASSERT_EQUALS(result.fighterEffects[3].name, "Player 9 Fighter");
    TS_ASSERT_DELTA(result.fighterEffects[3].shieldEffect * scale, 1.92, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[3].damageEffect * scale, 0.96, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[3].crewEffect   * scale, 0.58, 0.01);
    TS_ASSERT_EQUALS(result.fighterEffects[4].name, "Player 10 Fighter");
    TS_ASSERT_DELTA(result.fighterEffects[4].shieldEffect * scale, 1.15, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[4].damageEffect * scale, 0.58, 0.01);
    TS_ASSERT_DELTA(result.fighterEffects[4].crewEffect   * scale, 0.43, 0.01);
}

/** Test describeWeaponEffects(), experience behaviour. */
void
TestGameSpecInfoInfo::testDescribeWeaponEffectsPHostExp()
{
    // Environment (similar as testDescribeWeaponEffectsPHostAC)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    game::test::Root root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root.hostConfiguration();
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
    describeWeaponEffects(result, q, shipList, root, tx);

    // Verify specimen
    const double scale = 1.0 / result.effectScale;
    TS_ASSERT_EQUALS(result.beamEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.beamEffects[9].name, "Multitraf Spiral");
    TS_ASSERT_DELTA(result.beamEffects[9].shieldEffect * scale, 15.38, 0.01);
    TS_ASSERT_DELTA(result.beamEffects[9].damageEffect * scale,  7.69, 0.01);
    TS_ASSERT_DELTA(result.beamEffects[9].crewEffect   * scale,  0.58, 0.01);  // CrewKillScaling reduced from 15 -> 3 (=factor 5), effect also reduced by factor 5
}

/** Test describeWeaponEffects(), experience behaviour, non-AC. */
void
TestGameSpecInfoInfo::testDescribeWeaponEffectsPHostExpNonAC()
{
    // Environment (similar as testDescribeWeaponEffectsPHostNonAC)
    game::spec::ShipList shipList;
    game::test::initPList32Beams(shipList);
    game::test::initPList32Torpedoes(shipList);

    game::test::Root root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));
    afl::string::NullTranslator tx;

    HostConfiguration& config = root.hostConfiguration();
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
    describeWeaponEffects(result, q, shipList, root, tx);

    // Verify specimen
    TS_ASSERT_EQUALS(result.effectScale, 1);
    TS_ASSERT_EQUALS(result.beamEffects.size(), 10U);
    TS_ASSERT_EQUALS(result.beamEffects[9].name, "Multitraf Spiral");
    TS_ASSERT_EQUALS(result.beamEffects[9].shieldEffect, 16);
    TS_ASSERT_EQUALS(result.beamEffects[9].damageEffect,  2);
    TS_ASSERT_EQUALS(result.beamEffects[9].crewEffect,    1);
}

void
TestGameSpecInfoInfo::testDescribeHullFunction()
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
        h.root.playerList().create(i);
    }

    h.root.hostConfiguration()[game::config::HostConfiguration::NumExperienceLevels].set(5);
    h.root.hostConfiguration()[game::config::HostConfiguration::DamageLevelForCloakFail].set(10);

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
        describeHullFunctions(out, hfList, 0, h.shipList, h.picNamer, h.root, h.tx);
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT_EQUALS(out[0].info, "cloaking device (player 5)");
        TS_ASSERT_EQUALS(out[1].info, "tow-capture (level 3; ship)");

        TS_ASSERT(!out[0].flags.contains(game::spec::info::DamagedAbility));
        TS_ASSERT(!out[0].flags.contains(game::spec::info::ForeignAbility));
        TS_ASSERT(!out[0].flags.contains(game::spec::info::ReachableAbility));
        TS_ASSERT(!out[0].flags.contains(game::spec::info::OutgrownAbility));
    }

    // describeHullFunctions() with query
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::Abilities_t out;
        describeHullFunctions(out, hfList, &q, h.shipList, h.picNamer, h.root, h.tx);
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT_EQUALS(out[0].info, "cloaking device (player 5; damaged)");
        TS_ASSERT_EQUALS(out[1].info, "tow-capture (level 3; ship)");

        TS_ASSERT(out[0].flags.contains(game::spec::info::DamagedAbility));
        TS_ASSERT(out[0].flags.contains(game::spec::info::ForeignAbility));
        TS_ASSERT(!out[0].flags.contains(game::spec::info::ReachableAbility));
        TS_ASSERT(!out[0].flags.contains(game::spec::info::OutgrownAbility));
    }

    // describeHullFunctionDetails()
    {
        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, 0, h.shipList, h.picNamer, false, h.root, h.tx);
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT_EQUALS(out[0].name, "Cloak");
        TS_ASSERT_EQUALS(out[0].description, "cloaking device");
        TS_ASSERT_EQUALS(out[0].explanation, "it cloaks");
        // damageLimit not known (but might be someday)
        TS_ASSERT_EQUALS(out[0].playerLimit, "player 5");
        TS_ASSERT_EQUALS(out[0].levelLimit, "");
        TS_ASSERT_EQUALS(out[0].kind, game::spec::info::ClassAbility);

        TS_ASSERT_EQUALS(out[1].name, "Boarding");
        TS_ASSERT_EQUALS(out[1].description, "tow-capture");
        TS_ASSERT_EQUALS(out[1].explanation, "it boards!");
        TS_ASSERT_EQUALS(out[1].damageLimit.isValid(), false);
        TS_ASSERT_EQUALS(out[1].playerLimit, "");
        TS_ASSERT_EQUALS(out[1].levelLimit, "level 3");
        TS_ASSERT_EQUALS(out[1].kind, game::spec::info::ShipAbility);
    }

    // describeHullFunctionDetails() with query
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, &q, h.shipList, h.picNamer, false, h.root, h.tx);
        TS_ASSERT_EQUALS(out.size(), 2U);
        TS_ASSERT_EQUALS(out[0].name, "Cloak");
        TS_ASSERT_EQUALS(out[0].description, "cloaking device");
        TS_ASSERT_EQUALS(out[0].explanation, "it cloaks");
        TS_ASSERT_EQUALS(out[0].damageLimit.orElse(-1), 10);
        TS_ASSERT_EQUALS(out[0].playerLimit, "player 5");
        TS_ASSERT_EQUALS(out[0].levelLimit, "");
        TS_ASSERT_EQUALS(out[0].kind, game::spec::info::ClassAbility);
        TS_ASSERT(out[0].flags.contains(game::spec::info::DamagedAbility));
        TS_ASSERT_EQUALS(out[0].minimumExperience, 0);

        TS_ASSERT_EQUALS(out[1].name, "Boarding");
        TS_ASSERT_EQUALS(out[1].description, "tow-capture");
        TS_ASSERT_EQUALS(out[1].explanation, "it boards!");
        TS_ASSERT_EQUALS(out[1].damageLimit.isValid(), false);
        TS_ASSERT_EQUALS(out[1].playerLimit, "");
        TS_ASSERT_EQUALS(out[1].levelLimit, "level 3");
        TS_ASSERT_EQUALS(out[1].kind, game::spec::info::ShipAbility);
        TS_ASSERT_EQUALS(out[1].minimumExperience, 3000);
    }
}

void
TestGameSpecInfoInfo::testDescribeHullFunctionPicture()
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
        h.root.playerList().create(i);
    }
    h.root.hostConfiguration()[game::config::HostConfiguration::DamageLevelForCloakFail].set(10);

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
        describeHullFunctionDetails(out, hfList, &q, h.shipList, picNamer, false, h.root, h.tx);
        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT_EQUALS(out[0].name, "Cloak");
        TS_ASSERT_EQUALS(out[0].kind, game::spec::info::ClassAbility);
        TS_ASSERT(out[0].flags.contains(game::spec::info::DamagedAbility));
        TS_ASSERT_EQUALS(out[0].pictureName, "broken-cloaker");
        TS_ASSERT_EQUALS(out[0].minimumExperience, 0);
    }

    // useNormalPictures=true
    {
        game::ShipQuery q;
        q.setDamage(20);
        q.setOwner(2);

        gsi::AbilityDetails_t out;
        describeHullFunctionDetails(out, hfList, &q, h.shipList, picNamer, true, h.root, h.tx);
        TS_ASSERT_EQUALS(out.size(), 1U);
        TS_ASSERT_EQUALS(out[0].name, "Cloak");
        TS_ASSERT_EQUALS(out[0].kind, game::spec::info::ClassAbility);
        TS_ASSERT(out[0].flags.contains(game::spec::info::DamagedAbility));
        TS_ASSERT_EQUALS(out[0].pictureName, "good-cloaker");
        TS_ASSERT_EQUALS(out[0].minimumExperience, 0);
    }
}
