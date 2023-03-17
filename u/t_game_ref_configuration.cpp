/**
  *  \file u/t_game_ref_configuration.cpp
  *  \brief Test for game::ref::Configuration
  */

#include "game/ref/configuration.hpp"

#include "t_game_ref.hpp"
#include "afl/base/deleter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/ref/sortpredicate.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"

using game::Reference;
using game::config::UserConfiguration;
using game::map::Point;
using game::ref::SortPredicate;

namespace {
    /*
     *  Test environment and utilities
     *
     *  These are a subset of the tests for SortBy.
     *  Likewise, the tests largely re-use setups from SortBy.
     */

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { }
    };

    game::Root& addRoot(Environment& env)
    {
        if (env.session.getRoot().get() == 0) {
            env.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
        }
        return *env.session.getRoot();
    }

    game::Game& addGame(Environment& env)
    {
        if (env.session.getGame().get() == 0) {
            env.session.setGame(new game::Game());
        }
        return *env.session.getGame();
    }

    game::spec::ShipList& addShipList(Environment& env)
    {
        if (env.session.getShipList().get() == 0) {
            env.session.setShipList(new game::spec::ShipList());
        }
        return *env.session.getShipList();
    }

    game::map::Planet& addPlanet(Environment& env, int nr)
    {
        return *addGame(env).currentTurn().universe().planets().create(nr);
    }

    game::map::Ship& addPlayedShip(Environment& env, int nr, int owner, Point pos)
    {
        game::map::Ship& sh = *addGame(env).currentTurn().universe().ships().create(nr);
        game::map::ShipData sd;
        sd.x = pos.getX();
        sd.y = pos.getY();
        sd.owner = owner;
        sh.addCurrentShipData(sd, game::PlayerSet_t(owner));
        sh.internalCheck(game::PlayerSet_t(owner), 15);
        sh.setPlayability(game::map::Object::Playable);
        return sh;
    }

    game::spec::Hull& addHull(Environment& env, int nr)
    {
        return *addShipList(env).hulls().create(nr);
    }

    game::map::Ship& addScannedShip(Environment& env, int id, int owner, int x, int y, int mass)
    {
        game::map::Ship& sh = *addGame(env).currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(x, y), owner, mass, game::PlayerSet_t(1));
        return sh;
    }
}

/** Test transfer to/from preferences (UserConfiguration). */
void
TestGameRefConfiguration::testPreferences()
{
    // Environment
    Environment env;
    UserConfiguration& config = addRoot(env).userConfiguration();
    config[UserConfiguration::Sort_Cargo].set(3);
    config[UserConfiguration::Sort_Cargo_Secondary].set(5);

    // Fetch
    game::ref::Configuration testee;
    fetchConfiguration(env.session, game::ref::CARGO_TRANSFER, testee);

    // Check
    TS_ASSERT_EQUALS(testee.order.first, 3);
    TS_ASSERT_EQUALS(testee.order.second, 5);

    // Update
    testee.order.first = 1;
    testee.order.second = 9;
    storeConfiguration(env.session, game::ref::CARGO_TRANSFER, testee);

    // Verify
    TS_ASSERT_EQUALS(config[UserConfiguration::Sort_Cargo](), 1);
    TS_ASSERT_EQUALS(config[UserConfiguration::Sort_Cargo_Secondary](), 9);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by ID
    (which is actually "do not sort at all" / NullPredicate). */
void
TestGameRefConfiguration::testCreatePredicateSortById()
{
    Environment env;
    afl::base::Deleter del;

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortById, env.session, del);
    TS_ASSERT_EQUALS(p.compare(Reference(Reference::Ship, 10), Reference(Reference::Ship, 20)), 0);
    TS_ASSERT_EQUALS(p.compare(Reference(Reference::Planet, 10), Reference(Reference::Ship, 20)), 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by owner. */
void
TestGameRefConfiguration::testCreatePredicateSortByOwner()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addPlanet(env, 10);
    addPlanet(env, 20).setOwner(3);
    addPlanet(env, 30).setOwner(1);
    addRoot(env);                                  // required to access potential names

    const Reference r10(Reference::Planet, 10);    // owner 0
    const Reference r20(Reference::Planet, 20);    // owner Bird
    const Reference r30(Reference::Planet, 30);    // owner Fed

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByOwner, env.session, del);
    TS_ASSERT(p.compare(r10, r20) < 0);
    TS_ASSERT(p.compare(r20, r30) > 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by hull. */
void
TestGameRefConfiguration::testCreatePredicateSortByHull()
{
    Environment env;
    afl::base::Deleter del;

    // Hull definitions, required to access potential names
    game::test::addOutrider(addShipList(env));
    game::test::addAnnihilation(addShipList(env));

    // Objects
    addPlayedShip(env, 1, 1, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);
    addPlayedShip(env, 2, 1, Point(1000, 1000)).setHull(game::test::OUTRIDER_HULL_ID);
    addPlayedShip(env, 3, 1, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByHull, env.session, del);
    TS_ASSERT(p.compare(r1, r1) == 0);
    TS_ASSERT(p.compare(r1, r2) > 0);
    TS_ASSERT(p.compare(r1, r3) == 0);
    TS_ASSERT(p.compare(r2, r3) < 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by mass. */
void
TestGameRefConfiguration::testCreatePredicateSortByMass()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addScannedShip(env, 10, 3, 2000, 2100, 200);
    addScannedShip(env, 20, 5, 2000, 2200, 400);
    addScannedShip(env, 30, 4, 2000, 2100, 400);
    addShipList(env);                              // required to compute masses of played ships (not used here)

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByMass, env.session, del);
    TS_ASSERT(p.compare(r10, r20) < 0);
    TS_ASSERT(p.compare(r20, r10) > 0);
    TS_ASSERT(p.compare(r20, r30) == 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by fleet. */
void
TestGameRefConfiguration::testCreatePredicateSortByFleet()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addPlayedShip(env, 10, 1, Point(1000, 1000)).setFleetNumber(20);
    addPlayedShip(env, 20, 1, Point(1000, 1000)).setFleetNumber(20);
    addPlayedShip(env, 30, 1, Point(1000, 1000));
    addPlayedShip(env, 40, 1, Point(1000, 1000)).setFleetNumber(20);

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference r40(Reference::Ship, 40);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByFleet, env.session, del);
    TS_ASSERT(p.compare(r10, r20) > 0);   // member after leader
    TS_ASSERT(p.compare(r20, r30) > 0);   // fleet after not-fleet
    TS_ASSERT(p.compare(r30, r40) < 0);
    TS_ASSERT(p.compare(r40, r10) == 0);  // members are equal
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by tow group. */
void
TestGameRefConfiguration::testCreatePredicateSortByTowGroup()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addPlayedShip(env, 10, 1, Point(1000, 1000));
    addPlayedShip(env, 20, 1, Point(1000, 1000)).setMission(game::spec::Mission::msn_Tow, 0, 30);
    addPlayedShip(env, 30, 1, Point(1000, 1000));
    addPlayedShip(env, 40, 1, Point(1000, 1000));

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference r40(Reference::Ship, 40);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByTowGroup, env.session, del);
    TS_ASSERT(p.compare(r10, r20) < 0);        // not towed before tow group
    TS_ASSERT(p.compare(r20, r30) < 0);        // tower before towee
    TS_ASSERT(p.compare(r30, r40) > 0);        // towee after not towed
    TS_ASSERT(p.compare(r40, r10) == 0);       // not towed equal
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by battle order. */
void
TestGameRefConfiguration::testCreatePredicateSortByBattleOrder()
{
    Environment env;
    afl::base::Deleter del;

    // Use fixed host version
    addRoot(env).hostVersion() = game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0));

    // Objects
    game::map::Ship& sh1 = addPlayedShip(env, 1, 1, Point(1000, 1000));
    sh1.setFriendlyCode(String_t("200"));
    sh1.setCargo(game::Element::Neutronium, 1);
    game::map::Ship& sh2 = addPlayedShip(env, 2, 1, Point(1000, 1000));
    sh2.setFriendlyCode(String_t("250"));
    sh2.setCargo(game::Element::Neutronium, 1);
    game::map::Ship& sh3 = addPlayedShip(env, 3, 1, Point(1000, 1000));
    sh3.setFriendlyCode(String_t("150"));
    sh3.setCargo(game::Element::Neutronium, 1);
    game::map::Ship& sh4 = addPlayedShip(env, 4, 1, Point(1000, 1000));
    sh4.setFriendlyCode(String_t("-50"));
    sh4.setCargo(game::Element::Neutronium, 1);
    game::map::Ship& sh5 = addPlayedShip(env, 5, 1, Point(1000, 1000));
    sh5.setFriendlyCode(String_t("abc"));
    sh5.setCargo(game::Element::Neutronium, 1);

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);
    const Reference r4(Reference::Ship, 4);
    const Reference r5(Reference::Ship, 5);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByBattleOrder, env.session, del);
    TS_ASSERT(p.compare(r1, r2) < 0);
    TS_ASSERT(p.compare(r2, r3) > 0);
    TS_ASSERT(p.compare(r3, r4) > 0);
    TS_ASSERT(p.compare(r4, r5) < 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by position. */
void
TestGameRefConfiguration::testCreatePredicateSortByPosition()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addPlanet(env, 10).setPosition(Point(1000, 2000));
    addPlanet(env, 20).setPosition(Point(1000, 1500));

    const Reference r10(Reference::Planet, 10);
    const Reference r20(Reference::Planet, 20);
    const Reference rPos(Point(1000, 2000));

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByPosition, env.session, del);
    TS_ASSERT(p.compare(r10, r20) > 0);
    TS_ASSERT(p.compare(r10, rPos) == 0);
    TS_ASSERT(p.compare(r20, rPos) < 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by hull mass. */
void
TestGameRefConfiguration::testCreatePredicateSortByHullMass()
{
    Environment env;
    afl::base::Deleter del;

    // Hulls
    addHull(env, 30).setMass(100);
    addHull(env, 40).setMass(70);
    addHull(env, 50).setMass(200);

    // Objects
    addPlayedShip(env, 1, 1, Point(1000, 1000)).setHull(30);
    addPlayedShip(env, 2, 1, Point(1000, 1000)).setHull(40);
    addPlayedShip(env, 3, 1, Point(1000, 1000)).setHull(50);

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);
    const Reference rHull(Reference::Hull, 40);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByHullMass, env.session, del);
    TS_ASSERT(p.compare(r1, r2) > 0);       // 100 > 70
    TS_ASSERT(p.compare(r1, r1) == 0);
    TS_ASSERT(p.compare(r2, r3) < 0);       // 70 < 200
    TS_ASSERT(p.compare(rHull, r1) < 0);    // 70 < 100
    TS_ASSERT(p.compare(rHull, r2) == 0);   // same
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by damage. */
void
TestGameRefConfiguration::testCreatePredicateSortByDamage()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addPlayedShip(env, 10, 1, Point(1000, 1000)).setDamage(5);
    addPlayedShip(env, 20, 1, Point(1000, 1000)).setDamage(0);
    addPlayedShip(env, 30, 1, Point(1000, 1000)).setDamage(50);

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByDamage, env.session, del);
    TS_ASSERT(p.compare(r10, r20) > 0);
    TS_ASSERT(p.compare(r20, r30) < 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by name. */
void
TestGameRefConfiguration::testCreatePredicateSortByName()
{
    Environment env;
    afl::base::Deleter del;

    // Objects
    addScannedShip(env, 10, 1, 2000, 2100, 400).setName("zehn");
    addScannedShip(env, 20, 1, 2000, 2100, 400).setName("zwanzig");
    addScannedShip(env, 30, 1, 2000, 2100, 400).setName("dreissig");

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByName, env.session, del);
    TS_ASSERT(p.compare(r10, r20) < 0);
    TS_ASSERT(p.compare(r10, r30) > 0);
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by next position. */
void
TestGameRefConfiguration::testCreatePredicateSortByNextPosition()
{
    const int HULL_NR = 7;
    Environment env;
    afl::base::Deleter del;

    // Config/Spec
    addRoot(env);
    addHull(env, HULL_NR).setMass(100);
    game::test::addTranswarp(addShipList(env));

    // Objects
    game::map::Ship& s1 = addPlayedShip(env, 10, 1, Point(1000, 1000));
    s1.setHull(HULL_NR);
    s1.setWaypoint(Point(1000, 1020));
    s1.setWarpFactor(9);

    game::map::Ship& s2 = addPlayedShip(env, 20, 1, Point(1000, 1010));
    s2.setHull(HULL_NR);
    s2.setWaypoint(Point(1000, 1000));
    s2.setWarpFactor(9);

    addPlanet(env, 77).setPosition(Point(1000, 1000));

    const Reference r1(Reference::Ship, 10);
    const Reference r2(Reference::Ship, 20);
    const Reference rPlanet(Reference::Planet, 77);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByNextPosition, env.session, del);
    TS_ASSERT(p.compare(r1, r2) > 0);       // 1000,1020 > 1000,1000
    TS_ASSERT(p.compare(r2, rPlanet) == 0); // 1000,1000 = 1000,1000
}

/** Test createSortPredicate(), single-predicate (int parameter) version, sort by transfer target. */
void
TestGameRefConfiguration::testCreatePredicateSortByTransferTarget()
{
    Environment env;
    afl::base::Deleter del;

    // Use fixed host version
    addRoot(env).hostVersion() = game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0));

    // Objects
    /*game::map::Ship& sh1 =*/ addPlayedShip(env, 10, 1, Point(1000, 1000));  // no transfer
    /*game::map::Ship& sh2 =*/ addPlayedShip(env, 20, 1, Point(1000, 1000));  // transfer target
    game::map::Ship& sh3 = addPlayedShip(env, 30, 1, Point(1000, 1000));  // transfer to #20
    sh3.setTransporterTargetId(game::map::Ship::TransferTransporter, 20);
    sh3.setTransporterCargo(game::map::Ship::TransferTransporter, game::Element::Neutronium, 1);

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference r40(Reference::Ship, 40);
    const Reference r50(Reference::Ship, 50);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::ConfigSortByTransferTarget, env.session, del);
    TS_ASSERT(p.compare(r10, r20) < 0);          // unrelated, but sorted by Id
    TS_ASSERT(p.compare(r20, r30) < 0);          // 30 is unrelated, we're not looking at this transporter, thus sorted by Id
}

/** Test createSortPredicate(), complex-predicate version from configuration. */
void
TestGameRefConfiguration::testCreatePredicateFromConfig()
{
    // Environment
    Environment env;
    afl::base::Deleter del;
    game::test::addOutrider(addShipList(env));
    game::test::addAnnihilation(addShipList(env));

    // Configuration
    UserConfiguration& config = addRoot(env).userConfiguration();
    config[UserConfiguration::Sort_Cargo].set(game::ref::ConfigSortByOwner);
    config[UserConfiguration::Sort_Cargo_Secondary].set(game::ref::ConfigSortByHull);

    // Objects
    addPlayedShip(env, 1, 1, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);
    addPlayedShip(env, 2, 1, Point(1000, 1000)).setHull(game::test::OUTRIDER_HULL_ID);
    addPlayedShip(env, 3, 4, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);
    addPlayedShip(env, 4, 1, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);
    const Reference r4(Reference::Ship, 4);

    // Test
    const SortPredicate& p = game::ref::createSortPredicate(game::ref::CARGO_TRANSFER, env.session, del);
    TS_ASSERT(p.compare(r1, r2) > 0);       // Outrider before Annihilation
    TS_ASSERT(p.compare(r2, r3) < 0);       // Fed before Klingon
    TS_ASSERT(p.compare(r3, r4) > 0);       // Klingon after Fed
    TS_ASSERT(p.compare(r2, r4) < 0);       // Outrider before Annihilation
}

/** Test createSortPredicate(), missing preconditions.
    Must safely produce null predicate. */
void
TestGameRefConfiguration::testBlank()
{
    Environment env;
    afl::base::Deleter del;

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);

    // Check a range of single predicates
    for (int i = 0; i < 100; ++i) {
        if (i != game::ref::ConfigSortById && i != game::ref::ConfigSortByName) {
            TS_ASSERT_EQUALS(game::ref::createSortPredicate(i, env.session, del).compare(r1, r2), 0);
        }
    }

    // Check configured predicate
    TS_ASSERT_EQUALS(game::ref::createSortPredicate(game::ref::CARGO_TRANSFER, env.session, del).compare(r1, r2), 0);
}
