/**
  *  \file u/t_game_map_location.cpp
  *  \brief Test for game::map::Location
  */

#include "game/map/location.hpp"

#include "t_game_map.hpp"
#include "game/test/simpleturn.hpp"

using game::map::Location;
using game::map::Point;
using game::map::Object;
using game::Reference;

namespace {
    struct PositionReceiver {
        std::vector<Point> results;

        void onPositionChange(Point pt)
            { results.push_back(pt); }
    };
}

/** Test operation on a point.
    A: set position using a point.
    E: check that updates are received, correct values reported. */
void
TestGameMapLocation::testPoint()
{
    // Testee
    Location testee;
    PositionReceiver recv;
    testee.sig_positionChange.add(&recv, &PositionReceiver::onPositionChange);

    // Initial position is unset
    Point pt;
    TS_ASSERT_EQUALS(testee.getPosition(pt), false);
    TS_ASSERT_EQUALS(testee.getReference(), Reference());

    // Set a position; must give a signal and be readable back
    testee.set(Point(10,20));
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], Point(10,20));
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, Point(10,20));

    // Same position again gives no notification
    testee.set(Point(10,20));
    TS_ASSERT_EQUALS(recv.results.size(), 1U);

    // Different position
    testee.set(Point(20,30));
    TS_ASSERT_EQUALS(recv.results.size(), 2U);
    TS_ASSERT_EQUALS(recv.results[1], Point(20,30));
}

/** Test operation on a reference.
    A: create universe. Set position using a reference.
    E: check that updates are received, correct values reported. */
void
TestGameMapLocation::testRef()
{
    // Environment
    const Point POS1(2000, 1500);
    const Point POS2(2010, 1600);
    game::test::SimpleTurn t;
    t.setPosition(POS1);
    t.addShip(1, 1, Object::Playable);
    t.addShip(2, 1, Object::Playable);
    t.setPosition(POS2);
    t.addShip(3, 1, Object::Playable);

    // Testee
    Location testee;
    PositionReceiver recv;
    testee.sig_positionChange.add(&recv, &PositionReceiver::onPositionChange);

    // Set position by reference
    Point pt;
    testee.setUniverse(&t.universe());
    testee.set(Reference(Reference::Ship, 1));
    TS_ASSERT_EQUALS(recv.results.size(), 1U);
    TS_ASSERT_EQUALS(recv.results[0], POS1);
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS1);

    // Change to another ship at same position
    testee.set(Reference(Reference::Ship, 2));
    TS_ASSERT_EQUALS(recv.results.size(), 1U);

    // Different ship
    testee.set(Reference(Reference::Ship, 3));
    TS_ASSERT_EQUALS(recv.results.size(), 2U);
    TS_ASSERT_EQUALS(recv.results[1], POS2);
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS2);
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 3));
}

/** Test universe change.
    A: create two universes. Set position using a reference.
    E: position changes if universe changes */
void
TestGameMapLocation::testUniv()
{
    // Environment - Turn 1
    const Point POS1(2000, 1500);
    game::test::SimpleTurn t1;
    t1.setPosition(POS1);
    t1.addShip(1, 1, Object::Playable);

    // Environment - Turn 2
    const Point POS2(2100, 1600);
    game::test::SimpleTurn t2;
    t2.setPosition(POS2);
    t2.addShip(1, 1, Object::Playable);

    // Testee
    Location testee;

    // Set position by reference
    Point pt;
    testee.setUniverse(&t1.universe());
    testee.set(Reference(Reference::Ship, 1));
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS1);

    // Reset universe, keeps position
    testee.setUniverse(0);
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS1);

    // Set to turn 2, moves position
    testee.setUniverse(&t2.universe());
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS2);
}

/** Test getEffectiveReference().
    A: set position to valid reference, valid point, invalid reference. Check getEffectiveReference().
    E: getEffectiveReference() returns reference only in case "valid reference" */
void
TestGameMapLocation::testEffectiveRef()
{
    const Point POS1(2000, 1500);
    const Point POS2(2010, 1600);
    game::test::SimpleTurn t;
    t.setPosition(POS1);
    t.addShip(1, 1, Object::Playable);

    // Testee
    Location testee;
    testee.setUniverse(&t.universe());

    // Set position to ship; verify
    Point pt;
    testee.set(Reference(Reference::Ship, 1));
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS1);
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 1));
    TS_ASSERT_EQUALS(testee.getEffectiveReference(), Reference(Reference::Ship, 1));

    // Set position
    testee.set(POS2);
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS2);
    TS_ASSERT_EQUALS(testee.getReference(), Reference());
    TS_ASSERT_EQUALS(testee.getEffectiveReference(), Reference());

    // Set invalid reference
    testee.set(Reference(Reference::Ship, 77));
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt, POS2);                                               // previous position
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 77));  // read-back correctly
    TS_ASSERT_EQUALS(testee.getEffectiveReference(), Reference());            // cleared out in getEffectiveReference()
}

/** Test browse().
    A: set up some ships. Try some browser operations.
    E: correct result */
void
TestGameMapLocation::testBrowse()
{
    game::test::SimpleTurn t;
    t.setPosition(Point(1000,2000));
    for (int i = 1; i <= 10; ++i) {
        t.addShip(i, 1, Object::Playable);
    }
    for (int i = 11; i <= 20; ++i) {
        t.addShip(i, 2, Object::NotPlayable);
    }

    // Testee
    Location testee;
    testee.setUniverse(&t.universe());

    // Start at ship 9
    testee.set(Reference(Reference::Ship, 9));

    // Browse forward
    testee.browse(Location::BrowseFlags_t());
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 10));
    testee.browse(Location::BrowseFlags_t());
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 11));
    testee.browse(Location::BrowseFlags_t());
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 12));

    // Accept only played
    testee.set(Reference(Reference::Ship, 9));
    testee.browse(Location::BrowseFlags_t(Location::PlayedOnly));
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 10));
    testee.browse(Location::BrowseFlags_t(Location::PlayedOnly));
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 1));

    // Backward
    testee.set(Reference(Reference::Ship, 9));
    testee.browse(Location::BrowseFlags_t(Location::Backwards));
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 8));

    // Backward, played
    testee.set(Reference(Reference::Ship, 1));
    testee.browse(Location::BrowseFlags_t() + Location::Backwards + Location::PlayedOnly);
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 10));

    // Marked
    t.universe().ships().get(13)->setIsMarked(true);
    testee.set(Reference(Reference::Ship, 1));
    testee.browse(Location::BrowseFlags_t(Location::MarkedOnly));
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Ship, 13));
}

/** Test browse().
    A: set up some planets. Try some browser operations.
    E: correct result */
void
TestGameMapLocation::testBrowsePlanet()
{
    game::test::SimpleTurn t;
    t.setPosition(Point(1000,2000));
    for (int i = 1; i <= 5; ++i) {
        t.addPlanet(i, 1, Object::Playable);
        t.setPosition(Point(1000+100*i,2000));
    }

    // Testee
    Location testee;
    testee.setUniverse(&t.universe());

    // Start at ship 9
    testee.set(Reference(Reference::Planet, 2));

    // Browse forward
    testee.browse(Location::BrowseFlags_t());
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Planet, 3));
    testee.browse(Location::BrowseFlags_t());
    TS_ASSERT_EQUALS(testee.getReference(), Reference(Reference::Planet, 4));
}

