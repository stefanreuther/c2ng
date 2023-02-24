/**
  *  \file u/t_game_map_historyshiptype.cpp
  *  \brief Test for game::map::HistoryShipType
  */

#include "game/map/historyshiptype.hpp"

#include "t_game_map.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"

using game::map::Point;
using game::map::Ship;

namespace {
    const int TURN_NR = 32;

    typedef game::map::ObjectVector<Ship> ShipVector_t;

    Ship& addShip(ShipVector_t& v, game::Id_t id, Point pos, int owner)
    {
        // Let source be different from owner, to make this "true" scans.
        // With source=owner, Ship::internalCheck would discard the ships as bogons,
        // because they should have got a proper full record (addCurrentShipData).
        game::PlayerSet_t source(owner+1);

        Ship& sh = *v.create(id);
        sh.addShipXYData(pos, owner, 100, source);
        sh.setPlayability(game::map::Object::NotPlayable);

        return sh;
    }

    void addShipTrack(Ship& ship, int age, Point pos)
    {
        game::parser::MessageInformation mi(game::parser::MessageInformation::Ship, ship.getId(), TURN_NR - age);
        mi.addValue(game::parser::mi_X, pos.getX());
        mi.addValue(game::parser::mi_Y, pos.getY());
        mi.addValue(game::parser::mi_Mass, 100);
        ship.addMessageInformation(mi, game::PlayerSet_t());
    }

    void finish(ShipVector_t& v)
    {
        game::map::Universe univ;
        for (game::Id_t i = 1, n = v.size(); i <= n; ++i) {
            if (Ship* sh = v.get(i)) {
                sh->internalCheck(game::PlayerSet_t(), TURN_NR);
            }
        }
    }
}

/** Test empty vector. */
void
TestGameMapHistoryShipType::testEmpty()
{
    game::map::ObjectVector<game::map::Ship> vec;
    vec.create(100);
    game::map::HistoryShipType testee(vec);

    TS_ASSERT_EQUALS(testee.countObjects(), 0);
    TS_ASSERT_EQUALS(testee.isEmpty(), true);
    TS_ASSERT_EQUALS(testee.isUnit(), false);
}

/** Test iteration functions. */
void
TestGameMapHistoryShipType::testIteration()
{
    game::map::ObjectVector<game::map::Ship> vec;
    const Point PA(1000, 1000);
    const Point PB(2000, 1000);
    const Point PE(99, 99);

    // Ship 10 normal
    /*Ship& s10 =*/ addShip(vec, 10, PA, 3);

    // Ship 20 with history
    Ship& s20 = addShip(vec, 20, PB, 3);
    addShipTrack(s20, 1, PA);

    // Ship 30 normal
    Ship& s30 = addShip(vec, 30, PA, 4);
    s30.setIsMarked(true);

    // Ship 40 has no data
    vec.create(40);

    // Ship 50 normal
    /*Ship& s50 =*/ addShip(vec, 50, PB, 4);

    // Finish (this sets the ship's kind)
    finish(vec);

    game::map::HistoryShipType testee(vec);
    TS_ASSERT_EQUALS(testee.countObjects(), 4);
    TS_ASSERT_EQUALS(testee.isEmpty(), false);
    TS_ASSERT_EQUALS(testee.isUnit(), false);

    // Regular browsing (base class function using isValid())
    // - next
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(0, false),  10);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(10, false), 20);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(20, false), 30);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(30, false), 50);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(50, false), 0);
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(7, false), 10);

    // - next, marked
    TS_ASSERT_EQUALS(testee.findNextIndexNoWrap(0, true), 30);

    // - previous
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(0, false),  50);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(50, false), 30);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(30, false), 20);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(20, false), 10);
    TS_ASSERT_EQUALS(testee.findPreviousIndexNoWrap(39, false), 30);

    // Location-based browsing
    int t;

    // - next, not marked
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtNoWrap(PA, 0, false, t), 10);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtNoWrap(PA, 10, false, t), 20);
    TS_ASSERT_EQUALS(t, TURN_NR-1);
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtNoWrap(PA, 20, false, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtNoWrap(PA, 30, false, t), 0);

    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtNoWrap(PE, 0, false, t), 0);

    // - next, marked
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtNoWrap(PA, 0, true, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);

    // - previous, not marked
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtNoWrap(PA, 0, false, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtNoWrap(PA, 30, false, t), 20);
    TS_ASSERT_EQUALS(t, TURN_NR-1);
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtNoWrap(PA, 20, false, t), 10);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtNoWrap(PA, 10, false, t), 0);

    // - previous, marked
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtNoWrap(PA, 0, true, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);

    // - with, wrap
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtWrap(PA, 30, false, t), 10);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtWrap(PA, 10, false, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtWrap(PA, 30, true, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);
    t = 0;
    TS_ASSERT_EQUALS(testee.findPreviousShipAtWrap(PA, 30, true, t), 30);
    TS_ASSERT_EQUALS(t, TURN_NR);

    t = 0;
    TS_ASSERT_EQUALS(testee.findNextShipAtWrap(PE, 0, false, t), 0);
    TS_ASSERT_EQUALS(testee.findPreviousShipAtWrap(PE, 0, false, t), 0);
}

