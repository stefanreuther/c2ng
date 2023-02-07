/**
  *  \file u/t_game_map_minefieldformula.cpp
  *  \brief Test for game::map::MinefieldFormula
  */

#include "game/map/minefieldformula.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/configuration.hpp"
#include "game/map/minefieldmission.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

using afl::base::Ref;
using game::HostVersion;
using game::PlayerSet_t;
using game::Root;
using game::config::HostConfiguration;
using game::map::Minefield;
using game::map::MinefieldMission;
using game::map::Planet;
using game::map::Point;
using game::map::Ship;

namespace {
    struct Environment {
        game::map::Universe univ;
        game::map::Configuration mapConfig;
        game::HostVersion hostVersion;
        game::config::HostConfiguration config;
        game::UnitScoreDefinitionList shipScores;
        game::spec::ShipList shipList;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
    };

    void addPlanet(Environment& env, int id, Point pt, int owner)
    {
        Planet* pl = env.univ.planets().create(id);
        pl->setPosition(pt);
        if (owner >= 0) {
            pl->setOwner(owner);
        }
        pl->internalCheck(env.mapConfig, env.tx, env.log);
    }

    void addShip(Environment& env, int id, Point pt, int owner)
    {
        env.univ.ships().create(id)->addShipXYData(pt, owner, 100, PlayerSet_t(owner));
    }

    void addMinefield(Environment& env, int id, Point pt, int owner, int units, const Root& root)
    {
        Minefield* mf = env.univ.minefields().create(id);
        mf->addReport(pt, owner, Minefield::IsMine, Minefield::UnitsKnown, units, 1, Minefield::MinefieldScanned);
        mf->internalCheck(1, root.hostVersion(), root.hostConfiguration());
    }

    void configureMineLayer(Ship& sh)
    {
        sh.setOwner(1);
        sh.setNumLaunchers(10);
        sh.setTorpedoType(9);
        sh.setAmmo(64);
        sh.setMission(game::spec::Mission::msn_LayMines, 0, 0);
        sh.setPosition(Point(1200, 1300));
    }

    void configureMineScooper(Ship& sh)
    {
        sh.setOwner(1);
        sh.setNumLaunchers(10);
        sh.setTorpedoType(9);
        sh.setBeamType(9);
        sh.setNumBeams(10);
        sh.setAmmo(0);
        sh.setPosition(Point(1200, 1300));
        sh.setHull(game::test::ANNIHILATION_HULL_ID);
        sh.setCargo(game::Element::Tritanium, 0);
        sh.setCargo(game::Element::Duranium, 0);
        sh.setCargo(game::Element::Molybdenum, 0);
        sh.setCargo(game::Element::Supplies, 0);
        sh.setCargo(game::Element::Colonists, 0);
        sh.setCargo(game::Element::Money, 0);
    }
}

/** Test isMinefieldEndangered(), base case.
    Minefield is not endangered. */
void
TestGameMapMinefieldFormula::testIsMinefieldEndangered()
{
    Environment env;
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    TS_ASSERT(!isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, env.config));
}

/** Test isMinefieldEndangered(), enemy ship.
    Minefield is endangered by ship. */
void
TestGameMapMinefieldFormula::testIsMinefieldEndangeredEnemyShip()
{
    Environment env;
    addShip(env, 10, Point(1000, 1010), 2);                    // enemy ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    TS_ASSERT(isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, env.config));
}

/** Test isMinefieldEndangered(), unowned planet.
    Minefield is endangered because planet may be hiding ships. */
void
TestGameMapMinefieldFormula::testIsMinefieldEndangeredUnownedPlanet()
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), -1);                 // unowned planet
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    TS_ASSERT(isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, env.config));
}

/** Test isMinefieldEndangered(), unowned planet, own ship.
    Minefield is not endangered because our ship would see the enemy ships. */
void
TestGameMapMinefieldFormula::testIsMinefieldEndangeredUnownedPlanetShip()
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), -1);                 // unowned planet
    addShip(env, 10, Point(1000, 1010), 1);                    // own ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    TS_ASSERT(!isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, env.config));
}

/** Test isMinefieldEndangered(), unowned planet, own and enemy ship.
    Minefield is endangered by the ship orbiting the planet. */
void
TestGameMapMinefieldFormula::testIsMinefieldEndangeredUnownedPlanet2Ships()
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), -1);                 // unowned planet
    addShip(env, 10, Point(1000, 1010), 1);                    // own ship
    addShip(env, 10, Point(1000, 1010), 2);                    // enemy ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    TS_ASSERT(isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, env.config));
}

/** Test isMinefieldEndangered(), enemy planet, own ship.
    The planet itself does not endanger the minefield. */
void
TestGameMapMinefieldFormula::testIsMinefieldEndangeredEnemyPlanet()
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), 3);                  // enemy planet
    addShip(env, 10, Point(1000, 1010), 1);                    // own ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    TS_ASSERT(!isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, env.config));
}

/** Test computeMineLayEffect(), new minefield. */
void
TestGameMapMinefieldFormula::testComputeMineLayEffectNew()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);

    // A ship that is laying mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineLayer(sh);
    TS_ASSERT(msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    // We are laying 9*9*64 = 5184 = 72**2 units
    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT_EQUALS(result[0].center, Point(1200, 1300));
    TS_ASSERT_EQUALS(result[0].id, 0);
    TS_ASSERT_EQUALS(result[0].radiusChange, 72);
    TS_ASSERT_EQUALS(result[0].newUnits, 5184);
    TS_ASSERT_EQUALS(result[0].unitLimit, 6400);     // 80**2
    TS_ASSERT_EQUALS(result[0].owner, 1);
    TS_ASSERT_EQUALS(result[0].numTorps, 64);
    TS_ASSERT_EQUALS(result[0].isWeb, false);
    TS_ASSERT_EQUALS(result[0].isEndangered, false);
}

/** Test computeMineLayEffect(), new minefield, with planet danger.
    Same as above, but with a ship that triggers "danger". */
void
TestGameMapMinefieldFormula::testComputeMineLayEffectNewDanger()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);

    // A ship that is laying mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineLayer(sh);
    TS_ASSERT(msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Danger
    addShip(env, 99, Point(1200, 1310), 7);

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    TS_ASSERT_EQUALS(result[0].isEndangered, true);
}

/** Test computeMineLayEffect(), existing minefield, THost. */
void
TestGameMapMinefieldFormula::testComputeMineLayEffectExisting()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::Host, MKVERSION(3,0,0)));
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);
    root->hostConfiguration()[HostConfiguration::MineDecayRate].set(1);

    // An existing minefield
    addMinefield(env, 20, Point(1200, 1320), 1, 4816, *root);

    // A ship that is laying mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineLayer(sh);
    TS_ASSERT(msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    // We are laying 9*9*64 = 5184 = 72**2 units, +4816 = 10000
    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT_EQUALS(result[0].center, Point(1200, 1320));
    TS_ASSERT_EQUALS(result[0].id, 20);
    TS_ASSERT_EQUALS(result[0].radiusChange, 31);       // 69 + 31 = 100
    TS_ASSERT_EQUALS(result[0].newUnits, 9899);         // 10000 - MineDecayRate, Host
    TS_ASSERT_EQUALS(result[0].unitLimit, 6400);        // 80**2
    TS_ASSERT_EQUALS(result[0].owner, 1);
    TS_ASSERT_EQUALS(result[0].numTorps, 64);
    TS_ASSERT_EQUALS(result[0].isWeb, false);
    TS_ASSERT_EQUALS(result[0].isEndangered, false);
}

/** Test computeMineLayEffect(), existing minefield, THost.
    Same as above, but with different formulas for decay. */
void
TestGameMapMinefieldFormula::testComputeMineLayEffectExistingPHost()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);
    root->hostConfiguration()[HostConfiguration::MineDecayRate].set(1);

    // An existing minefield
    addMinefield(env, 20, Point(1200, 1320), 1, 4816, *root);

    // A ship that is laying mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineLayer(sh);
    TS_ASSERT(msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    // We are laying 9*9*64 = 5184 = 72**2 units, +4816 = 10000
    TS_ASSERT_EQUALS(result.size(), 1U);
    TS_ASSERT_EQUALS(result[0].center, Point(1200, 1320));
    TS_ASSERT_EQUALS(result[0].id, 20);
    TS_ASSERT_EQUALS(result[0].radiusChange, 30);       // 70 + 30 = 99
    TS_ASSERT_EQUALS(result[0].newUnits, 9951);         // 10000 - MineDecayRate, PHost
    TS_ASSERT_EQUALS(result[0].unitLimit, 6400);        // 80**2
    TS_ASSERT_EQUALS(result[0].owner, 1);
    TS_ASSERT_EQUALS(result[0].numTorps, 64);
    TS_ASSERT_EQUALS(result[0].isWeb, false);
    TS_ASSERT_EQUALS(result[0].isEndangered, false);
}

/** Test computeMineScoopEffect(), base case. */
void
TestGameMapMinefieldFormula::testComputeMineScoopEffect()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Registered);
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);
    game::test::addAnnihilation(env.shipList);

    // Some minefields
    addMinefield(env, 20, Point(1200, 1320), 1, 5000, *root);
    addMinefield(env, 30, Point(1220, 1300), 2, 5000, *root);    // wrong owner
    addMinefield(env, 40, Point(1200, 1280), 1, 4000, *root);

    // A ship that is scooping mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineScooper(sh);
    sh.setMission(game::spec::Mission::msn_MineSweep, 0, 0);
    sh.setFriendlyCode(String_t("msc"));
    TS_ASSERT(msn.checkScoopMission(sh, *root, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineScoopEffect(result, msn, sh, env.univ, env.mapConfig, *root, env.shipList);

    // Verify
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(result[0].center, Point(1200, 1320));
    TS_ASSERT_EQUALS(result[0].id, 20);
    TS_ASSERT_EQUALS(result[0].radiusChange, -68);
    TS_ASSERT_EQUALS(result[0].newUnits, 0);
    TS_ASSERT_EQUALS(result[0].unitLimit, 6400);
    TS_ASSERT_EQUALS(result[0].owner, 1);
    TS_ASSERT_EQUALS(result[0].numTorps, 58);
    TS_ASSERT_EQUALS(result[0].isWeb, false);
    TS_ASSERT_EQUALS(result[0].isEndangered, false);

    TS_ASSERT_EQUALS(result[1].center, Point(1200, 1280));
    TS_ASSERT_EQUALS(result[1].id, 40);
    TS_ASSERT_EQUALS(result[1].radiusChange, -61);
    TS_ASSERT_EQUALS(result[1].newUnits, 0);
    TS_ASSERT_EQUALS(result[1].unitLimit, 6400);
    TS_ASSERT_EQUALS(result[1].owner, 1);
    TS_ASSERT_EQUALS(result[1].numTorps, 46);
    TS_ASSERT_EQUALS(result[1].isWeb, false);
    TS_ASSERT_EQUALS(result[1].isEndangered, false);
}

/** Test computeMineScoopEffect(), mission limit.
    Will scoop the same amount from each affected field. */
void
TestGameMapMinefieldFormula::testComputeMineScoopEffectMissionLimit()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Registered);
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);
    root->hostConfiguration()[HostConfiguration::ExtMissionsStartAt].set(50);
    game::test::addAnnihilation(env.shipList);

    // One minefield
    addMinefield(env, 20, Point(1200, 1320), 1, 5000, *root);
    addMinefield(env, 40, Point(1200, 1280), 1, 4000, *root);

    // A ship that is scooping mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineScooper(sh);
    sh.setMission(game::spec::Mission::pmsn_ScoopTorps + 50, 15, 0);
    sh.setFriendlyCode(String_t("abc"));
    TS_ASSERT(msn.checkScoopMission(sh, *root, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineScoopEffect(result, msn, sh, env.univ, env.mapConfig, *root, env.shipList);

    // Verify
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(result[0].center, Point(1200, 1320));
    TS_ASSERT_EQUALS(result[0].id, 20);
    TS_ASSERT_EQUALS(result[0].radiusChange, -9);
    TS_ASSERT_EQUALS(result[0].newUnits, 3535);
    TS_ASSERT_EQUALS(result[0].unitLimit, 6400);
    TS_ASSERT_EQUALS(result[0].owner, 1);
    TS_ASSERT_EQUALS(result[0].numTorps, 15);
    TS_ASSERT_EQUALS(result[0].isWeb, false);
    TS_ASSERT_EQUALS(result[0].isEndangered, false);

    TS_ASSERT_EQUALS(result[1].center, Point(1200, 1280));
    TS_ASSERT_EQUALS(result[1].id, 40);
    TS_ASSERT_EQUALS(result[1].radiusChange, -11);
    TS_ASSERT_EQUALS(result[1].newUnits, 2585);
    TS_ASSERT_EQUALS(result[1].unitLimit, 6400);
    TS_ASSERT_EQUALS(result[1].owner, 1);
    TS_ASSERT_EQUALS(result[1].numTorps, 15);
    TS_ASSERT_EQUALS(result[1].isWeb, false);
    TS_ASSERT_EQUALS(result[1].isEndangered, false);
}

/** Test computeMineScoopEffect(), room limit.
    Will scoop until room is full. */
void
TestGameMapMinefieldFormula::testComputeMineScoopEffectRoomLimit()
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)), game::RegistrationKey::Registered);
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);
    root->hostConfiguration()[HostConfiguration::ExtMissionsStartAt].set(50);
    game::test::addAnnihilation(env.shipList);

    // One minefield
    addMinefield(env, 20, Point(1200, 1320), 1, 5000, *root);
    addMinefield(env, 40, Point(1200, 1280), 1, 4000, *root);
    addMinefield(env, 50, Point(1210, 1280), 1, 4000, *root);     // This field does not affect the result, room is full before

    // A ship that is scooping mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineScooper(sh);
    sh.setMission(game::spec::Mission::msn_MineSweep, 0, 0);
    sh.setFriendlyCode(String_t("msc"));
    sh.setCargo(game::Element::Colonists, 250);      // ship has 320 total, leaving 70 free
    TS_ASSERT(msn.checkScoopMission(sh, *root, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineScoopEffect(result, msn, sh, env.univ, env.mapConfig, *root, env.shipList);

    // Verify
    TS_ASSERT_EQUALS(result.size(), 2U);
    TS_ASSERT_EQUALS(result[0].center, Point(1200, 1320));
    TS_ASSERT_EQUALS(result[0].id, 20);
    TS_ASSERT_EQUALS(result[0].radiusChange, -68);
    TS_ASSERT_EQUALS(result[0].newUnits, 0);
    TS_ASSERT_EQUALS(result[0].unitLimit, 6400);
    TS_ASSERT_EQUALS(result[0].owner, 1);
    TS_ASSERT_EQUALS(result[0].numTorps, 58);
    TS_ASSERT_EQUALS(result[0].isWeb, false);
    TS_ASSERT_EQUALS(result[0].isEndangered, false);

    TS_ASSERT_EQUALS(result[1].center, Point(1200, 1280));
    TS_ASSERT_EQUALS(result[1].id, 40);
    TS_ASSERT_EQUALS(result[1].radiusChange, -8);
    TS_ASSERT_EQUALS(result[1].newUnits, 2828);
    TS_ASSERT_EQUALS(result[1].unitLimit, 6400);
    TS_ASSERT_EQUALS(result[1].owner, 1);
    TS_ASSERT_EQUALS(result[1].numTorps, 12);    // 12 + 58 = 70
    TS_ASSERT_EQUALS(result[1].isWeb, false);
    TS_ASSERT_EQUALS(result[1].isEndangered, false);
}

