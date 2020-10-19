/**
  *  \file u/t_game_battleorderrule.cpp
  *  \brief Test for game::BattleOrderRule
  */

#include "game/battleorderrule.hpp"

#include "t_game.hpp"

/** Test getShipBattleOrder() function. */
void
TestGameBattleOrderRule::testGetShipBattleOrder()
{
    using game::BattleOrderRule;
    using game::HostVersion;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    /*
     *  Non-Numerical FCode
     */

    //              Host/Rule                 FCode  Weapon Enemy  Kill   Fuel
    // - Weapons and Fuel -
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  false, false, true),  1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  false, false, true),  1002);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  true,  false, true),  1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  true,  false, true),  1002);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  false, true,  true),  1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  false, true,  true),  1000);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  true,  true,  true),  1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  true,  true,  true),  1000);

    // - No weapons, Fuel - (affects PHost case)
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, false, false, true),  1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, false, false, true),  1004);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, true,  false, true),  1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, true,  false, true),  1004);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, false, true,  true),  1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, false, true,  true),  1000);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, true,  true,  true),  1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, true,  true,  true),  1000);

    // - Weapons but no fuel - (affects THost case if numerical FC is used)
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  false, false, false), 1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  false, false, false), 1002);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  true,  false, false), 1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  true,  false, false), 1002);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  false, true,  false), 1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  false, true,  false), 1000);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", true,  true,  true,  false), 1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", true,  true,  true,  false), 1000);

    // - Neither weapons nor fuel -
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, false, false, false), 1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, false, false, false), 1004);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, true,  false, false), 1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, true,  false, false), 1004);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, false, true,  false), 1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, false, true,  false), 1000);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("xyz", false, true,  true,  false), 1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("xyz", false, true,  true,  false), 1000);

    /*
     *  Negative Numerical FCode
     */

    //              Host/Rule                 FCode  Weapon Enemy  Kill   Fuel
    // - Weapons and Fuel -
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, false, true),  1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  false, true),  1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, true,  true),  1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, true,  true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  true,  true),  1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  true,  true),  -42);

    // - No weapons, Fuel - (affects PHost case)
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, false, true),  1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  false, true),  1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, true,  true),  1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, true,  true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  true,  true),  1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  true,  true),  -42);

    // - Weapons but no fuel - (affects THost case if numerical FC is used)
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, false, false), 1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  false, false), 1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, true,  false), 1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, true,  false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  true,  false), 1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  true,  false), -42);

    // - Neither weapons nor fuel -
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, false, false), 1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  false, false), 1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, true,  false), 1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, true,  false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  true,  false), 1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  true,  false), -42);

    /*
     *  Numerical FCode
     */

    //              Host/Rule                 FCode  Weapon Enemy  Kill   Fuel
    // - Weapons and Fuel -
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, false, true),  1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  false, true),  1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, true,  true),  1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, true,  true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  true,  true),  1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  true,  true),  -42);

    // - No weapons, Fuel - (affects PHost case)
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, false, true),  1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  false, true),  1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  false, true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, true,  true),  1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, true,  true),  -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  true,  true),  1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  true,  true),  -42);

    // - Weapons but no fuel - (affects THost case if numerical FC is used)
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, false, false), 1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  false, false), 1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  false, true,  false), 1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  false, true,  false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", true,  true,  true,  false), 1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", true,  true,  true,  false), -42);

    // - Neither weapons nor fuel -
    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, false, false), 1015);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  false, false), 1010);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  false, false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, false, true,  false), 1005);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, false, true,  false), -42);

    TS_ASSERT_EQUALS(tRule.getShipBattleOrder("-42", false, true,  true,  false), 1000);
    TS_ASSERT_EQUALS(pRule.getShipBattleOrder("-42", false, true,  true,  false), -42);
}

/** Test getPlanetBattleOrder() function. */
void
TestGameBattleOrderRule::testGetPlanetBattleOrder()
{
    using game::BattleOrderRule;
    using game::HostVersion;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    /*
     *  Numerical FCode
     */

    //              Host/Rule                   FCode  Defense
    TS_ASSERT_EQUALS(tRule.getPlanetBattleOrder("345", false), BattleOrderRule::UNKNOWN);
    TS_ASSERT_EQUALS(pRule.getPlanetBattleOrder("345", false), 345);
    TS_ASSERT_EQUALS(tRule.getPlanetBattleOrder("345", true),  BattleOrderRule::UNKNOWN);
    TS_ASSERT_EQUALS(pRule.getPlanetBattleOrder("345", true),  345);

    /*
     *  ATT
     */

    //              Host/Rule                   FCode  Defense
    TS_ASSERT_EQUALS(tRule.getPlanetBattleOrder("ATT", false), BattleOrderRule::UNKNOWN);
    TS_ASSERT_EQUALS(pRule.getPlanetBattleOrder("ATT", false), 0);
    TS_ASSERT_EQUALS(tRule.getPlanetBattleOrder("ATT", true),  BattleOrderRule::UNKNOWN);
    TS_ASSERT_EQUALS(pRule.getPlanetBattleOrder("ATT", true),  0);

    /*
     *  Non-Numerical FCode
     */

    //              Host/Rule                   FCode  Defense
    TS_ASSERT_EQUALS(tRule.getPlanetBattleOrder("poo", false), BattleOrderRule::UNKNOWN);
    TS_ASSERT_EQUALS(pRule.getPlanetBattleOrder("poo", false), 1003);
    TS_ASSERT_EQUALS(tRule.getPlanetBattleOrder("poo", true),  BattleOrderRule::UNKNOWN);
    TS_ASSERT_EQUALS(pRule.getPlanetBattleOrder("poo", true),  1001);
}

/** Test get(game::map::Ship). */
void
TestGameBattleOrderRule::testGameShip()
{
    using game::BattleOrderRule;
    using game::HostVersion;
    using game::map::Ship;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    // Totally unknown ship
    {
        Ship sh(99);
        TS_ASSERT_EQUALS(tRule.get(sh), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(sh), BattleOrderRule::UNKNOWN);
    }

    // Friendly code known
    {
        Ship sh(99);
        sh.setFriendlyCode(String_t("-50"));
        TS_ASSERT_EQUALS(tRule.get(sh), 1015);     // assumes no fuel and not aggressive
        TS_ASSERT_EQUALS(pRule.get(sh), -50);

        // Test Object& entry point as well
        const game::map::Object& obj = sh;
        TS_ASSERT_EQUALS(tRule.get(obj), 1015);
        TS_ASSERT_EQUALS(pRule.get(obj), -50);
    }

    // Friendly code and mission known
    {
        Ship sh(99);
        sh.setFriendlyCode(String_t("xyz"));
        sh.setMission(1, 0, 0);
        sh.setPrimaryEnemy(7);
        sh.setCargo(game::Element::Neutronium, 100);
        TS_ASSERT_EQUALS(tRule.get(sh), 1010);      // not kill, but PE
        TS_ASSERT_EQUALS(pRule.get(sh), 1004);      // no weapons
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
        TS_ASSERT_EQUALS(tRule.get(sh), 1010);      // not kill, but PE
        TS_ASSERT_EQUALS(pRule.get(sh), 1002);      // not kill, but PE + weapons
    }
}

/** Test get(game::map::Planet&). */
void
TestGameBattleOrderRule::testGamePlanet()
{
    using game::BattleOrderRule;
    using game::HostVersion;
    using game::map::Planet;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    // Totally unknown planet
    {
        Planet pl(99);
        TS_ASSERT_EQUALS(tRule.get(pl), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(pl), BattleOrderRule::UNKNOWN);
    }

    // Friendly code known
    {
        Planet pl(99);
        pl.setFriendlyCode(String_t("-50"));
        TS_ASSERT_EQUALS(tRule.get(pl), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(pl), -50);

        // Test Object& entry point as well
        const game::map::Object& obj = pl;
        TS_ASSERT_EQUALS(tRule.get(obj), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(obj), -50);
    }

    // Friendly code and defense known
    {
        Planet pl(99);
        pl.setFriendlyCode(String_t("xyz"));
        pl.setNumBuildings(game::DefenseBuilding, 1);
        TS_ASSERT_EQUALS(tRule.get(pl), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(pl), 1001);
    }
}

/** Test add(game::sim::Ship&). */
void
TestGameBattleOrderRule::testSimShip()
{
    using game::BattleOrderRule;
    using game::HostVersion;
    using game::sim::Ship;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    {
        Ship sh;
        sh.setAggressiveness(0);
        sh.setFriendlyCode("-50");
        TS_ASSERT_EQUALS(tRule.get(sh), 1015);
        TS_ASSERT_EQUALS(pRule.get(sh), -50);

        const game::sim::Object& obj = sh;
        TS_ASSERT_EQUALS(tRule.get(obj), 1015);
        TS_ASSERT_EQUALS(pRule.get(obj), -50);
    }

    {
        Ship sh;
        sh.setAggressiveness(Ship::agg_Kill);
        sh.setFriendlyCode("xxx");
        TS_ASSERT_EQUALS(tRule.get(sh), 1000);
        TS_ASSERT_EQUALS(pRule.get(sh), 1000);
    }

}

/** Test add(game::sim::Planet&). */
void
TestGameBattleOrderRule::testSimPlanet()
{
    using game::BattleOrderRule;
    using game::HostVersion;
    using game::sim::Planet;

    const BattleOrderRule tRule(HostVersion(HostVersion::Host,  MKVERSION(3, 22, 40)));
    const BattleOrderRule pRule(HostVersion(HostVersion::PHost, MKVERSION(3,  4,  7)));

    {
        Planet pl;
        pl.setFriendlyCode("200");
        pl.setDefense(0);
        TS_ASSERT_EQUALS(tRule.get(pl), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(pl), 200);

        const game::sim::Object& obj = pl;
        TS_ASSERT_EQUALS(tRule.get(obj), BattleOrderRule::UNKNOWN);
        TS_ASSERT_EQUALS(pRule.get(obj), 200);
    }
}

