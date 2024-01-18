/**
  *  \file test/game/sim/setuptest.cpp
  *  \brief Test for game::sim::Setup
  */

#include "game/sim/setup.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/gameinterface.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/ship.hpp"
#include "game/test/counter.hpp"
#include <map>

namespace {
    int compareOwner(const game::sim::Ship& a, const game::sim::Ship& b)
    {
        return a.getOwner() - b.getOwner();
    }
}

using game::sim::GameInterface;
using game::sim::Object;
using game::sim::Planet;
using game::sim::Setup;
using game::sim::Ship;

/** Test object management. */
AFL_TEST("game.sim.Setup:object-management", a)
{
    // Initial state
    Setup testee;
    const Setup& cs = testee;
    a.check("01. hasPlanet", !testee.hasPlanet());
    a.checkNull("02. getPlanet", testee.getPlanet());
    a.checkEqual("03. getNumShips", testee.getNumShips(), 0U);
    a.checkNull("04. getShip", testee.getShip(0));
    a.checkEqual("05. getNumObjects", testee.getNumObjects(), 0U);

    // Add a planet
    Planet* p = testee.addPlanet();
    a.checkNonNull("11", p);
    a.check("12. hasPlanet", testee.hasPlanet());
    a.checkEqual("13. getPlanet", testee.getPlanet(), p);
    a.checkEqual("14. getNumObjects", testee.getNumObjects(), 1U);
    a.checkEqual("15. getObject", testee.getObject(0), p);
    a.checkEqual("16. getPlanet", cs.getPlanet(), p);
    a.check("17. hasPlanet", cs.hasPlanet());

    // Add two ships
    Ship* s1 = testee.addShip();
    Ship* s2 = testee.addShip();
    a.checkNonNull("21. addShip", s1);
    a.checkNonNull("22. addShip", s2);
    a.checkEqual("23. getNumShips", testee.getNumShips(), 2U);
    a.checkEqual("24. getShip", testee.getShip(0), s1);
    a.checkEqual("25. getShip", testee.getShip(1), s2);
    a.checkEqual("26. getNumObjects", testee.getNumObjects(), 3U);
    a.checkEqual("27. getObject", testee.getObject(0), s1);
    a.checkEqual("28. getObject", testee.getObject(1), s2);
    a.checkEqual("29. getObject", testee.getObject(2), p);
    a.checkNull("30. getObject", testee.getObject(3));

    // Find
    Setup::Slot_t slot;
    a.check("31. findIndex", testee.findIndex(s1).get(slot)); a.checkEqual("31. result", slot, 0U);
    a.check("32. findIndex", testee.findIndex(s2).get(slot)); a.checkEqual("32. result", slot, 1U);

    a.check("41. result", testee.findIndex((Object*) s1).get(slot)); a.checkEqual("41. result", slot, 0U);
    a.check("42. result", testee.findIndex((Object*) s2).get(slot)); a.checkEqual("42. result", slot, 1U);
    a.check("43. result", testee.findIndex(p).get(slot));            a.checkEqual("43. result", slot, 2U);

    // Copy
    Setup copy(testee);
    a.check("51. hasPlanet", copy.hasPlanet());
    a.checkEqual("52. getNumShips", copy.getNumShips(), 2U);
    a.checkEqual("53. getNumObjects", copy.getNumObjects(), 3U);
    a.checkDifferent("54. getObject", copy.getObject(0), s1);
    a.checkDifferent("55. getObject", copy.getObject(1), s2);
    a.checkDifferent("56. getObject", copy.getObject(2), p);

    // Self-assignment
    a.checkEqual("61. op=", &(copy = copy), &copy);
    a.check("62. hasPlanet", copy.hasPlanet());
    a.checkEqual("63. getNumShips", copy.getNumShips(), 2U);
    a.checkEqual("64. getNumObjects", copy.getNumObjects(), 3U);
    a.checkDifferent("65. getObject", copy.getObject(0), s1);
    a.checkDifferent("66. getObject", copy.getObject(1), s2);
    a.checkDifferent("67. getObject", copy.getObject(2), p);
    a.checkNonNull("68. getObject", copy.getObject(0));
    a.checkNonNull("69. getObject", copy.getObject(1));
    a.checkNonNull("70. getObject", copy.getObject(2));

    // Remove
    testee.removePlanet();
    testee.removeShip(0);
    a.checkEqual("71. getNumObjects", testee.getNumObjects(), 1U);
    a.checkNull("72. getPlanet", testee.getPlanet());
    a.check("73. hasPlanet", !testee.hasPlanet());
    a.checkEqual("74. getShip", testee.getShip(0), s2);

    // a is unaffected
    a.checkEqual("81. getNumObjects", copy.getNumObjects(), 3U);
}

/** Test ship operations. */
AFL_TEST("game.sim.Setup:ship-operations", a)
{
    // 4 ships
    Setup testee;
    Ship* s4 = testee.addShip();
    Ship* s1 = testee.addShip();
    Ship* s2 = testee.addShip();
    Ship* s5 = testee.addShip();
    s1->setId(1);
    s2->setId(2);
    s4->setId(4);
    s5->setId(5);

    Ship other;

    // Find
    Setup::Slot_t slot = 0;
    a.check("01. findIndex", testee.findIndex(s5).get(slot));
    a.checkEqual("02. result", slot, 3U);
    a.check("03. findIndex", !testee.findIndex(&other).get(slot));
    a.check("04. findIndex", !testee.findIndex((Ship*) 0).get(slot));
    a.check("05. findIndex", !testee.findIndex((Object*) 0).get(slot));

    a.check("11. findShipSlotById", testee.findShipSlotById(4).get(slot));
    a.checkEqual("12. result", slot, 0U);
    a.check("13. findShipSlotById", testee.findShipSlotById(2).get(slot));
    a.checkEqual("14. result", slot, 2U);
    a.check("15. findShipSlotById", !testee.findShipSlotById(3).get(slot));

    a.checkEqual("21. findShipById", testee.findShipById(1), s1);
    a.checkEqual("22. findShipById", testee.findShipById(2), s2);
    a.checkNull ("23. findShipById", testee.findShipById(3));
    a.checkEqual("24. findShipById", testee.findShipById(4), s4);
    a.checkEqual("25. findShipById", testee.findShipById(5), s5);

    // Const find
    const Setup& ct = testee;
    a.checkEqual("31. findShipById", testee.findShipById(1), ct.findShipById(1));
    a.checkEqual("32. findShipById", testee.findShipById(2), ct.findShipById(2));
    a.checkEqual("33. findShipById", testee.findShipById(3), ct.findShipById(3));

    // Ship Ids
    a.checkEqual("41. findUnusedShipId", testee.findUnusedShipId(1, 0), 3);
    a.checkEqual("42. findUnusedShipId", testee.findUnusedShipId(4, 0), 6);
    a.checkEqual("43. findUnusedShipId", testee.findUnusedShipId(10, 0), 10);

    // Swap
    a.checkEqual("51. getShip", testee.getShip(0), s4);
    a.checkEqual("52. getShip", testee.getShip(1), s1);
    a.checkEqual("53. getShip", testee.getShip(2), s2);
    a.checkEqual("54. getShip", testee.getShip(3), s5);
    testee.swapShips(1, 3);
    a.checkEqual("55. getShip", testee.getShip(0), s4);
    a.checkEqual("56. getShip", testee.getShip(1), s5);
    a.checkEqual("57. getShip", testee.getShip(2), s2);
    a.checkEqual("58. getShip", testee.getShip(3), s1);

    // Duplicate
    afl::string::NullTranslator tx;
    s2->setHullTypeOnly(92);
    testee.duplicateShip(2, 77, tx);
    a.checkEqual("61. getNumShips", testee.getNumShips(), 5U);
    a.checkEqual("62. getShip", testee.getShip(0), s4);
    a.checkEqual("63. getShip", testee.getShip(1), s5);
    a.checkEqual("64. getShip", testee.getShip(2), s2);
    a.checkNonNull("65. getShip", testee.getShip(3));           // newly-inserted ship
    a.checkEqual("66. getShip", testee.getShip(4), s1);
    a.checkEqual("67. getShip", testee.getShip(3)->getHullType(), 92);
}

/** Test isMatchingShipList(). */
AFL_TEST("game.sim.Setup:isMatchingShipList", a)
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
    Setup testee;
    a.check("01", testee.isMatchingShipList(list));

    // Add a ship
    Ship* s1 = testee.addShip();
    s1->setId(1);
    s1->setHullType(2, list);
    a.check("11", testee.isMatchingShipList(list));

    // Add another ship
    Ship* s2 = testee.addShip();
    s2->setId(2);
    s2->setHullType(1, list);
    a.check("21", testee.isMatchingShipList(list));

    // Vary
    s1->setNumBeams(6); // limit is 5
    a.check("31", !testee.isMatchingShipList(list));
}

/** Test setRandomFriendlyCodes(). */
AFL_TEST("game.sim.Setup:setRandomFriendlyCodes", a)
{
    Setup testee;

    // Ship 1
    Ship* s1 = testee.addShip();
    s1->setFlags(Object::fl_RandomFC);
    s1->setFriendlyCode("aaa");

    // Ship 2
    Ship* s2 = testee.addShip();
    s2->setFlags(Object::fl_RandomFC + Object::fl_RandomFC2);
    s2->setFriendlyCode("axc");

    // Do it
    util::RandomNumberGenerator rng(999);
    for (int i = 0; i < 1000; ++i) {
        testee.setRandomFriendlyCodes(rng);

        String_t s = s1->getFriendlyCode();
        a.checkEqual("01. size", s.size(), 3U);
        a.check("02. s[0]", '0' <= s[0]);
        a.check("03. s[0]", s[0] <= '9');
        a.check("04. s[1]", '0' <= s[1]);
        a.check("05. s[1]", s[1] <= '9');
        a.check("06. s[2]", '0' <= s[2]);
        a.check("07. s[2]", s[2] <= '9');

        s = s2->getFriendlyCode();
        a.checkEqual("11. size", s.size(), 3U);
        a.checkEqual("12", s[0], 'a');
        a.check("13. s[1]", '0' <= s[1]);
        a.check("14. s[1]", s[1] <= '9');
        a.checkEqual("13", s[2], 'c');
    }
}

/** Test listeners. */
AFL_TEST("game.sim.Setup:notifyListeners", a)
{
    // Set up and clear notifications
    Setup testee;
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
    Planet* p = testee.addPlanet();
    testee.notifyListeners();
    a.checkEqual("01. shipChange", shipChange.get(), 0);
    a.checkEqual("02. planetChange", planetChange.get(), 0);
    a.checkEqual("03. structChange", structChange.get(), 1);

    // Create ships
    Ship* s1 = testee.addShip();
    Ship* s2 = testee.addShip();
    testee.notifyListeners();
    a.checkEqual("11. shipChange", shipChange.get(), 0);
    a.checkEqual("12. planetChange", planetChange.get(), 0);
    a.checkEqual("13. structChange", structChange.get(), 2);

    // Modify planet
    p->setId(99);
    testee.notifyListeners();
    a.checkEqual("21. shipChange", shipChange.get(), 0);
    a.checkEqual("22. planetChange", planetChange.get(), 1);
    a.checkEqual("23. structChange", structChange.get(), 2);

    // Modify ship 1
    s1->setId(42);
    testee.notifyListeners();
    a.checkEqual("31. shipChange", shipChange.get(), 1);
    a.checkEqual("32. planetChange", planetChange.get(), 1);
    a.checkEqual("33. structChange", structChange.get(), 2);

    // Modify both ships
    s1->setHullTypeOnly(9);
    s2->setHullTypeOnly(8);
    testee.notifyListeners();
    a.checkEqual("41. shipChange", shipChange.get(), 3);      // Two notifications!
    a.checkEqual("42. planetChange", planetChange.get(), 1);
    a.checkEqual("43. structChange", structChange.get(), 2);

    // Swap
    testee.swapShips(0, 1);
    testee.notifyListeners();
    a.checkEqual("51. shipChange", shipChange.get(), 3);
    a.checkEqual("52. planetChange", planetChange.get(), 1);
    a.checkEqual("53. structChange", structChange.get(), 3);
}

/** Test merging. */
AFL_TEST("game.sim.Setup:merge", a)
{
    // Prepare
    Setup sa;
    {
        Ship* a1 = sa.addShip();
        a1->setId(1);
        a1->setName("a1");
        Ship* a2 = sa.addShip();
        a2->setId(2);
        a2->setName("a2");
    }

    Setup sb;
    {
        Ship* b2 = sb.addShip();
        b2->setId(2);
        b2->setName("b2");
        Ship* b3 = sb.addShip();
        b3->setId(3);
        b3->setName("b3");
        Planet* p = sb.addPlanet();
        p->setId(77);
    }

    // Do it
    sa.merge(sb);

    // Verify
    a.checkEqual("01. getNumShips", sa.getNumShips(), 3U);
    a.checkEqual("02. hasPlanet",   sa.hasPlanet(), true);
    a.checkEqual("03. getShip",     sa.getShip(0)->getName(), "a1");
    a.checkEqual("04. getShip",     sa.getShip(1)->getName(), "b2");
    a.checkEqual("05. getShip",     sa.getShip(2)->getName(), "b3");
    a.checkEqual("06. getPlanet",   sa.getPlanet()->getId(), 77);
}

/** Test findUnusedShipId with an interface. */
AFL_TEST("game.sim.Setup:findUnusedShipId", a)
{
    // Mock interface that declares every ship present unless its Id is divisible by 5
    class MockInterface : public GameInterface {
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
        virtual bool copyShipFromGame(Ship& /*out*/) const
            { return false; }
        virtual bool copyShipToGame(const Ship& /*in*/)
            { return false; }
        virtual Relation getShipRelation(const Ship& /*in*/) const
            { return Unknown; }
        virtual afl::base::Optional<game::map::Point> getShipPosition(const Ship& /*in*/) const
            { return afl::base::Nothing; }
        virtual bool copyPlanetFromGame(Planet& /*out*/) const
            { return false; }
        virtual bool copyPlanetToGame(const Planet& /*in*/)
            { return false; }
        virtual Relation getPlanetRelation(const Planet& /*in*/) const
            { return Unknown; }
        virtual afl::base::Optional<game::map::Point> getPlanetPosition(const Planet& /*in*/) const
            { return afl::base::Nothing; }
        virtual void getPlayerRelations(game::PlayerBitMatrix& /*alliances*/, game::PlayerBitMatrix& /*enemies*/) const
            { }
    };

    Setup testee;
    testee.addShip()->setId(8);
    testee.addShip()->setId(9);
    testee.addShip()->setId(10);
    testee.addShip()->setId(11);

    MockInterface gi;

    // Without interface
    a.checkEqual("01", testee.findUnusedShipId( 1, 0), 1);
    a.checkEqual("02", testee.findUnusedShipId( 4, 0), 4);
    a.checkEqual("03", testee.findUnusedShipId( 5, 0), 5);
    a.checkEqual("04", testee.findUnusedShipId( 9, 0), 12);
    a.checkEqual("05", testee.findUnusedShipId(17, 0), 17);

    // With interface
    a.checkEqual("11", testee.findUnusedShipId( 1, &gi), 5);
    a.checkEqual("12", testee.findUnusedShipId( 4, &gi), 5);
    a.checkEqual("13", testee.findUnusedShipId( 5, &gi), 5);
    a.checkEqual("14", testee.findUnusedShipId( 9, &gi), 15);
    a.checkEqual("15", testee.findUnusedShipId(17, &gi), 20);
}

/** Test replicateShip(). */
AFL_TEST("game.sim.Setup:replicateShip", a)
{
    afl::string::NullTranslator tx;

    // Prepare a setup [1,4]
    Setup testee;
    Ship* s1 = testee.addShip();
    s1->setId(1);
    s1->setName("One");
    s1->setHullTypeOnly(7);

    Ship* s2 = testee.addShip();
    s2->setId(4);
    s2->setName("Four");
    s2->setHullTypeOnly(9);

    // Do it
    testee.replicateShip(0, 10, 0, tx);

    // Should now be [1, 2,3,5,6,7,8,9,10,11,12, 4]
    a.checkEqual("01. getNumShips", testee.getNumShips(), 12U);
    a.checkEqual("02. getId", testee.getShip(0)->getId(), 1);
    a.checkEqual("03. getId", testee.getShip(1)->getId(), 2);
    a.checkEqual("04. getId", testee.getShip(2)->getId(), 3);
    a.checkEqual("05. getId", testee.getShip(3)->getId(), 5);
    a.checkEqual("06. getId", testee.getShip(4)->getId(), 6);
    a.checkEqual("07. getId", testee.getShip(11)->getId(), 4);

    a.checkEqual("11. getHullType", testee.getShip(0)->getHullType(), 7);
    a.checkEqual("12. getHullType", testee.getShip(1)->getHullType(), 7);
    a.checkEqual("13. getHullType", testee.getShip(2)->getHullType(), 7);
    a.checkEqual("14. getHullType", testee.getShip(3)->getHullType(), 7);
    a.checkEqual("15. getHullType", testee.getShip(4)->getHullType(), 7);
    a.checkEqual("16. getHullType", testee.getShip(11)->getHullType(), 9);
}

/*
 *  Test copyToGame / copyFromGame
 */

namespace {
    class CopyMockInterface : public GameInterface {
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
        virtual bool copyShipFromGame(Ship& out) const
            {
                NameMap_t::const_iterator it = shipNames.find(out.getId());
                if (it != shipNames.end()) {
                    out.setName(it->second);
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool copyShipToGame(const Ship& in)
            {
                NameMap_t::iterator it = shipNames.find(in.getId());
                if (it != shipNames.end()) {
                    it->second = in.getName();
                    return true;
                } else {
                    return false;
                }
            }
        virtual Relation getShipRelation(const Ship& in) const
            {
                RelationMap_t::const_iterator it = shipRelations.find(in.getId());
                return it != shipRelations.end() ? it->second : Unknown;
            }
        virtual afl::base::Optional<game::map::Point> getShipPosition(const Ship& /*in*/) const
            { return afl::base::Nothing; }
        virtual bool copyPlanetFromGame(Planet& out) const
            {
                NameMap_t::const_iterator it = planetNames.find(out.getId());
                if (it != planetNames.end()) {
                    out.setName(it->second);
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool copyPlanetToGame(const Planet& in)
            {
                NameMap_t::iterator it = planetNames.find(in.getId());
                if (it != planetNames.end()) {
                    it->second = in.getName();
                    return true;
                } else {
                    return false;
                }
            }
        virtual Relation getPlanetRelation(const Planet& in) const
            {
                RelationMap_t::const_iterator it = planetRelations.find(in.getId());
                return it != planetRelations.end() ? it->second : Unknown;
            }
        virtual afl::base::Optional<game::map::Point> getPlanetPosition(const Planet& /*in*/) const
            { return afl::base::Nothing; }
        virtual void getPlayerRelations(game::PlayerBitMatrix& /*alliances*/, game::PlayerBitMatrix& /*enemies*/) const
            { }

        NameMap_t planetNames;
        NameMap_t shipNames;
        RelationMap_t planetRelations;
        RelationMap_t shipRelations;
    };
}

// Test failure to copy from game
AFL_TEST("game.sim.Setup:copyFromGame:fail", a)
{
    Setup testee;
    testee.addShip()->setId(4);
    testee.addShip()->setId(9);
    testee.addPlanet()->setId(12);

    CopyMockInterface gi;
    gi.shipRelations[4] = GameInterface::Playable;
    gi.shipRelations[9] = GameInterface::Playable;
    gi.planetRelations[12] = GameInterface::Playable;

    Setup::Status st = testee.copyFromGame(gi);
    a.checkEqual("01. failed", st.failed, 3U);
    a.checkEqual("02. succeeded", st.succeeded, 0U);
}

// Test success to copy from game
AFL_TEST("game.sim.Setup:copyFromGame:success", a)
{
    Setup testee;
    testee.addShip()->setId(4);
    testee.addShip()->setId(9);
    testee.addPlanet()->setId(12);

    CopyMockInterface gi;
    gi.shipRelations[4] = GameInterface::Playable;
    gi.shipRelations[9] = GameInterface::Playable;
    gi.planetRelations[12] = GameInterface::Playable;
    gi.shipNames[9] = "a";
    gi.planetNames[12] = "b";

    Setup::Status st = testee.copyFromGame(gi);
    a.checkEqual("01. failed", st.failed, 1U);
    a.checkEqual("02. succeeded", st.succeeded, 2U);
    a.checkEqual("03. getName", testee.getShip(1)->getName(), "a");
    a.checkEqual("04. getName", testee.getPlanet()->getName(), "b");
}

// Test ranged copy from game
AFL_TEST("game.sim.Setup:copyFromGame:range", a)
{
    Setup testee;
    testee.addShip()->setId(3);
    testee.addShip()->setId(5);
    testee.addShip()->setId(7);
    testee.getShip(2)->setName("xx");

    CopyMockInterface gi;
    gi.shipRelations[3] = GameInterface::Playable;
    gi.shipRelations[5] = GameInterface::Playable;
    gi.shipRelations[7] = GameInterface::Playable;
    gi.shipNames[3] = "a";
    gi.shipNames[5] = "b";
    gi.shipNames[7] = "c";

    Setup::Status st = testee.copyFromGame(gi, 0, 2);
    a.checkEqual("01. failed", st.failed, 0U);
    a.checkEqual("02. succeeded", st.succeeded, 2U);
    a.checkEqual("03. getName", testee.getShip(0)->getName(), "a");
    a.checkEqual("04. getName", testee.getShip(1)->getName(), "b");
    a.checkEqual("05. getName", testee.getShip(2)->getName(), "xx");
}

// Test copy from unknown ship
AFL_TEST("game.sim.Setup:copyFromGame:unknown", a)
{
    Setup testee;
    testee.addShip()->setId(3);
    testee.addShip()->setId(5);
    testee.addShip()->setId(7);
    testee.getShip(1)->setName("xx");

    CopyMockInterface gi;
    gi.shipRelations[3] = GameInterface::Playable;
    gi.shipRelations[7] = GameInterface::Playable;
    gi.shipNames[3] = "a";
    gi.shipNames[5] = "b";
    gi.shipNames[7] = "c";

    Setup::Status st = testee.copyFromGame(gi);
    a.checkEqual("01. failed", st.failed, 0U);
    a.checkEqual("02. succeeded", st.succeeded, 2U);
    a.checkEqual("03. getName", testee.getShip(0)->getName(), "a");
    a.checkEqual("04. getName", testee.getShip(1)->getName(), "xx");
    a.checkEqual("05. getName", testee.getShip(2)->getName(), "c");
}

// Test failure to copy to game
AFL_TEST("game.sim.Setup:copyToGame:fail", a)
{
    Setup testee;
    testee.addShip()->setId(4);
    testee.addShip()->setId(9);
    testee.addPlanet()->setId(12);
    testee.getShip(0)->setName("four");
    testee.getShip(1)->setName("nine");
    testee.getPlanet()->setName("twelve");

    CopyMockInterface gi;
    gi.shipRelations[4] = GameInterface::Playable;
    gi.shipRelations[9] = GameInterface::Playable;
    gi.planetRelations[12] = GameInterface::Playable;

    Setup::Status st = testee.copyToGame(gi);
    a.checkEqual("01. failed", st.failed, 3U);
    a.checkEqual("02. succeeded", st.succeeded, 0U);
}

// Test success to copy to game
AFL_TEST("game.sim.Setup:copyToGame:success", a)
{
    Setup testee;
    testee.addShip()->setId(4);
    testee.addShip()->setId(9);
    testee.addPlanet()->setId(12);
    testee.getShip(0)->setName("four");
    testee.getShip(1)->setName("nine");
    testee.getPlanet()->setName("twelve");

    CopyMockInterface gi;
    gi.shipRelations[4] = GameInterface::Playable;
    gi.shipRelations[9] = GameInterface::Playable;
    gi.planetRelations[12] = GameInterface::Playable;
    gi.shipNames[9] = "a";
    gi.planetNames[12] = "b";

    Setup::Status st = testee.copyToGame(gi);
    a.checkEqual("01. failed", st.failed, 1U);
    a.checkEqual("02. succeeded", st.succeeded, 2U);
    a.checkEqual("03. shipNames", gi.shipNames[9], "nine");
    a.checkEqual("04. planetNames", gi.planetNames[12], "twelve");
}

// Test ranged copy to game
AFL_TEST("game.sim.Setup:copyToGame:range", a)
{
    Setup testee;
    testee.addShip()->setId(3);
    testee.addShip()->setId(5);
    testee.addShip()->setId(7);
    testee.getShip(0)->setName("three");
    testee.getShip(1)->setName("five");
    testee.getShip(2)->setName("seven");

    CopyMockInterface gi;
    gi.shipRelations[3] = GameInterface::Playable;
    gi.shipRelations[5] = GameInterface::Playable;
    gi.shipRelations[7] = GameInterface::Playable;
    gi.shipNames[3] = "a";
    gi.shipNames[5] = "b";
    gi.shipNames[7] = "c";

    Setup::Status st = testee.copyToGame(gi, 0, 2);
    a.checkEqual("01. failed", st.failed, 0U);
    a.checkEqual("02. succeeded", st.succeeded, 2U);
    a.checkEqual("03. shipNames", gi.shipNames[3], "three");
    a.checkEqual("04. shipNames", gi.shipNames[5], "five");
    a.checkEqual("05. shipNames", gi.shipNames[7], "c");
}

// Test copy to unknown ship
AFL_TEST("game.sim.Setup:copyToGame:unknown", a)
{
    Setup testee;
    testee.addShip()->setId(3);
    testee.addShip()->setId(5);
    testee.addShip()->setId(7);
    testee.getShip(0)->setName("three");
    testee.getShip(1)->setName("five");
    testee.getShip(2)->setName("seven");

    CopyMockInterface gi;
    gi.shipRelations[3] = GameInterface::Playable;
    gi.shipRelations[7] = GameInterface::Playable;
    gi.shipNames[3] = "a";
    gi.shipNames[5] = "b";
    gi.shipNames[7] = "c";

    Setup::Status st = testee.copyToGame(gi);
    a.checkEqual("01. failed", st.failed, 0U);
    a.checkEqual("02. succeeded", st.succeeded, 2U);
    a.checkEqual("03. shipNames", gi.shipNames[3], "three");
    a.checkEqual("04. shipNames", gi.shipNames[5], "b");
    a.checkEqual("05. shipNames", gi.shipNames[7], "seven");
}

/*
 *  setSequentialFriendlyCode
 */

// Single ship -> random numeric code
AFL_TEST("game.sim.Setup:setSequentialFriendlyCode:single-ship", a)
{
    Setup t;
    Ship* sh = t.addShip();
    t.setSequentialFriendlyCode(0);

    a.checkEqual("01", sh->getFriendlyCode().size(), 3U);
    a.check("02", sh->getFriendlyCode()[0] >= '0');
    a.check("03", sh->getFriendlyCode()[1] >= '0');
    a.check("04", sh->getFriendlyCode()[2] >= '0');
    a.check("05", sh->getFriendlyCode()[0] <= '9');
    a.check("06", sh->getFriendlyCode()[1] <= '9');
    a.check("07", sh->getFriendlyCode()[2] <= '9');
}

// Single planet -> random numeric code
AFL_TEST("game.sim.Setup:setSequentialFriendlyCode:single-planet", a)
{
    Setup t;
    Planet* pl = t.addPlanet();
    t.setSequentialFriendlyCode(0);

    a.checkEqual("01", pl->getFriendlyCode().size(), 3U);
    a.check("02", pl->getFriendlyCode()[0] >= '0');
    a.check("03", pl->getFriendlyCode()[1] >= '0');
    a.check("04", pl->getFriendlyCode()[2] >= '0');
    a.check("05", pl->getFriendlyCode()[0] <= '9');
    a.check("06", pl->getFriendlyCode()[1] <= '9');
    a.check("07", pl->getFriendlyCode()[2] <= '9');
}

// Normal sequence
AFL_TEST("game.sim.Setup:setSequentialFriendlyCode:normal", a)
{
    Setup t;
    Ship* s1 = t.addShip();
    Ship* s2 = t.addShip();
    Ship* s3 = t.addShip();
    s1->setFriendlyCode("109");
    s2->setFriendlyCode("abc");
    s3->setFriendlyCode("110");

    t.setSequentialFriendlyCode(1);
    a.checkEqual("01. getFriendlyCode", s2->getFriendlyCode(), "111");

    t.setSequentialFriendlyCode(2);
    a.checkEqual("02. getFriendlyCode", s3->getFriendlyCode(), "112");
}

// Copying of numerical places: x27 converted to <digit>28, then incremented
AFL_TEST("game.sim.Setup:setSequentialFriendlyCode:non-numeric", a)
{
    Setup t;
    Ship* s1 = t.addShip();
    Ship* s2 = t.addShip();
    s1->setFriendlyCode("x27");
    s2->setFriendlyCode("abc");

    t.setSequentialFriendlyCode(1);
    a.checkEqual("01", s2->getFriendlyCode().size(), 3U);
    a.check("02", s2->getFriendlyCode()[0] >= '0');
    a.check("03", s2->getFriendlyCode()[0] <= '9');
    a.checkEqual("04", s2->getFriendlyCode()[1], '2');
    a.checkEqual("05", s2->getFriendlyCode()[2], '8');
}

// Copying of random places: x<random>7 converted to <digit><digit>8, then incremented
AFL_TEST("game.sim.Setup:setSequentialFriendlyCode:random", a)
{
    Setup t;
    Ship* s1 = t.addShip();
    Ship* s2 = t.addShip();
    s1->setFriendlyCode("x27");
    s1->setFlags(Ship::fl_RandomFC2);
    s2->setFriendlyCode("abc");
    s2->setFlags(Ship::fl_RandomFC);

    t.setSequentialFriendlyCode(1);
    a.checkEqual("01", s2->getFriendlyCode().size(), 3U);
    a.check("02", s2->getFriendlyCode()[0] >= '0');
    a.check("03", s2->getFriendlyCode()[0] <= '9');
    a.check("04", s2->getFriendlyCode()[1] >= '0');
    a.check("05", s2->getFriendlyCode()[1] <= '9');
    a.checkEqual("06", s2->getFriendlyCode()[2], '8');
    a.checkEqual("07. getFlags", s2->getFlags(), Ship::fl_RandomFC + Ship::fl_RandomFC2);
}

/** Test sort(). */
AFL_TEST("game.sim.Setup:sortShips", a)
{
    Setup t;
    Ship* s1 = t.addShip(); s1->setOwner(3); s1->setId(1);
    Ship* s2 = t.addShip(); s2->setOwner(1); s2->setId(2);
    Ship* s3 = t.addShip(); s3->setOwner(4); s3->setId(3);
    Ship* s4 = t.addShip(); s4->setOwner(2); s4->setId(4);
    Ship* s5 = t.addShip(); s5->setOwner(1); s5->setId(5);
    t.sortShips(compareOwner);

    a.checkEqual("01. getOwner", t.getShip(0)->getOwner(), 1);
    a.checkEqual("02. getOwner", t.getShip(1)->getOwner(), 1);
    a.checkEqual("03. getOwner", t.getShip(2)->getOwner(), 2);
    a.checkEqual("04. getOwner", t.getShip(3)->getOwner(), 3);
    a.checkEqual("05. getOwner", t.getShip(4)->getOwner(), 4);

    a.checkEqual("11. getId", t.getShip(0)->getId(), 2);
    a.checkEqual("12. getId", t.getShip(1)->getId(), 5);
    a.checkEqual("13. getId", t.getShip(2)->getId(), 4);
    a.checkEqual("14. getId", t.getShip(3)->getId(), 1);
    a.checkEqual("15. getId", t.getShip(4)->getId(), 3);
}

/** Test addShip(), addPlanet() with data. */
AFL_TEST("game.sim.Setup:add-with-data", a)
{
    // Some objects
    Planet p;
    p.setId(10);
    p.setName("Ten");

    Ship s1;
    s1.setId(20);
    s1.setName("Twenty");

    Ship s2;
    s2.setId(30);
    s2.setName("Thirty");

    Ship s3;
    s3.setId(20);
    s3.setName("Twenty too");

    // Add them
    Setup testee;
    Planet* pp = testee.addPlanet(p);
    Ship* ps1 = testee.addShip(s1);
    Ship* ps2 = testee.addShip(s2);
    Ship* ps3 = testee.addShip(s3);

    a.checkNonNull("01. addPlanet", pp);
    a.checkNonNull("02. addShip", ps1);
    a.checkNonNull("03. addShip", ps2);
    a.checkNonNull("04. addShip", ps3);
    // Note that we don't make any guarantees about lifetimes, so ps1 may be dead now.

    a.checkEqual("11. getId",       testee.getPlanet()->getId(), 10);
    a.checkEqual("12. getName",     testee.getPlanet()->getName(), "Ten");
    a.checkEqual("13. getNumShips", testee.getNumShips(), 2U);
    a.checkEqual("14. getId",       testee.getShip(0)->getId(), 20);
    a.checkEqual("15. getName",     testee.getShip(0)->getName(), "Twenty too");
    a.checkEqual("16. getId",       testee.getShip(1)->getId(), 30);
    a.checkEqual("17. getName",     testee.getShip(1)->getName(), "Thirty");
}

/** Test setFlags(). */
AFL_TEST("game.sim.Setup:setFlags", a)
{
    Setup testee;
    Planet* pp = testee.addPlanet();
    Ship* ps1 = testee.addShip();
    Ship* ps2 = testee.addShip();
    Ship* ps3 = testee.addShip();
    pp->setFlags(Object::fl_Deactivated);
    ps2->setFlags(Object::fl_Cloaked);

    // Test:        set RandomFC,         clear Cloaked,      toggle Deactivated
    testee.setFlags(Object::fl_RandomFC | Object::fl_Cloaked,
                    Object::fl_RandomFC                      | Object::fl_Deactivated);

    // Verify
    a.checkEqual("01. planet", pp->getFlags(),  Object::fl_RandomFC);
    a.checkEqual("02. ship 1", ps1->getFlags(), Object::fl_RandomFC | Object::fl_Deactivated);
    a.checkEqual("03. ship 2", ps2->getFlags(), Object::fl_RandomFC | Object::fl_Deactivated);
    a.checkEqual("04. ship 3", ps3->getFlags(), Object::fl_RandomFC | Object::fl_Deactivated);
}

/** Test getInvolvedPlayers(), getInvolvedTeams(). */
AFL_TEST("game.sim.Setup:getInvolvedPlayers", a)
{
    // Setup
    Setup testee;
    testee.addShip()->setOwner(1);
    testee.addShip()->setOwner(2);
    testee.addShip()->setOwner(1);
    testee.addShip()->setOwner(7);
    testee.addPlanet()->setOwner(4);

    // Team settings
    game::TeamSettings team;
    team.setPlayerTeam(2, 9);
    team.setPlayerTeam(4, 9);

    // Check
    a.checkEqual("01. getInvolvedPlayers", testee.getInvolvedPlayers(),   game::PlayerSet_t() + 1 + 2 + 4 + 7);
    a.checkEqual("02. getInvolvedTeams",   testee.getInvolvedTeams(team), game::PlayerSet_t() + 1         + 7 + 9);
}
