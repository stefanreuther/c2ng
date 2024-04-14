/**
  *  \file test/game/battleorderruletest.cpp
  *  \brief Test for game::BattleOrderRule
  */

#include "game/battleorderrule.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/minefield.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"

using game::BattleOrderRule;
using game::HostVersion;

/** Test getShipBattleOrder() function. */
AFL_TEST("game.BattleOrderRule:getShipBattleOrder", a)
{
    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    /*
     *  Non-Numerical FCode
     */

    //                Host/Rule                 FCode  Weapon Enemy  Kill   Fuel
    // - Weapons and Fuel -
    a.checkEqual("01", tRule.getShipBattleOrder("xyz", true,  false, false, true),  1015);
    a.checkEqual("02", pRule.getShipBattleOrder("xyz", true,  false, false, true),  1002);

    a.checkEqual("11", tRule.getShipBattleOrder("xyz", true,  true,  false, true),  1010);
    a.checkEqual("12", pRule.getShipBattleOrder("xyz", true,  true,  false, true),  1002);

    a.checkEqual("21", tRule.getShipBattleOrder("xyz", true,  false, true,  true),  1005);
    a.checkEqual("22", pRule.getShipBattleOrder("xyz", true,  false, true,  true),  1000);

    a.checkEqual("31", tRule.getShipBattleOrder("xyz", true,  true,  true,  true),  1000);
    a.checkEqual("32", pRule.getShipBattleOrder("xyz", true,  true,  true,  true),  1000);

    // - No weapons, Fuel - (affects PHost case)
    a.checkEqual("41", tRule.getShipBattleOrder("xyz", false, false, false, true),  1015);
    a.checkEqual("42", pRule.getShipBattleOrder("xyz", false, false, false, true),  1004);

    a.checkEqual("51", tRule.getShipBattleOrder("xyz", false, true,  false, true),  1010);
    a.checkEqual("52", pRule.getShipBattleOrder("xyz", false, true,  false, true),  1004);

    a.checkEqual("61", tRule.getShipBattleOrder("xyz", false, false, true,  true),  1005);
    a.checkEqual("62", pRule.getShipBattleOrder("xyz", false, false, true,  true),  1000);

    a.checkEqual("71", tRule.getShipBattleOrder("xyz", false, true,  true,  true),  1000);
    a.checkEqual("72", pRule.getShipBattleOrder("xyz", false, true,  true,  true),  1000);

    // - Weapons but no fuel - (affects THost case if numerical FC is used)
    a.checkEqual("81", tRule.getShipBattleOrder("xyz", true,  false, false, false), 1015);
    a.checkEqual("82", pRule.getShipBattleOrder("xyz", true,  false, false, false), 1002);

    a.checkEqual("91", tRule.getShipBattleOrder("xyz", true,  true,  false, false), 1010);
    a.checkEqual("92", pRule.getShipBattleOrder("xyz", true,  true,  false, false), 1002);

    a.checkEqual("101", tRule.getShipBattleOrder("xyz", true,  false, true,  false), 1005);
    a.checkEqual("102", pRule.getShipBattleOrder("xyz", true,  false, true,  false), 1000);

    a.checkEqual("111", tRule.getShipBattleOrder("xyz", true,  true,  true,  false), 1000);
    a.checkEqual("112", pRule.getShipBattleOrder("xyz", true,  true,  true,  false), 1000);

    // - Neither weapons nor fuel -
    a.checkEqual("121", tRule.getShipBattleOrder("xyz", false, false, false, false), 1015);
    a.checkEqual("122", pRule.getShipBattleOrder("xyz", false, false, false, false), 1004);

    a.checkEqual("131", tRule.getShipBattleOrder("xyz", false, true,  false, false), 1010);
    a.checkEqual("132", pRule.getShipBattleOrder("xyz", false, true,  false, false), 1004);

    a.checkEqual("141", tRule.getShipBattleOrder("xyz", false, false, true,  false), 1005);
    a.checkEqual("142", pRule.getShipBattleOrder("xyz", false, false, true,  false), 1000);

    a.checkEqual("151", tRule.getShipBattleOrder("xyz", false, true,  true,  false), 1000);
    a.checkEqual("152", pRule.getShipBattleOrder("xyz", false, true,  true,  false), 1000);

    /*
     *  Negative Numerical FCode
     */

    //                 Host/Rule                 FCode  Weapon Enemy  Kill   Fuel
    // - Weapons and Fuel -
    a.checkEqual("161", tRule.getShipBattleOrder("-42", true,  false, false, true),  1015);
    a.checkEqual("162", pRule.getShipBattleOrder("-42", true,  false, false, true),  -42);

    a.checkEqual("171", tRule.getShipBattleOrder("-42", true,  true,  false, true),  1010);
    a.checkEqual("172", pRule.getShipBattleOrder("-42", true,  true,  false, true),  -42);

    a.checkEqual("181", tRule.getShipBattleOrder("-42", true,  false, true,  true),  1005);
    a.checkEqual("182", pRule.getShipBattleOrder("-42", true,  false, true,  true),  -42);

    a.checkEqual("191", tRule.getShipBattleOrder("-42", true,  true,  true,  true),  1000);
    a.checkEqual("192", pRule.getShipBattleOrder("-42", true,  true,  true,  true),  -42);

    // - No weapons, Fuel - (affects PHost case)
    a.checkEqual("201", tRule.getShipBattleOrder("-42", false, false, false, true),  1015);
    a.checkEqual("202", pRule.getShipBattleOrder("-42", false, false, false, true),  -42);

    a.checkEqual("211", tRule.getShipBattleOrder("-42", false, true,  false, true),  1010);
    a.checkEqual("212", pRule.getShipBattleOrder("-42", false, true,  false, true),  -42);

    a.checkEqual("221", tRule.getShipBattleOrder("-42", false, false, true,  true),  1005);
    a.checkEqual("222", pRule.getShipBattleOrder("-42", false, false, true,  true),  -42);

    a.checkEqual("231", tRule.getShipBattleOrder("-42", false, true,  true,  true),  1000);
    a.checkEqual("232", pRule.getShipBattleOrder("-42", false, true,  true,  true),  -42);

    // - Weapons but no fuel - (affects THost case if numerical FC is used)
    a.checkEqual("241", tRule.getShipBattleOrder("-42", true,  false, false, false), 1015);
    a.checkEqual("242", pRule.getShipBattleOrder("-42", true,  false, false, false), -42);

    a.checkEqual("251", tRule.getShipBattleOrder("-42", true,  true,  false, false), 1010);
    a.checkEqual("252", pRule.getShipBattleOrder("-42", true,  true,  false, false), -42);

    a.checkEqual("261", tRule.getShipBattleOrder("-42", true,  false, true,  false), 1005);
    a.checkEqual("262", pRule.getShipBattleOrder("-42", true,  false, true,  false), -42);

    a.checkEqual("271", tRule.getShipBattleOrder("-42", true,  true,  true,  false), 1000);
    a.checkEqual("272", pRule.getShipBattleOrder("-42", true,  true,  true,  false), -42);

    // - Neither weapons nor fuel -
    a.checkEqual("281", tRule.getShipBattleOrder("-42", false, false, false, false), 1015);
    a.checkEqual("282", pRule.getShipBattleOrder("-42", false, false, false, false), -42);

    a.checkEqual("291", tRule.getShipBattleOrder("-42", false, true,  false, false), 1010);
    a.checkEqual("292", pRule.getShipBattleOrder("-42", false, true,  false, false), -42);

    a.checkEqual("301", tRule.getShipBattleOrder("-42", false, false, true,  false), 1005);
    a.checkEqual("302", pRule.getShipBattleOrder("-42", false, false, true,  false), -42);

    a.checkEqual("311", tRule.getShipBattleOrder("-42", false, true,  true,  false), 1000);
    a.checkEqual("312", pRule.getShipBattleOrder("-42", false, true,  true,  false), -42);

    /*
     *  Numerical FCode
     */

    //                 Host/Rule                 FCode  Weapon Enemy  Kill   Fuel
    // - Weapons and Fuel -
    a.checkEqual("321", tRule.getShipBattleOrder("-42", true,  false, false, true),  1015);
    a.checkEqual("322", pRule.getShipBattleOrder("-42", true,  false, false, true),  -42);

    a.checkEqual("331", tRule.getShipBattleOrder("-42", true,  true,  false, true),  1010);
    a.checkEqual("332", pRule.getShipBattleOrder("-42", true,  true,  false, true),  -42);

    a.checkEqual("341", tRule.getShipBattleOrder("-42", true,  false, true,  true),  1005);
    a.checkEqual("342", pRule.getShipBattleOrder("-42", true,  false, true,  true),  -42);

    a.checkEqual("351", tRule.getShipBattleOrder("-42", true,  true,  true,  true),  1000);
    a.checkEqual("352", pRule.getShipBattleOrder("-42", true,  true,  true,  true),  -42);

    // - No weapons, Fuel - (affects PHost case)
    a.checkEqual("361", tRule.getShipBattleOrder("-42", false, false, false, true),  1015);
    a.checkEqual("362", pRule.getShipBattleOrder("-42", false, false, false, true),  -42);

    a.checkEqual("371", tRule.getShipBattleOrder("-42", false, true,  false, true),  1010);
    a.checkEqual("372", pRule.getShipBattleOrder("-42", false, true,  false, true),  -42);

    a.checkEqual("381", tRule.getShipBattleOrder("-42", false, false, true,  true),  1005);
    a.checkEqual("382", pRule.getShipBattleOrder("-42", false, false, true,  true),  -42);

    a.checkEqual("391", tRule.getShipBattleOrder("-42", false, true,  true,  true),  1000);
    a.checkEqual("392", pRule.getShipBattleOrder("-42", false, true,  true,  true),  -42);

    // - Weapons but no fuel - (affects THost case if numerical FC is used)
    a.checkEqual("401", tRule.getShipBattleOrder("-42", true,  false, false, false), 1015);
    a.checkEqual("402", pRule.getShipBattleOrder("-42", true,  false, false, false), -42);

    a.checkEqual("411", tRule.getShipBattleOrder("-42", true,  true,  false, false), 1010);
    a.checkEqual("412", pRule.getShipBattleOrder("-42", true,  true,  false, false), -42);

    a.checkEqual("421", tRule.getShipBattleOrder("-42", true,  false, true,  false), 1005);
    a.checkEqual("422", pRule.getShipBattleOrder("-42", true,  false, true,  false), -42);

    a.checkEqual("431", tRule.getShipBattleOrder("-42", true,  true,  true,  false), 1000);
    a.checkEqual("432", pRule.getShipBattleOrder("-42", true,  true,  true,  false), -42);

    // - Neither weapons nor fuel -
    a.checkEqual("441", tRule.getShipBattleOrder("-42", false, false, false, false), 1015);
    a.checkEqual("442", pRule.getShipBattleOrder("-42", false, false, false, false), -42);

    a.checkEqual("451", tRule.getShipBattleOrder("-42", false, true,  false, false), 1010);
    a.checkEqual("452", pRule.getShipBattleOrder("-42", false, true,  false, false), -42);

    a.checkEqual("461", tRule.getShipBattleOrder("-42", false, false, true,  false), 1005);
    a.checkEqual("462", pRule.getShipBattleOrder("-42", false, false, true,  false), -42);

    a.checkEqual("471", tRule.getShipBattleOrder("-42", false, true,  true,  false), 1000);
    a.checkEqual("472", pRule.getShipBattleOrder("-42", false, true,  true,  false), -42);
}

/** Test getPlanetBattleOrder() function. */
AFL_TEST("game.BattleOrderRule:getPlanetBattleOrder", a)
{
    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    /*
     *  Numerical FCode
     */

    //              Host/Rule                   FCode  Defense
    a.checkEqual("01", tRule.getPlanetBattleOrder("345", false), BattleOrderRule::UNKNOWN);
    a.checkEqual("02", pRule.getPlanetBattleOrder("345", false), 345);
    a.checkEqual("03", tRule.getPlanetBattleOrder("345", true),  BattleOrderRule::UNKNOWN);
    a.checkEqual("04", pRule.getPlanetBattleOrder("345", true),  345);

    /*
     *  ATT
     */

    //              Host/Rule                   FCode  Defense
    a.checkEqual("11", tRule.getPlanetBattleOrder("ATT", false), BattleOrderRule::UNKNOWN);
    a.checkEqual("12", pRule.getPlanetBattleOrder("ATT", false), 0);
    a.checkEqual("13", tRule.getPlanetBattleOrder("ATT", true),  BattleOrderRule::UNKNOWN);
    a.checkEqual("14", pRule.getPlanetBattleOrder("ATT", true),  0);

    /*
     *  Non-Numerical FCode
     */

    //              Host/Rule                   FCode  Defense
    a.checkEqual("21", tRule.getPlanetBattleOrder("poo", false), BattleOrderRule::UNKNOWN);
    a.checkEqual("22", pRule.getPlanetBattleOrder("poo", false), 1003);
    a.checkEqual("23", tRule.getPlanetBattleOrder("poo", true),  BattleOrderRule::UNKNOWN);
    a.checkEqual("24", pRule.getPlanetBattleOrder("poo", true),  1001);
}

/** Test get(game::map::Ship). */
AFL_TEST("game.BattleOrderRule:get:game-ship", a)
{
    using game::map::Ship;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    // Totally unknown ship
    {
        Ship sh(99);
        a.checkEqual("01", tRule.get(sh), BattleOrderRule::UNKNOWN);
        a.checkEqual("02", pRule.get(sh), BattleOrderRule::UNKNOWN);
    }

    // Friendly code known
    {
        Ship sh(99);
        sh.setFriendlyCode(String_t("-50"));
        a.checkEqual("11", tRule.get(sh), 1015);     // assumes no fuel and not aggressive
        a.checkEqual("12", pRule.get(sh), -50);

        // Test Object& entry point as well
        const game::map::Object& obj = sh;
        a.checkEqual("21", tRule.get(obj), 1015);
        a.checkEqual("22", pRule.get(obj), -50);
    }

    // Friendly code and mission known
    {
        Ship sh(99);
        sh.setFriendlyCode(String_t("xyz"));
        sh.setMission(1, 0, 0);
        sh.setPrimaryEnemy(7);
        sh.setCargo(game::Element::Neutronium, 100);
        a.checkEqual("31", tRule.get(sh), 1010);      // not kill, but PE
        a.checkEqual("32", pRule.get(sh), 1004);      // no weapons
    }

    // Friendly code, weapons and mission known
    {
        Ship sh(99);
        sh.setFriendlyCode(String_t("xyz"));
        sh.setMission(1, 0, 0);
        sh.setPrimaryEnemy(7);
        sh.setCargo(game::Element::Neutronium, 100);
        sh.setBeamType(1);
        sh.setNumBeams(7);
        sh.setTorpedoType(1);
        sh.setNumLaunchers(2);
        sh.setNumBays(0);
        a.checkEqual("41", tRule.get(sh), 1010);      // not kill, but PE
        a.checkEqual("42", pRule.get(sh), 1002);      // not kill, but PE + weapons
    }
}

/** Test get(game::map::Planet&). */
AFL_TEST("game.BattleOrderRule:get:game-planet", a)
{
    using game::map::Planet;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    // Totally unknown planet
    {
        Planet pl(99);
        a.checkEqual("01", tRule.get(pl), BattleOrderRule::UNKNOWN);
        a.checkEqual("02", pRule.get(pl), BattleOrderRule::UNKNOWN);
    }

    // Friendly code known
    {
        Planet pl(99);
        pl.setFriendlyCode(String_t("-50"));
        a.checkEqual("11", tRule.get(pl), BattleOrderRule::UNKNOWN);
        a.checkEqual("12", pRule.get(pl), -50);

        // Test Object& entry point as well
        const game::map::Object& obj = pl;
        a.checkEqual("21", tRule.get(obj), BattleOrderRule::UNKNOWN);
        a.checkEqual("22", pRule.get(obj), -50);
    }

    // Friendly code and defense known
    {
        Planet pl(99);
        pl.setFriendlyCode(String_t("xyz"));
        pl.setNumBuildings(game::DefenseBuilding, 1);
        a.checkEqual("31", tRule.get(pl), BattleOrderRule::UNKNOWN);
        a.checkEqual("32", pRule.get(pl), 1001);
    }
}

/** Test get(game::map::Object), neither ship nor planet. */
AFL_TEST("game.BattleOrderRule:get:game-other", a)
{
    using game::map::Minefield;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    Minefield mf(99);
    a.checkEqual("01", tRule.get(mf), BattleOrderRule::UNKNOWN);
    a.checkEqual("02", pRule.get(mf), BattleOrderRule::UNKNOWN);
}

/** Test get(game::sim::Ship&). */
AFL_TEST("game.BattleOrderRule:get:sim-ship", a)
{
    using game::sim::Ship;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    {
        Ship sh;
        sh.setAggressiveness(0);
        sh.setFriendlyCode("-50");
        a.checkEqual("01", tRule.get(sh), 1015);
        a.checkEqual("02", pRule.get(sh), -50);

        const game::sim::Object& obj = sh;
        a.checkEqual("11", tRule.get(obj), 1015);
        a.checkEqual("12", pRule.get(obj), -50);
    }

    {
        Ship sh;
        sh.setAggressiveness(Ship::agg_Kill);
        sh.setFriendlyCode("xxx");
        a.checkEqual("21", tRule.get(sh), 1000);
        a.checkEqual("22", pRule.get(sh), 1000);
    }
}

/** Test get(game::sim::Planet&). */
AFL_TEST("game.BattleOrderRule:get:sim-planet", a)
{
    using game::sim::Planet;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    {
        Planet pl;
        pl.setFriendlyCode("200");
        pl.setDefense(0);
        a.checkEqual("01", tRule.get(pl), BattleOrderRule::UNKNOWN);
        a.checkEqual("02", pRule.get(pl), 200);

        const game::sim::Object& obj = pl;
        a.checkEqual("11", tRule.get(obj), BattleOrderRule::UNKNOWN);
        a.checkEqual("12", pRule.get(obj), 200);
    }
}

/** Test get(game::sim::Object), neither ship nor planet. */
AFL_TEST("game.BattleOrderRule:get:sim-other", a)
{
    class Tester : public game::sim::Object {
     public:
        virtual bool hasImpliedAbility(game::sim::Ability /*which*/, const game::sim::Configuration& /*opts*/, const game::spec::ShipList& /*shipList*/, const game::config::HostConfiguration& /*config*/) const
            { return false; }
    };

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    Tester t;
    a.checkEqual("01", tRule.get(t), BattleOrderRule::UNKNOWN);
    a.checkEqual("02", pRule.get(t), BattleOrderRule::UNKNOWN);
}
