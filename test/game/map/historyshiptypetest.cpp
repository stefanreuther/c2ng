/**
  *  \file test/game/map/historyshiptypetest.cpp
  *  \brief Test for game::map::HistoryShipType
  */

#include "game/map/historyshiptype.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/ship.hpp"

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
        for (game::Id_t i = 1, n = v.size(); i <= n; ++i) {
            if (Ship* sh = v.get(i)) {
                sh->internalCheck(game::PlayerSet_t(), TURN_NR);
            }
        }
    }
}

/** Test empty vector. */
AFL_TEST("game.map.HistoryShipType:empty", a)
{
    game::map::ObjectVector<game::map::Ship> vec;
    vec.create(100);
    game::map::HistoryShipType testee(vec);

    a.checkEqual("01. countObjects", testee.countObjects(), 0);
    a.checkEqual("02. isEmpty", testee.isEmpty(), true);
    a.checkEqual("03. isUnit", testee.isUnit(), false);
}

/** Test iteration functions. */
AFL_TEST("game.map.HistoryShipType:iteration", a)
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
    a.checkEqual("01. countObjects", testee.countObjects(), 4);
    a.checkEqual("02. isEmpty", testee.isEmpty(), false);
    a.checkEqual("03. isUnit", testee.isUnit(), false);

    // Regular browsing (base class function using isValid())
    // - next
    a.checkEqual("11. findNextIndexNoWrap", testee.findNextIndexNoWrap(0, false),  10);
    a.checkEqual("12. findNextIndexNoWrap", testee.findNextIndexNoWrap(10, false), 20);
    a.checkEqual("13. findNextIndexNoWrap", testee.findNextIndexNoWrap(20, false), 30);
    a.checkEqual("14. findNextIndexNoWrap", testee.findNextIndexNoWrap(30, false), 50);
    a.checkEqual("15. findNextIndexNoWrap", testee.findNextIndexNoWrap(50, false), 0);
    a.checkEqual("16. findNextIndexNoWrap", testee.findNextIndexNoWrap(7, false), 10);

    // - next, marked
    a.checkEqual("21. findNextIndexNoWrap", testee.findNextIndexNoWrap(0, true), 30);

    // - previous
    a.checkEqual("31. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(0, false),  50);
    a.checkEqual("32. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(50, false), 30);
    a.checkEqual("33. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(30, false), 20);
    a.checkEqual("34. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(20, false), 10);
    a.checkEqual("35. findPreviousIndexNoWrap", testee.findPreviousIndexNoWrap(39, false), 30);

    // Location-based browsing
    int t;

    // - next, not marked
    t = 0;
    a.checkEqual("41. findNextShipAtNoWrap", testee.findNextShipAtNoWrap(PA, 0, false, t), 10);
    a.checkEqual("42. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("43. findNextShipAtNoWrap", testee.findNextShipAtNoWrap(PA, 10, false, t), 20);
    a.checkEqual("44. turn result", t, TURN_NR-1);
    t = 0;
    a.checkEqual("45. findNextShipAtNoWrap", testee.findNextShipAtNoWrap(PA, 20, false, t), 30);
    a.checkEqual("46. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("47. findNextShipAtNoWrap", testee.findNextShipAtNoWrap(PA, 30, false, t), 0);

    t = 0;
    a.checkEqual("51. findNextShipAtNoWrap", testee.findNextShipAtNoWrap(PE, 0, false, t), 0);

    // - next, marked
    t = 0;
    a.checkEqual("61. findNextShipAtNoWrap", testee.findNextShipAtNoWrap(PA, 0, true, t), 30);
    a.checkEqual("62. turn result", t, TURN_NR);

    // - previous, not marked
    t = 0;
    a.checkEqual("71. findPreviousShipAtNoWrap", testee.findPreviousShipAtNoWrap(PA, 0, false, t), 30);
    a.checkEqual("72. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("73. findPreviousShipAtNoWrap", testee.findPreviousShipAtNoWrap(PA, 30, false, t), 20);
    a.checkEqual("74. turn result", t, TURN_NR-1);
    t = 0;
    a.checkEqual("75. findPreviousShipAtNoWrap", testee.findPreviousShipAtNoWrap(PA, 20, false, t), 10);
    a.checkEqual("76. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("77. findPreviousShipAtNoWrap", testee.findPreviousShipAtNoWrap(PA, 10, false, t), 0);

    // - previous, marked
    t = 0;
    a.checkEqual("81. findPreviousShipAtNoWrap", testee.findPreviousShipAtNoWrap(PA, 0, true, t), 30);
    a.checkEqual("82. turn result", t, TURN_NR);

    // - with, wrap
    t = 0;
    a.checkEqual("91. findNextShipAtWrap", testee.findNextShipAtWrap(PA, 30, false, t), 10);
    a.checkEqual("92. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("93. findPreviousShipAtWrap", testee.findPreviousShipAtWrap(PA, 10, false, t), 30);
    a.checkEqual("94. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("95. findNextShipAtWrap", testee.findNextShipAtWrap(PA, 30, true, t), 30);
    a.checkEqual("96. turn result", t, TURN_NR);
    t = 0;
    a.checkEqual("97. findPreviousShipAtWrap", testee.findPreviousShipAtWrap(PA, 30, true, t), 30);
    a.checkEqual("98. turn result", t, TURN_NR);

    t = 0;
    a.checkEqual("101. findNextShipAtWrap", testee.findNextShipAtWrap(PE, 0, false, t), 0);
    a.checkEqual("102. findPreviousShipAtWrap", testee.findPreviousShipAtWrap(PE, 0, false, t), 0);
}
