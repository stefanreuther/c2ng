/**
  *  \file u/t_game_map_renderlist.cpp
  *  \brief Test for game::map::RenderList
  */

#include "game/map/renderlist.hpp"

#include "t_game_map.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/base/staticassert.hpp"

using game::map::RenderList;
using game::map::Point;

namespace {
    using afl::string::Format;
    class TestListener : public game::map::RendererListener, public afl::test::CallReceiver {
     public:
        TestListener()
            : RendererListener(), CallReceiver("testReplay")
            { }
        virtual void drawGridLine(Point a, Point b)
            { checkCall(Format("drawGridLine(%d,%d,%d,%d)", a.getX(), a.getY(), b.getX(), b.getY())); }
        virtual void drawBorderLine(Point a, Point b)
            { checkCall(Format("drawBorderLine(%d,%d,%d,%d)", a.getX(), a.getY(), b.getX(), b.getY())); }
        virtual void drawBorderCircle(Point c, int r)
            { checkCall(Format("drawBorderCircle(%d,%d,%d)", c.getX(), c.getY(), r)); }
        virtual void drawSelection(Point p)
            { checkCall(Format("drawSelection(%d,%d)", p.getX(), p.getY())); }
        virtual void drawMessageMarker(Point p)
            { checkCall(Format("drawMessageMarker(%d,%d)", p.getX(), p.getY())); }
        virtual void drawPlanet(Point p, int id, int flags, String_t label)
            { checkCall(Format("drawPlanet(%d,%d,%d,%d,%s)") << p.getX() << p.getY() << id << flags << label); }
        virtual void drawShip(Point p, int id, Relation_t rel, int flags, String_t label)
            { checkCall(Format("drawShip(%d,%d,%d,%d,%d,%s)") << p.getX() << p.getY() << id << int(rel) << flags << label); }
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel, bool filled)
            { checkCall(Format("drawMinefield(%d,%d,%d,%d,%d,%d,%d)") << p.getX() << p.getY() << id << r << int(isWeb) << int(rel) << int(filled)); }
        virtual void drawUfo(Point p, int id, int r, int colorCode, int speed, int heading, bool filled)
            { checkCall(Format("drawUfo(%d,%d,%d,%d,%d,%d,%d,%d)") << p.getX() << p.getY() << id << r << colorCode << speed << heading << int(filled)); }
        virtual void drawUfoConnection(Point a, Point b, int colorCode)
            { checkCall(Format("drawUfoConnection(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << colorCode); }
        virtual void drawIonStorm(Point p, int r, int voltage, int speed, int heading, bool filled)
            { checkCall(Format("drawIonStorm(%d,%d,%d,%d,%d,%d,%d)") << p.getX() << p.getY() << r << voltage << speed << heading << int(filled)); }
        virtual void drawUserCircle(Point pt, int r, int color)
            { checkCall(Format("drawUserCircle(%d,%d,%d,%d)", pt.getX(), pt.getY(), r, color)); }
        virtual void drawUserLine(Point a, Point b, int color)
            { checkCall(Format("drawUserLine(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << color); }
        virtual void drawUserRectangle(Point a, Point b, int color)
            { checkCall(Format("drawUserRectangle(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << color); }
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label)
            { checkCall(Format("drawUserMarker(%d,%d,%d,%d,'%s')") << pt.getX() << pt.getY() << shape << color << label); }
        virtual void drawExplosion(Point p)
            { checkCall(Format("drawExplosion(%d,%d)", p.getX(), p.getY())); }
        virtual void drawShipTrail(Point a, Point b, Relation_t rel, int flags, int age)
            { checkCall(Format("drawShipTrail(%d,%d,%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << int(rel) << flags << age); }
        virtual void drawShipWaypoint(Point a, Point b, Relation_t rel)
            { checkCall(Format("drawShipWaypoint(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << int(rel)); }
        virtual void drawShipVector(Point a, Point b, Relation_t rel)
            { checkCall(Format("drawShipVector(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << int(rel)); }
        virtual void drawWarpWellEdge(Point a, Edge e)
            { checkCall(Format("drawWarpWellEdge(%d,%d,%d)", a.getX(), a.getY(), int(e))); }
    };
}

/** Test replay.
    Replay must reproduce the given sequence. */
void
TestGameMapRenderList::testReplay()
{
    static_assert(int(game::TeamSettings::AlliedPlayer) == 1, "AlliedPlayer");
    static_assert(int(game::TeamSettings::EnemyPlayer) == 2, "EnemyPlayer");

    // Prepare RenderList
    RenderList testee;
    TestListener li;
    testee.drawGridLine(Point(3, 4), Point(5, 7));
    li.expectCall("drawGridLine(3,4,5,7)");
    testee.drawBorderLine(Point(99, 88), Point(77, 66));
    li.expectCall("drawBorderLine(99,88,77,66)");
    testee.drawBorderCircle(Point(44, 55), 66);
    li.expectCall("drawBorderCircle(44,55,66)");
    testee.drawSelection(Point(32,54));
    li.expectCall("drawSelection(32,54)");
    testee.drawMessageMarker(Point(55,44));
    li.expectCall("drawMessageMarker(55,44)");
    testee.drawPlanet(Point(200,300), 42, 9, "lab");
    li.expectCall("drawPlanet(200,300,42,9,lab)");
    testee.drawShip(Point(-3,+9), 12, game::TeamSettings::AlliedPlayer, 5, "sh");
    li.expectCall("drawShip(-3,9,12,1,5,sh)");
    testee.drawMinefield(Point(3000, 4000), 498, 5000, true, game::TeamSettings::EnemyPlayer, true);
    li.expectCall("drawMinefield(3000,4000,498,5000,1,2,1)");
    testee.drawUfo(Point(1111, 2222), 77, 250, 4, 6, 135, true);
    li.expectCall("drawUfo(1111,2222,77,250,4,6,135,1)");
    testee.drawUfoConnection(Point(500, 600), Point(700, 800), 5);
    li.expectCall("drawUfoConnection(500,600,700,800,5)");
    testee.drawIonStorm(Point(1200, 1100), 150, 50, 6, 45, true);
    li.expectCall("drawIonStorm(1200,1100,150,50,6,45,1)");
    testee.drawUserCircle(Point(7,8), 100, 3);
    li.expectCall("drawUserCircle(7,8,100,3)");
    testee.drawUserRectangle(Point(101,102), Point(201,202), 7);
    li.expectCall("drawUserRectangle(101,102,201,202,7)");
    testee.drawUserLine(Point(22,33), Point(44,55), 9);
    li.expectCall("drawUserLine(22,33,44,55,9)");
    testee.drawUserMarker(Point(55,77), 17, 29, "hi");
    li.expectCall("drawUserMarker(55,77,17,29,'hi')");
    testee.drawExplosion(Point(42,23));
    li.expectCall("drawExplosion(42,23)");
    testee.drawShipTrail(Point(40,50), Point(20,90), game::TeamSettings::EnemyPlayer, 3, 7);
    li.expectCall("drawShipTrail(40,50,20,90,2,3,7)");
    testee.drawShipWaypoint(Point(9,8), Point(7,6), game::TeamSettings::AlliedPlayer);
    li.expectCall("drawShipWaypoint(9,8,7,6,1)");
    testee.drawShipVector(Point(19,28), Point(37,46), game::TeamSettings::EnemyPlayer);
    li.expectCall("drawShipVector(19,28,37,46,2)");
    testee.drawWarpWellEdge(Point(500,400), game::map::RendererListener::East);
    li.expectCall("drawWarpWellEdge(500,400,1)");

    // Replay and verify
    TS_ASSERT_THROWS_NOTHING(testee.replay(li));
    TS_ASSERT_THROWS_NOTHING(li.checkFinish());
}

/** Test replay.
    Replay must reproduce the given sequence even if called multiple times. */
void
TestGameMapRenderList::testReplayAgain()
{
    RenderList testee;
    testee.drawGridLine(Point(9,8), Point(7,6));
    testee.drawUserLine(Point(50,40), Point(30,20), 10);
    TS_ASSERT(testee.size() >= 2U);

    // Replay once
    {
        TestListener li;
        li.expectCall("drawGridLine(9,8,7,6)");
        li.expectCall("drawUserLine(50,40,30,20,10)");
        TS_ASSERT_THROWS_NOTHING(testee.replay(li));
        TS_ASSERT_THROWS_NOTHING(li.checkFinish());
    }

    // Replay again
    {
        TestListener li;
        li.expectCall("drawGridLine(9,8,7,6)");
        li.expectCall("drawUserLine(50,40,30,20,10)");
        TS_ASSERT_THROWS_NOTHING(testee.replay(li));
        TS_ASSERT_THROWS_NOTHING(li.checkFinish());
    }

    // Clear and replay. Must not produce any output
    testee.clear();
    TS_ASSERT_EQUALS(testee.size(), 0U);
    {
        TestListener li;
        TS_ASSERT_THROWS_NOTHING(testee.replay(li));
        TS_ASSERT_THROWS_NOTHING(li.checkFinish());
    }
}

