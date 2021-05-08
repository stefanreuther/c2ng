/**
  *  \file u/t_game_sim_setup.cpp
  *  \brief Test for game::sim::Setup
  */

#include <map>
#include "game/sim/setup.hpp"

#include "t_game_sim.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/sim/gameinterface.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/test/counter.hpp"

namespace {
    int compareOwner(const game::sim::Ship& a, const game::sim::Ship& b)
    {
        return a.getOwner() - b.getOwner();
    }
}


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

    // Find
    game::sim::Setup::Slot_t slot;
    TS_ASSERT(testee.findIndex(s1, slot)); TS_ASSERT_EQUALS(slot, 0U);
    TS_ASSERT(testee.findIndex(s2, slot)); TS_ASSERT_EQUALS(slot, 1U);

    TS_ASSERT(testee.findIndex((game::sim::Object*) s1, slot)); TS_ASSERT_EQUALS(slot, 0U);
    TS_ASSERT(testee.findIndex((game::sim::Object*) s2, slot)); TS_ASSERT_EQUALS(slot, 1U);
    TS_ASSERT(testee.findIndex(p, slot)); TS_ASSERT_EQUALS(slot, 2U);

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
    TS_ASSERT(!testee.findIndex((game::sim::Ship*) 0, slot));
    TS_ASSERT(!testee.findIndex((game::sim::Object*) 0, slot));

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

    // Const find
    const game::sim::Setup& ct = testee;
    TS_ASSERT_EQUALS(testee.findShipById(1), ct.findShipById(1));
    TS_ASSERT_EQUALS(testee.findShipById(2), ct.findShipById(2));
    TS_ASSERT_EQUALS(testee.findShipById(3), ct.findShipById(3));

    // Ship Ids
    TS_ASSERT_EQUALS(testee.findUnusedShipId(1, 0), 3);
    TS_ASSERT_EQUALS(testee.findUnusedShipId(4, 0), 6);
    TS_ASSERT_EQUALS(testee.findUnusedShipId(10, 0), 10);

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
    for (int i = 1; i <= 7; ++i) {
        list.engines().create(i);
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
    util::RandomNumberGenerator rng(999);
    for (int i = 0; i < 1000; ++i) {
        testee.setRandomFriendlyCodes(rng);

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
    using game::test::Counter;
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

/** Test merging. */
void
TestGameSimSetup::testMerge()
{
    // Prepare
    game::sim::Setup a;
    {
        game::sim::Ship* a1 = a.addShip();
        a1->setId(1);
        a1->setName("a1");
        game::sim::Ship* a2 = a.addShip();
        a2->setId(2);
        a2->setName("a2");
    }

    game::sim::Setup b;
    {
        game::sim::Ship* b2 = b.addShip();
        b2->setId(2);
        b2->setName("b2");
        game::sim::Ship* b3 = b.addShip();
        b3->setId(3);
        b3->setName("b3");
        game::sim::Planet* p = b.addPlanet();
        p->setId(77);
    }

    // Do it
    a.merge(b);

    // Verify
    TS_ASSERT_EQUALS(a.getNumShips(), 3U);
    TS_ASSERT_EQUALS(a.hasPlanet(), true);
    TS_ASSERT_EQUALS(a.getShip(0)->getName(), "a1");
    TS_ASSERT_EQUALS(a.getShip(1)->getName(), "b2");
    TS_ASSERT_EQUALS(a.getShip(2)->getName(), "b3");
    TS_ASSERT_EQUALS(a.getPlanet()->getId(), 77);
}

/** Test findUnusedShipId with an interface. */
void
TestGameSimSetup::testFindUnused()
{
    // Mock interface that declares every ship present unless its Id is divisible by 5
    class MockInterface : public game::sim::GameInterface {
     public:
        virtual bool hasGame() const
            { return true; }
        virtual bool hasShip(game::Id_t shipId) const
            { return shipId % 5 != 0; }
        virtual String_t getPlanetName(game::Id_t /*id*/) const
            { return String_t(); }
        virtual game::Id_t getMaxPlanetId() const
            { return 0; }
        virtual int getShipOwner(game::Id_t /*id*/) const
            { return 0; }
        virtual game::Id_t getMaxShipId() const
            { return 0; }
        virtual bool copyShipFromGame(game::sim::Ship& /*out*/) const
            { return false; }
        virtual bool copyShipToGame(const game::sim::Ship& /*in*/)
            { return false; }
        virtual Relation getShipRelation(const game::sim::Ship& /*in*/) const
            { return Unknown; }
        virtual bool copyPlanetFromGame(game::sim::Planet& /*out*/) const
            { return false; }
        virtual bool copyPlanetToGame(const game::sim::Planet& /*in*/)
            { return false; }
        virtual Relation getPlanetRelation(const game::sim::Planet& /*in*/) const
            { return Unknown; }
    };

    game::sim::Setup testee;
    testee.addShip()->setId(8);
    testee.addShip()->setId(9);
    testee.addShip()->setId(10);
    testee.addShip()->setId(11);

    MockInterface gi;

    // Without interface
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 1, 0), 1);
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 4, 0), 4);
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 5, 0), 5);
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 9, 0), 12);
    TS_ASSERT_EQUALS(testee.findUnusedShipId(17, 0), 17);

    // With interface
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 1, &gi), 5);
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 4, &gi), 5);
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 5, &gi), 5);
    TS_ASSERT_EQUALS(testee.findUnusedShipId( 9, &gi), 15);
    TS_ASSERT_EQUALS(testee.findUnusedShipId(17, &gi), 20);
}

/** Test replicateShip(). */
void
TestGameSimSetup::testReplicate()
{
    afl::string::NullTranslator tx;

    // Prepare a setup [1,4]
    game::sim::Setup testee;
    game::sim::Ship* s1 = testee.addShip();
    s1->setId(1);
    s1->setName("One");
    s1->setHullTypeOnly(7);

    game::sim::Ship* s2 = testee.addShip();
    s2->setId(4);
    s2->setName("Four");
    s2->setHullTypeOnly(9);

    // Do it
    testee.replicateShip(0, 10, 0, tx);

    // Should now be [1, 2,3,5,6,7,8,9,10,11,12, 4]
    TS_ASSERT_EQUALS(testee.getNumShips(), 12U);
    TS_ASSERT_EQUALS(testee.getShip(0)->getId(), 1);
    TS_ASSERT_EQUALS(testee.getShip(1)->getId(), 2);
    TS_ASSERT_EQUALS(testee.getShip(2)->getId(), 3);
    TS_ASSERT_EQUALS(testee.getShip(3)->getId(), 5);
    TS_ASSERT_EQUALS(testee.getShip(4)->getId(), 6);
    TS_ASSERT_EQUALS(testee.getShip(11)->getId(), 4);

    TS_ASSERT_EQUALS(testee.getShip(0)->getHullType(), 7);
    TS_ASSERT_EQUALS(testee.getShip(1)->getHullType(), 7);
    TS_ASSERT_EQUALS(testee.getShip(2)->getHullType(), 7);
    TS_ASSERT_EQUALS(testee.getShip(3)->getHullType(), 7);
    TS_ASSERT_EQUALS(testee.getShip(4)->getHullType(), 7);
    TS_ASSERT_EQUALS(testee.getShip(11)->getHullType(), 9);
}

void
TestGameSimSetup::testCopy()
{
    using game::sim::GameInterface;
    using game::sim::Setup;

    class MockInterface : public GameInterface {
     public:
        typedef std::map<game::Id_t, String_t> NameMap_t;
        typedef std::map<game::Id_t, Relation> RelationMap_t;

        virtual bool hasGame() const
            { return true; }
        virtual bool hasShip(game::Id_t shipId) const
            { return shipId % 5 != 0; }
        virtual String_t getPlanetName(game::Id_t /*id*/) const
            { return String_t(); }
        virtual game::Id_t getMaxPlanetId() const
            { return 0; }
        virtual int getShipOwner(game::Id_t /*id*/) const
            { return 0; }
        virtual game::Id_t getMaxShipId() const
            { return 0; }
        virtual bool copyShipFromGame(game::sim::Ship& out) const
            {
                NameMap_t::const_iterator it = shipNames.find(out.getId());
                if (it != shipNames.end()) {
                    out.setName(it->second);
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool copyShipToGame(const game::sim::Ship& in)
            {
                NameMap_t::iterator it = shipNames.find(in.getId());
                if (it != shipNames.end()) {
                    it->second = in.getName();
                    return true;
                } else {
                    return false;
                }
            }
        virtual Relation getShipRelation(const game::sim::Ship& in) const
            {
                RelationMap_t::const_iterator it = shipRelations.find(in.getId());
                return it != shipRelations.end() ? it->second : Unknown;
            }
        virtual bool copyPlanetFromGame(game::sim::Planet& out) const
            {
                NameMap_t::const_iterator it = planetNames.find(out.getId());
                if (it != planetNames.end()) {
                    out.setName(it->second);
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool copyPlanetToGame(const game::sim::Planet& in)
            {
                NameMap_t::iterator it = planetNames.find(in.getId());
                if (it != planetNames.end()) {
                    it->second = in.getName();
                    return true;
                } else {
                    return false;
                }
            }
        virtual Relation getPlanetRelation(const game::sim::Planet& in) const
            {
                RelationMap_t::const_iterator it = planetRelations.find(in.getId());
                return it != planetRelations.end() ? it->second : Unknown;
            }

        NameMap_t planetNames;
        NameMap_t shipNames;
        RelationMap_t planetRelations;
        RelationMap_t shipRelations;
    };

    // Test failure to copy from game
    {
        game::sim::Setup testee;
        testee.addShip()->setId(4);
        testee.addShip()->setId(9);
        testee.addPlanet()->setId(12);

        MockInterface gi;
        gi.shipRelations[4] = GameInterface::Playable;
        gi.shipRelations[9] = GameInterface::Playable;
        gi.planetRelations[12] = GameInterface::Playable;

        Setup::Status st = testee.copyFromGame(gi);
        TS_ASSERT_EQUALS(st.failed, 3U);
        TS_ASSERT_EQUALS(st.succeeded, 0U);
    }

    // Test success to copy from game
    {
        game::sim::Setup testee;
        testee.addShip()->setId(4);
        testee.addShip()->setId(9);
        testee.addPlanet()->setId(12);

        MockInterface gi;
        gi.shipRelations[4] = GameInterface::Playable;
        gi.shipRelations[9] = GameInterface::Playable;
        gi.planetRelations[12] = GameInterface::Playable;
        gi.shipNames[9] = "a";
        gi.planetNames[12] = "b";

        Setup::Status st = testee.copyFromGame(gi);
        TS_ASSERT_EQUALS(st.failed, 1U);
        TS_ASSERT_EQUALS(st.succeeded, 2U);
        TS_ASSERT_EQUALS(testee.getShip(1)->getName(), "a");
        TS_ASSERT_EQUALS(testee.getPlanet()->getName(), "b");
    }

    // Test ranged copy from game
    {
        game::sim::Setup testee;
        testee.addShip()->setId(3);
        testee.addShip()->setId(5);
        testee.addShip()->setId(7);
        testee.getShip(2)->setName("xx");

        MockInterface gi;
        gi.shipRelations[3] = GameInterface::Playable;
        gi.shipRelations[5] = GameInterface::Playable;
        gi.shipRelations[7] = GameInterface::Playable;
        gi.shipNames[3] = "a";
        gi.shipNames[5] = "b";
        gi.shipNames[7] = "c";

        Setup::Status st = testee.copyFromGame(gi, 0, 2);
        TS_ASSERT_EQUALS(st.failed, 0U);
        TS_ASSERT_EQUALS(st.succeeded, 2U);
        TS_ASSERT_EQUALS(testee.getShip(0)->getName(), "a");
        TS_ASSERT_EQUALS(testee.getShip(1)->getName(), "b");
        TS_ASSERT_EQUALS(testee.getShip(2)->getName(), "xx");
    }

    // Test copy from unknown ship
    {
        game::sim::Setup testee;
        testee.addShip()->setId(3);
        testee.addShip()->setId(5);
        testee.addShip()->setId(7);
        testee.getShip(1)->setName("xx");

        MockInterface gi;
        gi.shipRelations[3] = GameInterface::Playable;
        gi.shipRelations[7] = GameInterface::Playable;
        gi.shipNames[3] = "a";
        gi.shipNames[5] = "b";
        gi.shipNames[7] = "c";

        Setup::Status st = testee.copyFromGame(gi);
        TS_ASSERT_EQUALS(st.failed, 0U);
        TS_ASSERT_EQUALS(st.succeeded, 2U);
        TS_ASSERT_EQUALS(testee.getShip(0)->getName(), "a");
        TS_ASSERT_EQUALS(testee.getShip(1)->getName(), "xx");
        TS_ASSERT_EQUALS(testee.getShip(2)->getName(), "c");
    }

    // Test failure to copy to game
    {
        game::sim::Setup testee;
        testee.addShip()->setId(4);
        testee.addShip()->setId(9);
        testee.addPlanet()->setId(12);
        testee.getShip(0)->setName("four");
        testee.getShip(1)->setName("nine");
        testee.getPlanet()->setName("twelve");

        MockInterface gi;
        gi.shipRelations[4] = GameInterface::Playable;
        gi.shipRelations[9] = GameInterface::Playable;
        gi.planetRelations[12] = GameInterface::Playable;

        Setup::Status st = testee.copyToGame(gi);
        TS_ASSERT_EQUALS(st.failed, 3U);
        TS_ASSERT_EQUALS(st.succeeded, 0U);
    }

    // Test success to copy to game
    {
        game::sim::Setup testee;
        testee.addShip()->setId(4);
        testee.addShip()->setId(9);
        testee.addPlanet()->setId(12);
        testee.getShip(0)->setName("four");
        testee.getShip(1)->setName("nine");
        testee.getPlanet()->setName("twelve");

        MockInterface gi;
        gi.shipRelations[4] = GameInterface::Playable;
        gi.shipRelations[9] = GameInterface::Playable;
        gi.planetRelations[12] = GameInterface::Playable;
        gi.shipNames[9] = "a";
        gi.planetNames[12] = "b";

        Setup::Status st = testee.copyToGame(gi);
        TS_ASSERT_EQUALS(st.failed, 1U);
        TS_ASSERT_EQUALS(st.succeeded, 2U);
        TS_ASSERT_EQUALS(gi.shipNames[9], "nine");
        TS_ASSERT_EQUALS(gi.planetNames[12], "twelve");
    }

    // Test ranged copy to game
    {
        game::sim::Setup testee;
        testee.addShip()->setId(3);
        testee.addShip()->setId(5);
        testee.addShip()->setId(7);
        testee.getShip(0)->setName("three");
        testee.getShip(1)->setName("five");
        testee.getShip(2)->setName("seven");

        MockInterface gi;
        gi.shipRelations[3] = GameInterface::Playable;
        gi.shipRelations[5] = GameInterface::Playable;
        gi.shipRelations[7] = GameInterface::Playable;
        gi.shipNames[3] = "a";
        gi.shipNames[5] = "b";
        gi.shipNames[7] = "c";

        Setup::Status st = testee.copyToGame(gi, 0, 2);
        TS_ASSERT_EQUALS(st.failed, 0U);
        TS_ASSERT_EQUALS(st.succeeded, 2U);
        TS_ASSERT_EQUALS(gi.shipNames[3], "three");
        TS_ASSERT_EQUALS(gi.shipNames[5], "five");
        TS_ASSERT_EQUALS(gi.shipNames[7], "c");
    }

    // Test copy to unknown ship
    {
        game::sim::Setup testee;
        testee.addShip()->setId(3);
        testee.addShip()->setId(5);
        testee.addShip()->setId(7);
        testee.getShip(0)->setName("three");
        testee.getShip(1)->setName("five");
        testee.getShip(2)->setName("seven");

        MockInterface gi;
        gi.shipRelations[3] = GameInterface::Playable;
        gi.shipRelations[7] = GameInterface::Playable;
        gi.shipNames[3] = "a";
        gi.shipNames[5] = "b";
        gi.shipNames[7] = "c";

        Setup::Status st = testee.copyToGame(gi);
        TS_ASSERT_EQUALS(st.failed, 0U);
        TS_ASSERT_EQUALS(st.succeeded, 2U);
        TS_ASSERT_EQUALS(gi.shipNames[3], "three");
        TS_ASSERT_EQUALS(gi.shipNames[5], "b");
        TS_ASSERT_EQUALS(gi.shipNames[7], "seven");
    }
}

void
TestGameSimSetup::testSetSequential()
{
    // Single ship -> random numeric code
    {
        game::sim::Setup t;
        game::sim::Ship* sh = t.addShip();
        t.setSequentialFriendlyCode(0);

        TS_ASSERT_EQUALS(sh->getFriendlyCode().size(), 3U);
        TS_ASSERT(sh->getFriendlyCode()[0] >= '0');
        TS_ASSERT(sh->getFriendlyCode()[1] >= '0');
        TS_ASSERT(sh->getFriendlyCode()[2] >= '0');
        TS_ASSERT(sh->getFriendlyCode()[0] <= '9');
        TS_ASSERT(sh->getFriendlyCode()[1] <= '9');
        TS_ASSERT(sh->getFriendlyCode()[2] <= '9');
    }

    // Single planet -> random numeric code
    {
        game::sim::Setup t;
        game::sim::Planet* pl = t.addPlanet();
        t.setSequentialFriendlyCode(0);

        TS_ASSERT_EQUALS(pl->getFriendlyCode().size(), 3U);
        TS_ASSERT(pl->getFriendlyCode()[0] >= '0');
        TS_ASSERT(pl->getFriendlyCode()[1] >= '0');
        TS_ASSERT(pl->getFriendlyCode()[2] >= '0');
        TS_ASSERT(pl->getFriendlyCode()[0] <= '9');
        TS_ASSERT(pl->getFriendlyCode()[1] <= '9');
        TS_ASSERT(pl->getFriendlyCode()[2] <= '9');
    }

    // Normal sequence
    {
        game::sim::Setup t;
        game::sim::Ship* s1 = t.addShip();
        game::sim::Ship* s2 = t.addShip();
        game::sim::Ship* s3 = t.addShip();
        s1->setFriendlyCode("109");
        s2->setFriendlyCode("abc");
        s3->setFriendlyCode("110");

        t.setSequentialFriendlyCode(1);
        TS_ASSERT_EQUALS(s2->getFriendlyCode(), "111");

        t.setSequentialFriendlyCode(2);
        TS_ASSERT_EQUALS(s3->getFriendlyCode(), "112");
    }

    // Copying of numerical places: x27 converted to <digit>28, then incremented
    {
        game::sim::Setup t;
        game::sim::Ship* s1 = t.addShip();
        game::sim::Ship* s2 = t.addShip();
        s1->setFriendlyCode("x27");
        s2->setFriendlyCode("abc");

        t.setSequentialFriendlyCode(1);
        TS_ASSERT_EQUALS(s2->getFriendlyCode().size(), 3U);
        TS_ASSERT(s2->getFriendlyCode()[0] >= '0');
        TS_ASSERT(s2->getFriendlyCode()[0] <= '9');
        TS_ASSERT_EQUALS(s2->getFriendlyCode()[1], '2');
        TS_ASSERT_EQUALS(s2->getFriendlyCode()[2], '8');
    }

    // Copying of random places: x<random>7 converted to <digit><digit>8, then incremented
    {
        game::sim::Setup t;
        game::sim::Ship* s1 = t.addShip();
        game::sim::Ship* s2 = t.addShip();
        s1->setFriendlyCode("x27");
        s1->setFlags(game::sim::Ship::fl_RandomFC2);
        s2->setFriendlyCode("abc");
        s2->setFlags(game::sim::Ship::fl_RandomFC);

        t.setSequentialFriendlyCode(1);
        TS_ASSERT_EQUALS(s2->getFriendlyCode().size(), 3U);
        TS_ASSERT(s2->getFriendlyCode()[0] >= '0');
        TS_ASSERT(s2->getFriendlyCode()[0] <= '9');
        TS_ASSERT(s2->getFriendlyCode()[1] >= '0');
        TS_ASSERT(s2->getFriendlyCode()[1] <= '9');
        TS_ASSERT_EQUALS(s2->getFriendlyCode()[2], '8');
        TS_ASSERT_EQUALS(s2->getFlags(), game::sim::Ship::fl_RandomFC + game::sim::Ship::fl_RandomFC2);
    }
}

/** Test sort(). */
void
TestGameSimSetup::testSort()
{
    game::sim::Setup t;
    game::sim::Ship* s1 = t.addShip(); s1->setOwner(3); s1->setId(1);
    game::sim::Ship* s2 = t.addShip(); s2->setOwner(1); s2->setId(2);
    game::sim::Ship* s3 = t.addShip(); s3->setOwner(4); s3->setId(3);
    game::sim::Ship* s4 = t.addShip(); s4->setOwner(2); s4->setId(4);
    game::sim::Ship* s5 = t.addShip(); s5->setOwner(1); s5->setId(5);
    t.sortShips(compareOwner);

    TS_ASSERT_EQUALS(t.getShip(0)->getOwner(), 1);
    TS_ASSERT_EQUALS(t.getShip(1)->getOwner(), 1);
    TS_ASSERT_EQUALS(t.getShip(2)->getOwner(), 2);
    TS_ASSERT_EQUALS(t.getShip(3)->getOwner(), 3);
    TS_ASSERT_EQUALS(t.getShip(4)->getOwner(), 4);

    TS_ASSERT_EQUALS(t.getShip(0)->getId(), 2);
    TS_ASSERT_EQUALS(t.getShip(1)->getId(), 5);
    TS_ASSERT_EQUALS(t.getShip(2)->getId(), 4);
    TS_ASSERT_EQUALS(t.getShip(3)->getId(), 1);
    TS_ASSERT_EQUALS(t.getShip(4)->getId(), 3);
}

/** Test addShip(), addPlanet() with data. */
void
TestGameSimSetup::testAddData()
{
    // Some objects
    game::sim::Planet p;
    p.setId(10);
    p.setName("Ten");

    game::sim::Ship s1;
    s1.setId(20);
    s1.setName("Twenty");

    game::sim::Ship s2;
    s2.setId(30);
    s2.setName("Thirty");

    game::sim::Ship s3;
    s3.setId(20);
    s3.setName("Twenty too");

    // Add them
    game::sim::Setup testee;
    game::sim::Planet* pp = testee.addPlanet(p);
    game::sim::Ship* ps1 = testee.addShip(s1);
    game::sim::Ship* ps2 = testee.addShip(s2);
    game::sim::Ship* ps3 = testee.addShip(s3);

    TS_ASSERT(pp != 0);
    TS_ASSERT(ps1 != 0);
    TS_ASSERT(ps2 != 0);
    TS_ASSERT(ps3 != 0);
    // Note that we don't make any guarantees about lifetimes, so ps1 may be dead now.

    TS_ASSERT_EQUALS(testee.getPlanet()->getId(), 10);
    TS_ASSERT_EQUALS(testee.getPlanet()->getName(), "Ten");
    TS_ASSERT_EQUALS(testee.getNumShips(), 2U);
    TS_ASSERT_EQUALS(testee.getShip(0)->getId(), 20);
    TS_ASSERT_EQUALS(testee.getShip(0)->getName(), "Twenty too");
    TS_ASSERT_EQUALS(testee.getShip(1)->getId(), 30);
    TS_ASSERT_EQUALS(testee.getShip(1)->getName(), "Thirty");
}

