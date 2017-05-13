/**
  *  \file u/t_game_sim_setup.cpp
  *  \brief Test for game::sim::Setup
  */

#include "game/sim/setup.hpp"

#include "t_game_sim.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "afl/string/nulltranslator.hpp"
#include "u/helper/counter.hpp"

/** Test object management. */
void
TestGameSimSetup::testObj()
{
    // Initial state
    game::sim::Setup testee;
    const game::sim::Setup& cs = testee;
    TS_ASSERT(!testee.hasPlanet());
    TS_ASSERT(testee.getPlanet() == 0);
    TS_ASSERT_EQUALS(testee.getNumShips(), 0U);
    TS_ASSERT(testee.getShip(0) == 0);
    TS_ASSERT_EQUALS(testee.getNumObjects(), 0U);

    // Add a planet
    game::sim::Planet* p = testee.addPlanet();
    TS_ASSERT(p != 0);
    TS_ASSERT(testee.hasPlanet());
    TS_ASSERT_EQUALS(testee.getPlanet(), p);
    TS_ASSERT_EQUALS(testee.getNumObjects(), 1U);
    TS_ASSERT_EQUALS(testee.getObject(0), p);
    TS_ASSERT_EQUALS(cs.getPlanet(), p);
    TS_ASSERT(cs.hasPlanet());

    // Add two ships
    game::sim::Ship* s1 = testee.addShip();
    game::sim::Ship* s2 = testee.addShip();
    TS_ASSERT(s1 != 0);
    TS_ASSERT(s2 != 0);
    TS_ASSERT_EQUALS(testee.getNumShips(), 2U);
    TS_ASSERT_EQUALS(testee.getShip(0), s1);
    TS_ASSERT_EQUALS(testee.getShip(1), s2);
    TS_ASSERT_EQUALS(testee.getNumObjects(), 3U);
    TS_ASSERT_EQUALS(testee.getObject(0), s1);
    TS_ASSERT_EQUALS(testee.getObject(1), s2);
    TS_ASSERT_EQUALS(testee.getObject(2), p);
    TS_ASSERT(testee.getObject(3) == 0);

    // Copy
    game::sim::Setup a(testee);
    TS_ASSERT(a.hasPlanet());
    TS_ASSERT_EQUALS(a.getNumShips(), 2U);
    TS_ASSERT_EQUALS(a.getNumObjects(), 3U);
    TS_ASSERT_DIFFERS(a.getObject(0), s1);
    TS_ASSERT_DIFFERS(a.getObject(1), s2);
    TS_ASSERT_DIFFERS(a.getObject(2), p);

    // Self-assignment
    TS_ASSERT_EQUALS(&(a = a), &a);
    TS_ASSERT(a.hasPlanet());
    TS_ASSERT_EQUALS(a.getNumShips(), 2U);
    TS_ASSERT_EQUALS(a.getNumObjects(), 3U);
    TS_ASSERT_DIFFERS(a.getObject(0), s1);
    TS_ASSERT_DIFFERS(a.getObject(1), s2);
    TS_ASSERT_DIFFERS(a.getObject(2), p);
    TS_ASSERT(a.getObject(0) != 0);
    TS_ASSERT(a.getObject(1) != 0);
    TS_ASSERT(a.getObject(2) != 0);

    // Remove
    testee.removePlanet();
    testee.removeShip(0);
    TS_ASSERT_EQUALS(testee.getNumObjects(), 1U);
    TS_ASSERT(testee.getPlanet() == 0);
    TS_ASSERT(!testee.hasPlanet());
    TS_ASSERT_EQUALS(testee.getShip(0), s2);

    // a is unaffected
    TS_ASSERT_EQUALS(a.getNumObjects(), 3U);
}

/** Test ship operations. */
void
TestGameSimSetup::testShip()
{
    // 4 ships
    game::sim::Setup testee;
    game::sim::Ship* s4 = testee.addShip();
    game::sim::Ship* s1 = testee.addShip();
    game::sim::Ship* s2 = testee.addShip();
    game::sim::Ship* s5 = testee.addShip();
    s1->setId(1);
    s2->setId(2);
    s4->setId(4);
    s5->setId(5);

    game::sim::Ship other;

    // Find
    game::sim::Setup::Slot_t slot = 0;
    TS_ASSERT(testee.findIndex(s5, slot));
    TS_ASSERT_EQUALS(slot, 3U);
    TS_ASSERT(!testee.findIndex(&other, slot));
    TS_ASSERT(!testee.findIndex(0, slot));

    TS_ASSERT(testee.findShipSlotById(4, slot));
    TS_ASSERT_EQUALS(slot, 0U);
    TS_ASSERT(testee.findShipSlotById(2, slot));
    TS_ASSERT_EQUALS(slot, 2U);
    TS_ASSERT(!testee.findShipSlotById(3, slot));

    TS_ASSERT_EQUALS(testee.findShipById(1), s1);
    TS_ASSERT_EQUALS(testee.findShipById(2), s2);
    TS_ASSERT(testee.findShipById(3) == 0);
    TS_ASSERT_EQUALS(testee.findShipById(4), s4);
    TS_ASSERT_EQUALS(testee.findShipById(5), s5);

    // Ship Ids
    TS_ASSERT_EQUALS(testee.findUnusedShipId(1), 3);
    TS_ASSERT_EQUALS(testee.findUnusedShipId(4), 6);
    TS_ASSERT_EQUALS(testee.findUnusedShipId(10), 10);

    // Swap
    TS_ASSERT_EQUALS(testee.getShip(0), s4);
    TS_ASSERT_EQUALS(testee.getShip(1), s1);
    TS_ASSERT_EQUALS(testee.getShip(2), s2);
    TS_ASSERT_EQUALS(testee.getShip(3), s5);
    testee.swapShips(1, 3);
    TS_ASSERT_EQUALS(testee.getShip(0), s4);
    TS_ASSERT_EQUALS(testee.getShip(1), s5);
    TS_ASSERT_EQUALS(testee.getShip(2), s2);
    TS_ASSERT_EQUALS(testee.getShip(3), s1);

    // Duplicate
    afl::string::NullTranslator tx;
    s2->setHullTypeOnly(92);
    testee.duplicateShip(2, 77, tx);
    TS_ASSERT_EQUALS(testee.getNumShips(), 5U);
    TS_ASSERT_EQUALS(testee.getShip(0), s4);
    TS_ASSERT_EQUALS(testee.getShip(1), s5);
    TS_ASSERT_EQUALS(testee.getShip(2), s2);
    TS_ASSERT(testee.getShip(3) != 0);           // newly-inserted ship
    TS_ASSERT_EQUALS(testee.getShip(4), s1);
    TS_ASSERT_EQUALS(testee.getShip(3)->getHullType(), 92);
}

/** Test isMatchingShipList(). */
void
TestGameSimSetup::testShipList()
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

    // Initial state
    game::sim::Setup testee;
    TS_ASSERT(testee.isMatchingShipList(list));

    // Add a ship
    game::sim::Ship* s1 = testee.addShip();
    s1->setId(1);
    s1->setHullType(2, list);
    TS_ASSERT(testee.isMatchingShipList(list));

    // Add another ship
    game::sim::Ship* s2 = testee.addShip();
    s2->setId(2);
    s2->setHullType(1, list);
    TS_ASSERT(testee.isMatchingShipList(list));

    // Vary
    s1->setNumBeams(6); // limit is 5
    TS_ASSERT(!testee.isMatchingShipList(list));
}

/** Test setRandomFriendlyCodes(). */
void
TestGameSimSetup::testRandom()
{
    game::sim::Setup testee;

    // Ship 1
    game::sim::Ship* s1 = testee.addShip();
    s1->setFlags(game::sim::Object::fl_RandomFC);
    s1->setFriendlyCode("aaa");

    // Ship 2
    game::sim::Ship* s2 = testee.addShip();
    s2->setFlags(game::sim::Object::fl_RandomFC + game::sim::Object::fl_RandomFC2);
    s2->setFriendlyCode("axc");

    // Do it
    for (int i = 0; i < 1000; ++i) {
        testee.setRandomFriendlyCodes();

        String_t s = s1->getFriendlyCode();
        TS_ASSERT_EQUALS(s.size(), 3U);
        TS_ASSERT_LESS_THAN_EQUALS('0', s[0]);
        TS_ASSERT_LESS_THAN_EQUALS(s[0], '9');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[1]);
        TS_ASSERT_LESS_THAN_EQUALS(s[1], '9');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[2]);
        TS_ASSERT_LESS_THAN_EQUALS(s[2], '9');

        s = s2->getFriendlyCode();
        TS_ASSERT_EQUALS(s.size(), 3U);
        TS_ASSERT_EQUALS(s[0], 'a');
        TS_ASSERT_LESS_THAN_EQUALS('0', s[1]);
        TS_ASSERT_LESS_THAN_EQUALS(s[1], '9');
        TS_ASSERT_EQUALS(s[2], 'c');
    }
}

/** Test listeners. */
void
TestGameSimSetup::testListener()
{
    // Set up and clear notifications
    game::sim::Setup testee;
    testee.notifyListeners();

    // Add listeners
    Counter shipChange;
    Counter planetChange;
    Counter structChange;
    testee.sig_shipChange.add(&shipChange, &Counter::increment);
    testee.sig_planetChange.add(&planetChange, &Counter::increment);
    testee.sig_structureChange.add(&structChange, &Counter::increment);

    // Create a planet
    game::sim::Planet* p = testee.addPlanet();
    testee.notifyListeners();
    TS_ASSERT_EQUALS(shipChange.get(), 0);
    TS_ASSERT_EQUALS(planetChange.get(), 0);
    TS_ASSERT_EQUALS(structChange.get(), 1);

    // Create ships
    game::sim::Ship* s1 = testee.addShip();
    game::sim::Ship* s2 = testee.addShip();
    testee.notifyListeners();
    TS_ASSERT_EQUALS(shipChange.get(), 0);
    TS_ASSERT_EQUALS(planetChange.get(), 0);
    TS_ASSERT_EQUALS(structChange.get(), 2);

    // Modify planet
    p->setId(99);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(shipChange.get(), 0);
    TS_ASSERT_EQUALS(planetChange.get(), 1);
    TS_ASSERT_EQUALS(structChange.get(), 2);

    // Modify ship 1
    s1->setId(42);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(shipChange.get(), 1);
    TS_ASSERT_EQUALS(planetChange.get(), 1);
    TS_ASSERT_EQUALS(structChange.get(), 2);

    // Modify both ships
    s1->setHullTypeOnly(9);
    s2->setHullTypeOnly(8);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(shipChange.get(), 3);      // Two notifications!
    TS_ASSERT_EQUALS(planetChange.get(), 1);
    TS_ASSERT_EQUALS(structChange.get(), 2);

    // Swap
    testee.swapShips(0, 1);
    testee.notifyListeners();
    TS_ASSERT_EQUALS(shipChange.get(), 3);
    TS_ASSERT_EQUALS(planetChange.get(), 1);
    TS_ASSERT_EQUALS(structChange.get(), 3);
}

