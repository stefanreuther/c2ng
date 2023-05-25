/**
  *  \file u/t_game_ref_sortby.cpp
  *  \brief Test for game::ref::SortBy
  */

#include "game/ref/sortby.hpp"

#include "t_game_ref.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/spec/hull.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"

using game::Reference;
using game::map::Point;

namespace {
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

    game::Player& addPlayer(Environment& env, int nr)
    {
        return *addRoot(env).playerList().create(nr);
    }

    game::map::Planet& addPlanet(Environment& env, int nr)
    {
        return *addGame(env).currentTurn().universe().planets().create(nr);
    }

    game::map::Ship& addShip(Environment& env, int nr)
    {
        return *addGame(env).currentTurn().universe().ships().create(nr);
    }

    game::map::Ship& addPlayedShip(Environment& env, int nr, int owner, Point pos)
    {
        game::map::Ship& sh = addShip(env, nr);
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

    game::spec::Beam& addBeam(Environment& env, int nr)
    {
        return *addShipList(env).beams().create(nr);
    }

    void clearShipCargo(game::map::Ship& sh)
    {
        sh.setCargo(game::Element::Neutronium, 0);
        sh.setCargo(game::Element::Tritanium, 0);
        sh.setCargo(game::Element::Duranium, 0);
        sh.setCargo(game::Element::Molybdenum, 0);
        sh.setCargo(game::Element::Colonists, 0);
        sh.setCargo(game::Element::Supplies, 0);
        sh.setCargo(game::Element::Money, 0);
        sh.setAmmo(0);
        sh.setBeamType(0);
        sh.setNumBeams(0);
        sh.setTorpedoType(0);
        sh.setNumLaunchers(0);
        sh.setNumBays(0);
    }
}


/** Test game::ref::SortBy::Id. */
void
TestGameRefSortBy::testId()
{
    Reference s1(Reference::Ship, 1);
    Reference s2(Reference::Ship, 2);
    Reference p1(Reference::Planet, 1);

    game::ref::SortBy::Id t;
    TS_ASSERT(t.compare(s1, s1) == 0);
    TS_ASSERT(t.compare(s1, s2) < 0);
    TS_ASSERT(t.compare(s2, s1) > 0);
    TS_ASSERT(t.compare(p1, s2) < 0);
    TS_ASSERT(t.compare(p1, s1) == 0);

    TS_ASSERT_EQUALS(t.getClass(s1), "");
    TS_ASSERT_EQUALS(t.getClass(p1), "");
}

/** Test game::ref::SortBy::Name. */
void
TestGameRefSortBy::testName()
{
    Environment env;

    addPlayer(env, 3).setName(game::Player::ShortName, "N2");
    addPlanet(env, 99).setName("N1");
    addHull(env, 30).setName("N3");

    const Reference r2(Reference::Player, 3);
    const Reference r1(Reference::Planet, 99);
    const Reference r3(Reference::Hull, 30);

    // Testee
    game::ref::SortBy::Name t(env.session);

    // Verify comparison
    TS_ASSERT(t.compare(r1, r1) == 0);
    TS_ASSERT(t.compare(r1, r2) < 0);
    TS_ASSERT(t.compare(r2, r3) < 0);
    TS_ASSERT(t.compare(r1, r1) == 0);

    Reference rBadHull(Reference::Hull, 777);    // stringifies as 'Hull #777' which goes before N1
    Reference rBadPlanet(Reference::Planet, 777);  // stringifies as 'Planet #777' which goes after N1
    TS_ASSERT(t.compare(rBadHull, r1) < 0);
    TS_ASSERT(t.compare(rBadPlanet, r1) > 0);

    // Verify class name
    TS_ASSERT_EQUALS(t.getClass(r1), "");
}

/** Test game::ref::SortBy::Owner. */
void
TestGameRefSortBy::testOwner()
{
    Environment env;
    addPlayer(env, 1).setName(game::Player::ShortName, "Fed");
    addPlayer(env, 3).setName(game::Player::ShortName, "Bird");

    addPlanet(env, 10);
    addPlanet(env, 20).setOwner(3);
    addPlanet(env, 30).setOwner(1);

    const Reference r10(Reference::Planet, 10);    // owner 0
    const Reference r20(Reference::Planet, 20);    // owner Bird
    const Reference r30(Reference::Planet, 30);    // owner Fed
    const Reference rHull(Reference::Hull, 7);     // no owner
    const Reference rPlayer(Reference::Player, 3); // owner Bird, trivially

    game::ref::SortBy::Owner t(addGame(env).currentTurn().universe(), addRoot(env).playerList(), env.tx);

    // Verify comparison
    TS_ASSERT(t.compare(r10, r20) < 0);
    TS_ASSERT(t.compare(r20, r30) > 0);
    TS_ASSERT(t.compare(r30, rHull) > 0);
    TS_ASSERT(t.compare(r20, rPlayer) == 0);

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r10), "Nobody");
    TS_ASSERT_EQUALS(t.getClass(r20), "Bird");
    TS_ASSERT_EQUALS(t.getClass(r30), "Fed");
    TS_ASSERT_EQUALS(t.getClass(rHull), "Nobody");
    TS_ASSERT_EQUALS(t.getClass(rPlayer), "Bird");
}

/** Test game::ref::SortBy::Position. */
void
TestGameRefSortBy::testPosition()
{
    Environment env;
    addPlanet(env, 10).setPosition(Point(1000, 2000));
    addPlanet(env, 20).setPosition(Point(1000, 1500));

    const Reference r10(Reference::Planet, 10);
    const Reference r20(Reference::Planet, 20);
    const Reference rPos(Point(1000, 2000));
    const Reference rHull(Reference::Hull, 3);

    game::ref::SortBy::Position t(addGame(env).currentTurn().universe(), env.tx);

    // Verify compare
    TS_ASSERT(t.compare(r10, r20) > 0);
    TS_ASSERT(t.compare(r10, rPos) == 0);
    TS_ASSERT(t.compare(r20, rPos) < 0);
    TS_ASSERT(t.compare(r10, rHull) > 0);

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r10), "(1000,2000)");
    TS_ASSERT_EQUALS(t.getClass(r20), "(1000,1500)");
    TS_ASSERT_EQUALS(t.getClass(rHull), "not on map");
}

/** Test game::ref::SortBy::NextPosition. */
void
TestGameRefSortBy::testNextPosition()
{
    const int HULL_NR = 7;
    Environment env;
    addHull(env, HULL_NR).setMass(100);
    game::test::addTranswarp(addShipList(env));

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
    const Reference rHull(Reference::Hull, HULL_NR);

    game::ref::SortBy::NextPosition t(addGame(env).currentTurn().universe(), addGame(env), addShipList(env), addRoot(env), env.tx);

    // Verify compare
    TS_ASSERT(t.compare(r1, r2) > 0);       // 1000,1020 > 1000,1000
    TS_ASSERT(t.compare(r2, rPlanet) == 0); // 1000,1000 = 1000,1000
    TS_ASSERT(t.compare(r1, rHull) > 0);    // 1000,1020 > not no map

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r1), "(1000,1020)");
    TS_ASSERT_EQUALS(t.getClass(r2), "(1000,1000)");
    TS_ASSERT_EQUALS(t.getClass(rPlanet), "(1000,1000)");
    TS_ASSERT_EQUALS(t.getClass(rHull), "not on map");
}

/** Test game::ref::SortBy::Damage. */
void
TestGameRefSortBy::testDamage()
{
    Environment env;
    addPlayedShip(env, 10, 1, Point(1000, 1000)).setDamage(5);
    addPlayedShip(env, 20, 1, Point(1000, 1000)).setDamage(0);
    addPlayedShip(env, 30, 1, Point(1000, 1000)).setDamage(50);
    addHull(env, 33);

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference rHull(Reference::Hull, 33);

    game::ref::SortBy::Damage t(addGame(env).currentTurn().universe());

    // Verify compare
    TS_ASSERT(t.compare(r10, r20) > 0);
    TS_ASSERT(t.compare(r20, r30) < 0);
    TS_ASSERT(t.compare(r30, rHull) > 0);
    TS_ASSERT(t.compare(r20, rHull) == 0);

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r10), "");
    TS_ASSERT_EQUALS(t.getClass(rHull), "");
}

/** Test game::ref::SortBy::Mass. */
void
TestGameRefSortBy::testMass()
{
    Environment env;
    game::test::initStandardBeams(addShipList(env));
    game::test::initStandardTorpedoes(addShipList(env));
    game::test::addTranswarp(addShipList(env));
    game::test::addOutrider(addShipList(env));

    game::map::Ship& sh1 = addPlayedShip(env, 1, 1, Point(1000, 1000));
    sh1.setHull(game::test::OUTRIDER_HULL_ID);
    sh1.setEngineType(game::test::TRANSWARP_ENGINE_ID);
    clearShipCargo(sh1);
    sh1.setCargo(game::Element::Neutronium, 100);

    game::map::Ship& sh2 = addPlayedShip(env, 2, 1, Point(1000, 1000));
    sh2.setHull(game::test::OUTRIDER_HULL_ID);
    sh2.setEngineType(game::test::TRANSWARP_ENGINE_ID);
    clearShipCargo(sh2);
    sh2.setCargo(game::Element::Neutronium, 10);

    const Reference r1(Reference::Ship, 1);                                // 75 kt hull + 100 kt N --> 175 kt
    const Reference r2(Reference::Ship, 2);                                // 75 kt hull + 10 kt N --> 85 kt
    const Reference rHull(Reference::Hull, game::test::OUTRIDER_HULL_ID);  // no mass(!)

    game::ref::SortBy::Mass t(addGame(env).currentTurn().universe(), addShipList(env));

    // Verify compare
    TS_ASSERT(t.compare(r1, r2) > 0);
    TS_ASSERT(t.compare(r1, r1) == 0);
    TS_ASSERT(t.compare(rHull, r1) < 0);

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r1), "");
    TS_ASSERT_EQUALS(t.getClass(rHull), "");
}

/** Test game::ref::SortBy::HullMass. */
void
TestGameRefSortBy::testHullMass()
{
    Environment env;
    addHull(env, 30).setMass(100);
    addHull(env, 40).setMass(70);
    addHull(env, 50).setMass(200);

    addPlayedShip(env, 1, 1, Point(1000, 1000)).setHull(30);
    addPlayedShip(env, 2, 1, Point(1000, 1000)).setHull(40);
    addPlayedShip(env, 3, 1, Point(1000, 1000)).setHull(50);
    addPlanet(env, 33);

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);
    const Reference rPlanet(Reference::Planet, 33);
    const Reference rHull(Reference::Hull, 40);

    game::ref::SortBy::HullMass t(addGame(env).currentTurn().universe(), addShipList(env));

    // Verify compare
    TS_ASSERT(t.compare(r1, r2) > 0);       // 100 > 70
    TS_ASSERT(t.compare(r1, r1) == 0);
    TS_ASSERT(t.compare(r2, r3) < 0);       // 70 < 200
    TS_ASSERT(t.compare(rHull, r1) < 0);    // 70 < 100
    TS_ASSERT(t.compare(rHull, r2) == 0);   // same
    TS_ASSERT(t.compare(rPlanet, r2) < 0);  // no mass

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r1), "");
    TS_ASSERT_EQUALS(t.getClass(r2), "");
    TS_ASSERT_EQUALS(t.getClass(r3), "");
    TS_ASSERT_EQUALS(t.getClass(rHull), "");
    TS_ASSERT_EQUALS(t.getClass(rPlanet), "");
}

/** Test game::ref::SortBy::HullType. */
void
TestGameRefSortBy::testHullType()
{
    Environment env;
    game::test::addOutrider(addShipList(env));
    game::test::addAnnihilation(addShipList(env));

    addPlayedShip(env, 1, 1, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);
    addPlayedShip(env, 2, 1, Point(1000, 1000)).setHull(game::test::OUTRIDER_HULL_ID);
    addPlayedShip(env, 3, 1, Point(1000, 1000)).setHull(game::test::ANNIHILATION_HULL_ID);
    addShip(env, 4);
    addPlanet(env, 33);
    addBeam(env, 9);

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);
    const Reference r4(Reference::Ship, 77);
    const Reference rPlanet(Reference::Planet, 33);
    const Reference rHull(Reference::Hull, game::test::OUTRIDER_HULL_ID);
    const Reference rBeam(Reference::Beam, 9);

    game::ref::SortBy::HullType t(addGame(env).currentTurn().universe(), addShipList(env), env.tx);

    // Verify compare
    TS_ASSERT(t.compare(r1, r2) > 0);       // Anni after Outrider
    TS_ASSERT(t.compare(r1, r1) == 0);
    TS_ASSERT(t.compare(r2, r3) < 0);
    TS_ASSERT(t.compare(r4, r3) < 0);       // Unknown before known ship
    TS_ASSERT(t.compare(r4, r2) < 0);
    TS_ASSERT(t.compare(rHull, r1) < 0);    // Outrider before Anni
    TS_ASSERT(t.compare(rHull, r2) == 0);
    TS_ASSERT(t.compare(rPlanet, r2) < 0);  // Planet before outrider
    TS_ASSERT(t.compare(rPlanet, rBeam) < 0);  // Planet before beam
    TS_ASSERT(t.compare(rPlanet, r4) < 0);  // Planet before unknown ship

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r1), "ANNIHILATION CLASS BATTLESHIP");
    TS_ASSERT_EQUALS(t.getClass(r2), "OUTRIDER CLASS SCOUT");
    TS_ASSERT_EQUALS(t.getClass(r3), "ANNIHILATION CLASS BATTLESHIP");
    TS_ASSERT_EQUALS(t.getClass(r4), "unknown");
    TS_ASSERT_EQUALS(t.getClass(rHull), "OUTRIDER CLASS SCOUT");
    TS_ASSERT_EQUALS(t.getClass(rPlanet), "Planet");
    TS_ASSERT_EQUALS(t.getClass(rBeam), "unknown");
}

/** Test game::ref::SortBy::BattleOrder. */
void
TestGameRefSortBy::testBattleOrder()
{
    Environment env;
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
    addPlanet(env, 33).setFriendlyCode(String_t("050"));

    const Reference r1(Reference::Ship, 1);
    const Reference r2(Reference::Ship, 2);
    const Reference r3(Reference::Ship, 3);
    const Reference r4(Reference::Ship, 4);
    const Reference r5(Reference::Ship, 5);
    const Reference rPlanet(Reference::Planet, 33);
    const Reference rHull(Reference::Hull, 77);

    // PHost rules
    {
        game::ref::SortBy::BattleOrder t(addGame(env).currentTurn().universe(), game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)), env.tx);

        // Verify compare
        TS_ASSERT(t.compare(r1, r2) < 0);
        TS_ASSERT(t.compare(r2, r3) > 0);
        TS_ASSERT(t.compare(r3, r4) > 0);
        TS_ASSERT(t.compare(r4, r5) < 0);
        TS_ASSERT(t.compare(r1, rPlanet) > 0);
        TS_ASSERT(t.compare(r1, rHull) < 0);     // hull counts as unknown

        // Verify class names
        TS_ASSERT_EQUALS(t.getClass(r1), "200 .. 299");
        TS_ASSERT_EQUALS(t.getClass(r2), "200 .. 299");
        TS_ASSERT_EQUALS(t.getClass(r3), "100 .. 199");
        TS_ASSERT_EQUALS(t.getClass(r4), "< 0");
        TS_ASSERT_EQUALS(t.getClass(r5), UTF_GEQ " 1000");
        TS_ASSERT_EQUALS(t.getClass(rPlanet), "0 .. 99");
        TS_ASSERT_EQUALS(t.getClass(rHull), "unknown");
    }

    // THost rules
    {
        game::ref::SortBy::BattleOrder t(addGame(env).currentTurn().universe(), game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,0)), env.tx);

        // Verify compare
        TS_ASSERT(t.compare(r1, r2) < 0);
        TS_ASSERT(t.compare(r2, r3) > 0);
        TS_ASSERT(t.compare(r3, r4) < 0);
        TS_ASSERT(t.compare(r4, r5) == 0);       // "-50" and "abc" both mean "no battle order" in THost
        TS_ASSERT(t.compare(r1, rPlanet) < 0);
        TS_ASSERT(t.compare(r1, rHull) < 0);     // hull counts as unknown

        // Verify class names
        TS_ASSERT_EQUALS(t.getClass(r1), "200 .. 299");
        TS_ASSERT_EQUALS(t.getClass(r2), "200 .. 299");
        TS_ASSERT_EQUALS(t.getClass(r3), "100 .. 199");
        TS_ASSERT_EQUALS(t.getClass(r4), UTF_GEQ " 1000");
        TS_ASSERT_EQUALS(t.getClass(r5), UTF_GEQ " 1000");
        TS_ASSERT_EQUALS(t.getClass(rPlanet), "unknown");
        TS_ASSERT_EQUALS(t.getClass(rHull), "unknown");
    }
}

/** Test game::ref::SortBy::Fleet. */
void
TestGameRefSortBy::testFleet()
{
    Environment env;
    game::map::Ship& sh1 = addPlayedShip(env, 10, 1, Point(1000, 1000));
    sh1.setFleetNumber(20);
    game::map::Ship& sh2 = addPlayedShip(env, 20, 1, Point(1000, 1000));
    sh2.setFleetNumber(20);
    sh2.setName(String_t("Boss"));
    /*game::map::Ship& sh3 =*/ addPlayedShip(env, 30, 1, Point(1000, 1000));
    game::map::Ship& sh4 = addPlayedShip(env, 40, 1, Point(1000, 1000));
    sh4.setFleetNumber(20);

    addPlanet(env, 33);

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference r40(Reference::Ship, 40);
    const Reference rPlanet(Reference::Planet, 33);

    game::ref::SortBy::Fleet t(addGame(env).currentTurn().universe(), env.tx);

    // Verify compare
    TS_ASSERT(t.compare(r10, r20) > 0);   // member after leader
    TS_ASSERT(t.compare(r20, r30) > 0);   // fleet after not-fleet
    TS_ASSERT(t.compare(r30, r40) < 0);
    TS_ASSERT(t.compare(r40, r10) == 0);      // members are equal
    TS_ASSERT(t.compare(rPlanet, r30) == 0);  // non-members are equal

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r10), "Fleet 20: led by Boss");
    TS_ASSERT_EQUALS(t.getClass(r20), "Fleet 20: led by Boss");
    TS_ASSERT_EQUALS(t.getClass(r30), "not in a fleet");
    TS_ASSERT_EQUALS(t.getClass(r40), "Fleet 20: led by Boss");
    TS_ASSERT_EQUALS(t.getClass(rPlanet), "not in a fleet");
}

/** Test game::ref::SortBy::TowGroup. */
void
TestGameRefSortBy::testTowGroup()
{
    Environment env;
    game::map::Ship& sh1 = addPlayedShip(env, 10, 1, Point(1000, 1000));
    sh1.setName(String_t("one"));
    game::map::Ship& sh2 = addPlayedShip(env, 20, 1, Point(1000, 1000));
    sh2.setMission(game::spec::Mission::msn_Tow, 0, 30);
    sh2.setName(String_t("two"));
    game::map::Ship& sh3 = addPlayedShip(env, 30, 1, Point(1000, 1000));
    sh3.setName(String_t("three"));
    game::map::Ship& sh4 = addPlayedShip(env, 40, 1, Point(1000, 1000));
    sh4.setName(String_t("four"));
    addPlanet(env, 33);

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference r40(Reference::Ship, 40);
    const Reference rPlanet(Reference::Planet, 33);

    game::ref::SortBy::TowGroup t(addGame(env).currentTurn().universe(), env.tx);

    // Verify compare

    TS_ASSERT(t.compare(r10, r20) < 0);        // not towed before tow group
    TS_ASSERT(t.compare(r20, r30) < 0);        // tower before towee
    TS_ASSERT(t.compare(r30, r40) > 0);        // towee after not towed
    TS_ASSERT(t.compare(r40, r10) == 0);       // not towed equal
    TS_ASSERT(t.compare(rPlanet, r40) == 0);   // not towed equal

    // Verify class names
    TS_ASSERT_EQUALS(t.getClass(r10), "not in a tow group");
    TS_ASSERT_EQUALS(t.getClass(r20), "towing three");
    TS_ASSERT_EQUALS(t.getClass(r30), "towing three");
    TS_ASSERT_EQUALS(t.getClass(r40), "not in a tow group");
    TS_ASSERT_EQUALS(t.getClass(rPlanet), "not in a tow group");
}

/** Test game::ref::SortBy::TransferTarget. */
void
TestGameRefSortBy::testTransferTarget()
{
    Environment env;

    game::map::Ship& sh1 = addPlayedShip(env, 10, 1, Point(1000, 1000));  // no transfer
    sh1.setName(String_t("one"));

    game::map::Ship& sh2 = addPlayedShip(env, 20, 1, Point(1000, 1000));  // transfer target
    sh2.setName(String_t("two"));

    game::map::Ship& sh3 = addPlayedShip(env, 30, 1, Point(1000, 1000));  // transfer to #20
    sh3.setName(String_t("three"));
    sh3.setTransporterTargetId(game::map::Ship::TransferTransporter, 20);
    sh3.setTransporterCargo(game::map::Ship::TransferTransporter, game::Element::Neutronium, 1);

    game::map::Ship& sh4 = addPlayedShip(env, 40, 1, Point(1000, 1000));  // jettison
    sh4.setName(String_t("four"));
    sh4.setTransporterTargetId(game::map::Ship::UnloadTransporter, 0);
    sh4.setTransporterCargo(game::map::Ship::UnloadTransporter, game::Element::Neutronium, 1);

    game::map::Ship& sh5 = addPlayedShip(env, 50, 1, Point(1000, 1000));  // unload to planet
    sh5.setName(String_t("four"));
    sh5.setTransporterTargetId(game::map::Ship::UnloadTransporter, 33);
    sh5.setTransporterCargo(game::map::Ship::UnloadTransporter, game::Element::Neutronium, 1);

    game::map::Ship& sh6 = addPlayedShip(env, 60, 1, Point(1000, 1000));  // no transfer
    sh6.setName(String_t("six"));

    addPlanet(env, 33).setName(String_t("Meatball"));

    const Reference r10(Reference::Ship, 10);
    const Reference r20(Reference::Ship, 20);
    const Reference r30(Reference::Ship, 30);
    const Reference r40(Reference::Ship, 40);
    const Reference r50(Reference::Ship, 50);
    const Reference r60(Reference::Ship, 60);
    const Reference rPlanet(Reference::Planet, 33);

    // Classic (checkOther=false)
    {
        game::ref::SortBy::TransferTarget t(addGame(env).currentTurn().universe(), game::map::Ship::UnloadTransporter, false, env.tx);

        TS_ASSERT(t.compare(r10, r20) < 0);          // unrelated, but sorted by Id
        TS_ASSERT(t.compare(r20, r30) < 0);          // 30 is unrelated, we're not looking at this transporter, thus sorted by Id
        TS_ASSERT(t.compare(r30, r40) < 0);          // Jettison after unrelated
        TS_ASSERT(t.compare(r40, r50) < 0);          // Unload after Jettison
        TS_ASSERT(t.compare(r50, r60) > 0);          // unrelated before Jettison
        TS_ASSERT(t.compare(r50, rPlanet) > 0);      // unrelated before Jettison
        TS_ASSERT(t.compare(r10, rPlanet) < 0);      // unrelated planet after unrelated ship

        TS_ASSERT_EQUALS(t.getClass(r10), "");
        TS_ASSERT_EQUALS(t.getClass(r20), "");
        TS_ASSERT_EQUALS(t.getClass(r30), "");
        TS_ASSERT_EQUALS(t.getClass(r40), "Jettison");
        TS_ASSERT_EQUALS(t.getClass(r50), "Unloading to Meatball");
        TS_ASSERT_EQUALS(t.getClass(rPlanet), "");
    }

    // Nu (checkOther=true)
    {
        game::ref::SortBy::TransferTarget t(addGame(env).currentTurn().universe(), game::map::Ship::TransferTransporter, true, env.tx);

        TS_ASSERT(t.compare(r10, r20) < 0);          // unrelated, but sorted by Id
        TS_ASSERT(t.compare(r20, r30) < 0);          // 30 is unrelated, we're not looking at this transporter, thus sorted by Id
        TS_ASSERT(t.compare(r30, r40) < 0);          // Jettison after unrelated
        TS_ASSERT(t.compare(r40, r50) < 0);          // Unload after Jettison
        TS_ASSERT(t.compare(r50, r60) > 0);          // unrelated before Jettison
        TS_ASSERT(t.compare(r50, rPlanet) > 0);      // unrelated before Jettison
        TS_ASSERT(t.compare(r10, rPlanet) < 0);      // unrelated planet after unrelated ship

        TS_ASSERT_EQUALS(t.getClass(r10), "");
        TS_ASSERT_EQUALS(t.getClass(r20), "");
        TS_ASSERT_EQUALS(t.getClass(r30), "Transferring to two");
        TS_ASSERT_EQUALS(t.getClass(r40), "Jettison");
        TS_ASSERT_EQUALS(t.getClass(r50), "Unloading to Meatball");
        TS_ASSERT_EQUALS(t.getClass(rPlanet), "");
    }
}

