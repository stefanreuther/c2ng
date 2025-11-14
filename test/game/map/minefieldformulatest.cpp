/**
  *  \file test/game/map/minefieldformulatest.cpp
  *  \brief Test for game::map::MinefieldFormula
  */

#include "game/map/minefieldformula.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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
        Ref<game::config::HostConfiguration> config;
        game::UnitScoreDefinitionList shipScores;
        game::spec::ShipList shipList;
        afl::string::NullTranslator tx;
        afl::sys::Log log;

        Environment()
            : univ(), mapConfig(), hostVersion(),
              config(HostConfiguration::create()),
              shipScores(), shipList(), tx(), log()
            { }
    };

    void addPlanet(Environment& env, int id, Point pt, int owner)
    {
        Planet* pl = env.univ.planets().create(id);
        pl->setPosition(pt);
        if (owner >= 0) {
            pl->setOwner(owner);
        }
        pl->internalCheck(env.mapConfig, game::PlayerSet_t(12), 15, env.tx, env.log);
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
AFL_TEST("game.map.MinefieldFormula:isMinefieldEndangered", a)
{
    Environment env;
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    a.check("isMinefieldEndangered", !isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, *env.config));
}

/** Test isMinefieldEndangered(), enemy ship.
    Minefield is endangered by ship. */
AFL_TEST("game.map.MinefieldFormula:isMinefieldEndangered:enemy", a)
{
    Environment env;
    addShip(env, 10, Point(1000, 1010), 2);                    // enemy ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    a.check("isMinefieldEndangered", isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, *env.config));
}

/** Test isMinefieldEndangered(), unowned planet.
    Minefield is endangered because planet may be hiding ships. */
AFL_TEST("game.map.MinefieldFormula:isMinefieldEndangered:unowned-planet", a)
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), -1);                 // unowned planet
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    a.check("isMinefieldEndangered", isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, *env.config));
}

/** Test isMinefieldEndangered(), unowned planet, own ship.
    Minefield is not endangered because our ship would see the enemy ships. */
AFL_TEST("game.map.MinefieldFormula:isMinefieldEndangered:unowned-planet-own-ship", a)
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), -1);                 // unowned planet
    addShip(env, 10, Point(1000, 1010), 1);                    // own ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    a.check("isMinefieldEndangered", !isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, *env.config));
}

/** Test isMinefieldEndangered(), unowned planet, own and enemy ship.
    Minefield is endangered by the ship orbiting the planet. */
AFL_TEST("game.map.MinefieldFormula:isMinefieldEndangered:unowned-planet-two-ships", a)
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), -1);                 // unowned planet
    addShip(env, 10, Point(1000, 1010), 1);                    // own ship
    addShip(env, 10, Point(1000, 1010), 2);                    // enemy ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    a.check("isMinefieldEndangered", isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, *env.config));
}

/** Test isMinefieldEndangered(), enemy planet, own ship.
    The planet itself does not endanger the minefield. */
AFL_TEST("game.map.MinefieldFormula:isMinefieldEndangered:enemy-planet-own-ship", a)
{
    Environment env;
    addPlanet(env, 33, Point(1000, 1010), 3);                  // enemy planet
    addShip(env, 10, Point(1000, 1010), 1);                    // own ship
    Minefield field(100, Point(1000, 1000), 1, false, 400);    // 20 ly
    a.check("isMinefieldEndangered", !isMinefieldEndangered(field, env.univ, env.mapConfig, env.hostVersion, *env.config));
}

/** Test computeMineLayEffect(), new minefield. */
AFL_TEST("game.map.MinefieldFormula:computeMineLayEffect", a)
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);

    // A ship that is laying mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineLayer(sh);
    a.check("01. checkLayMission", msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    // We are laying 9*9*64 = 5184 = 72**2 units
    a.checkEqual("11. size",         result.size(), 1U);
    a.checkEqual("12. center",       result[0].center, Point(1200, 1300));
    a.checkEqual("13. id",           result[0].id, 0);
    a.checkEqual("14. radiusChange", result[0].radiusChange, 72);
    a.checkEqual("15. newUnits",     result[0].newUnits, 5184);
    a.checkEqual("16. unitLimit",    result[0].unitLimit, 6400);     // 80**2
    a.checkEqual("17. owner",        result[0].owner, 1);
    a.checkEqual("18. numTorps",     result[0].numTorps, 64);
    a.checkEqual("19. isWeb",        result[0].isWeb, false);
    a.checkEqual("20. isEndangered", result[0].isEndangered, false);
}

/** Test computeMineLayEffect(), new minefield, with planet danger.
    Same as above, but with a ship that triggers "danger". */
AFL_TEST("game.map.MinefieldFormula:computeMineLayEffect:danger", a)
{
    Environment env;
    Ref<Root> root = game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(3,0,0)));
    root->hostConfiguration()[HostConfiguration::MaximumMinefieldRadius].set(80);

    // A ship that is laying mines
    MinefieldMission msn;
    Ship sh(4);
    configureMineLayer(sh);
    a.check("01. checkLayMission", msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Danger
    addShip(env, 99, Point(1200, 1310), 7);

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    a.checkEqual("11. isEndangered", result[0].isEndangered, true);
}

/** Test computeMineLayEffect(), existing minefield, THost. */
AFL_TEST("game.map.MinefieldFormula:computeMineLayEffect:existing:host", a)
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
    a.check("01. checkLayMission", msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    // We are laying 9*9*64 = 5184 = 72**2 units, +4816 = 10000
    a.checkEqual("11. size", result.size(), 1U);
    a.checkEqual("12. center", result[0].center, Point(1200, 1320));
    a.checkEqual("13. id", result[0].id, 20);
    a.checkEqual("14. radiusChange", result[0].radiusChange, 31);       // 69 + 31 = 100
    a.checkEqual("15. newUnits", result[0].newUnits, 9899);         // 10000 - MineDecayRate, Host
    a.checkEqual("16. unitLimit", result[0].unitLimit, 6400);        // 80**2
    a.checkEqual("17. owner", result[0].owner, 1);
    a.checkEqual("18. numTorps", result[0].numTorps, 64);
    a.checkEqual("19. isWeb", result[0].isWeb, false);
    a.checkEqual("20. isEndangered", result[0].isEndangered, false);
}

/** Test computeMineLayEffect(), existing minefield, THost.
    Same as above, but with different formulas for decay. */
AFL_TEST("game.map.MinefieldFormula:computeMineLayEffect:existing:phost", a)
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
    a.check("01. checkLayMission", msn.checkLayMission(sh, env.univ, *root, env.mapConfig, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineLayEffect(result, msn, sh, env.univ, env.mapConfig, *root);

    // Verify
    // We are laying 9*9*64 = 5184 = 72**2 units, +4816 = 10000
    a.checkEqual("11. size", result.size(), 1U);
    a.checkEqual("12. center", result[0].center, Point(1200, 1320));
    a.checkEqual("13. id", result[0].id, 20);
    a.checkEqual("14. radiusChange", result[0].radiusChange, 30);       // 70 + 30 = 99
    a.checkEqual("15. newUnits", result[0].newUnits, 9951);         // 10000 - MineDecayRate, PHost
    a.checkEqual("16. unitLimit", result[0].unitLimit, 6400);        // 80**2
    a.checkEqual("17. owner", result[0].owner, 1);
    a.checkEqual("18. numTorps", result[0].numTorps, 64);
    a.checkEqual("19. isWeb", result[0].isWeb, false);
    a.checkEqual("20. isEndangered", result[0].isEndangered, false);
}

/** Test computeMineScoopEffect(), base case. */
AFL_TEST("game.map.MinefieldFormula:computeMineScoopEffect", a)
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
    a.check("01. checkScoopMission", msn.checkScoopMission(sh, *root, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineScoopEffect(result, msn, sh, env.univ, env.mapConfig, *root, env.shipList);

    // Verify
    a.checkEqual("11. size",         result.size(), 2U);
    a.checkEqual("12. center",       result[0].center, Point(1200, 1320));
    a.checkEqual("13. id",           result[0].id, 20);
    a.checkEqual("14. radiusChange", result[0].radiusChange, -68);
    a.checkEqual("15. newUnits",     result[0].newUnits, 0);
    a.checkEqual("16. unitLimit",    result[0].unitLimit, 6400);
    a.checkEqual("17. owner",        result[0].owner, 1);
    a.checkEqual("18. numTorps",     result[0].numTorps, 58);
    a.checkEqual("19. isWeb",        result[0].isWeb, false);
    a.checkEqual("20. isEndangered", result[0].isEndangered, false);

    a.checkEqual("21. center",       result[1].center, Point(1200, 1280));
    a.checkEqual("22. id",           result[1].id, 40);
    a.checkEqual("23. radiusChange", result[1].radiusChange, -61);
    a.checkEqual("24. newUnits",     result[1].newUnits, 0);
    a.checkEqual("25. unitLimit",    result[1].unitLimit, 6400);
    a.checkEqual("26. owner",        result[1].owner, 1);
    a.checkEqual("27. numTorps",     result[1].numTorps, 46);
    a.checkEqual("28. isWeb",        result[1].isWeb, false);
    a.checkEqual("29. isEndangered", result[1].isEndangered, false);
}

/** Test computeMineScoopEffect(), mission limit.
    Will scoop the same amount from each affected field. */
AFL_TEST("game.map.MinefieldFormula:computeMineScoopEffect:mission-limit", a)
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
    a.check("01. checkScoopMission", msn.checkScoopMission(sh, *root, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineScoopEffect(result, msn, sh, env.univ, env.mapConfig, *root, env.shipList);

    // Verify
    a.checkEqual("11. size",         result.size(), 2U);
    a.checkEqual("12. center",       result[0].center, Point(1200, 1320));
    a.checkEqual("13. id",           result[0].id, 20);
    a.checkEqual("14. radiusChange", result[0].radiusChange, -9);
    a.checkEqual("15. newUnits",     result[0].newUnits, 3535);
    a.checkEqual("16. unitLimit",    result[0].unitLimit, 6400);
    a.checkEqual("17. owner",        result[0].owner, 1);
    a.checkEqual("18. numTorps",     result[0].numTorps, 15);
    a.checkEqual("19. isWeb",        result[0].isWeb, false);
    a.checkEqual("20. isEndangered", result[0].isEndangered, false);

    a.checkEqual("21. center",       result[1].center, Point(1200, 1280));
    a.checkEqual("22. id",           result[1].id, 40);
    a.checkEqual("23. radiusChange", result[1].radiusChange, -11);
    a.checkEqual("24. newUnits",     result[1].newUnits, 2585);
    a.checkEqual("25. unitLimit",    result[1].unitLimit, 6400);
    a.checkEqual("26. owner",        result[1].owner, 1);
    a.checkEqual("27. numTorps",     result[1].numTorps, 15);
    a.checkEqual("28. isWeb",        result[1].isWeb, false);
    a.checkEqual("29. isEndangered", result[1].isEndangered, false);
}

/** Test computeMineScoopEffect(), room limit.
    Will scoop until room is full. */
AFL_TEST("game.map.MinefieldFormula:computeMineScoopEffect:room-limit", a)
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
    a.check("01. checkScoopMission", msn.checkScoopMission(sh, *root, env.shipScores, env.shipList));

    // Test
    game::map::MinefieldEffects_t result;
    computeMineScoopEffect(result, msn, sh, env.univ, env.mapConfig, *root, env.shipList);

    // Verify
    a.checkEqual("11. size",         result.size(), 2U);
    a.checkEqual("12. center",       result[0].center, Point(1200, 1320));
    a.checkEqual("13. id",           result[0].id, 20);
    a.checkEqual("14. radiusChange", result[0].radiusChange, -68);
    a.checkEqual("15. newUnits",     result[0].newUnits, 0);
    a.checkEqual("16. unitLimit",    result[0].unitLimit, 6400);
    a.checkEqual("17. owner",        result[0].owner, 1);
    a.checkEqual("18. numTorps",     result[0].numTorps, 58);
    a.checkEqual("19. isWeb",        result[0].isWeb, false);
    a.checkEqual("20. isEndangered", result[0].isEndangered, false);

    a.checkEqual("21. center",       result[1].center, Point(1200, 1280));
    a.checkEqual("22. id",           result[1].id, 40);
    a.checkEqual("23. radiusChange", result[1].radiusChange, -8);
    a.checkEqual("24. newUnits",     result[1].newUnits, 2828);
    a.checkEqual("25. unitLimit",    result[1].unitLimit, 6400);
    a.checkEqual("26. owner",        result[1].owner, 1);
    a.checkEqual("27. numTorps",     result[1].numTorps, 12);    // 12 + 58 = 70
    a.checkEqual("28. isWeb",        result[1].isWeb, false);
    a.checkEqual("29. isEndangered", result[1].isEndangered, false);
}
