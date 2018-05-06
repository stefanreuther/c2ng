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
        virtual void drawSelection(Point p)
            { checkCall(Format("drawSelection(%d,%d)", p.getX(), p.getY())); }
        virtual void drawPlanet(Point p, int id, int flags)
            { checkCall(Format("drawPlanet(%d,%d,%d,%d)", p.getX(), p.getY(), id, flags)); }
        virtual void drawShip(Point p, int id, Relation_t rel)
            { checkCall(Format("drawShip(%d,%d,%d,%d)", p.getX(), p.getY(), id, int(rel))); }
        virtual void drawFleetLeader(Point pt, int id, Relation_t rel)
            { checkCall(Format("drawFleetLeader(%d,%d,%d,%d)", pt.getX(), pt.getY(), id, int(rel))); }
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel)
            { checkCall(Format("drawMinefield(%d,%d,%d,%d,%d,%d)") << p.getX() << p.getY() << id << r << int(isWeb) << int(rel)); }
        virtual void drawUserCircle(Point pt, int r, int color)
            { checkCall(Format("drawUserCircle(%d,%d,%d,%d)", pt.getX(), pt.getY(), r, color)); }
        virtual void drawUserLine(Point a, Point b, int color)
            { checkCall(Format("drawUserLine(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << color); }
        virtual void drawUserRectangle(Point a, Point b, int color)
            { checkCall(Format("drawUserRectangle(%d,%d,%d,%d,%d)") << a.getX() << a.getY() << b.getX() << b.getY() << color); }
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label)
            { checkCall(Format("drawUserMarker(%d,%d,%d,%d,'%s')") << pt.getX() << pt.getY() << shape << color << label); }
    };
}

/** Test reading an instruction sequence.
    Iterator must provide all parameters. */
void
TestGameMapRenderList::testRead()
{
    RenderList testee;
    testee.drawSelection(Point(200, 300));
    testee.drawUserMarker(Point(555, 666), 3, 7, "text");

    RenderList::Iterator it(testee.read());

    // Selection
    {
        RenderList::Instruction insn;
        TS_ASSERT(it.readInstruction(insn));
        TS_ASSERT_EQUALS(insn, RenderList::riSelection);
        Point pt;
        TS_ASSERT(it.readPointParameter(pt));
        TS_ASSERT_EQUALS(pt.getX(), 200);
        TS_ASSERT_EQUALS(pt.getY(), 300);

        int32_t dummy;
        TS_ASSERT(!it.readParameter(dummy));
    }

    // User marker
    {
        RenderList::Instruction insn;
        TS_ASSERT(it.readInstruction(insn));
        TS_ASSERT_EQUALS(insn, RenderList::riUserMarker);
        Point pt;
        TS_ASSERT(it.readPointParameter(pt));
        TS_ASSERT_EQUALS(pt.getX(), 555);
        TS_ASSERT_EQUALS(pt.getY(), 666);

        int32_t i;
        TS_ASSERT(it.readParameter(i));
        TS_ASSERT_EQUALS(i, 3);
        TS_ASSERT(it.readParameter(i));
        TS_ASSERT_EQUALS(i, 7);

        String_t s;
        TS_ASSERT(it.readStringParameter(s));
        TS_ASSERT_EQUALS(s, "text");

        TS_ASSERT(!it.readParameter(i));
    }

    // End
    {
        RenderList::Instruction insn;
        TS_ASSERT(!it.readInstruction(insn));
    }
}

/** Test reading an instruction sequence without parameters.
    Iterator must correctly provide all instructions even if parameters are not read. */
void
TestGameMapRenderList::testReadInsnOnly()
{
    RenderList testee;
    testee.drawSelection(Point(200, 300));
    testee.drawUserMarker(Point(555, 666), 3, 7, "text");

    RenderList::Iterator it(testee.read());

    // Selection
    RenderList::Instruction insn;
    TS_ASSERT(it.readInstruction(insn));
    TS_ASSERT_EQUALS(insn, RenderList::riSelection);

    // User marker
    TS_ASSERT(it.readInstruction(insn));
    TS_ASSERT_EQUALS(insn, RenderList::riUserMarker);

    // End
    TS_ASSERT(!it.readInstruction(insn));
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
    testee.drawSelection(Point(32,54));
    li.expectCall("drawSelection(32,54)");
    testee.drawPlanet(Point(200,300), 42, 9);
    li.expectCall("drawPlanet(200,300,42,9)");
    testee.drawShip(Point(-3,+9), 12, game::TeamSettings::AlliedPlayer);
    li.expectCall("drawShip(-3,9,12,1)");
    testee.drawFleetLeader(Point(5,6), 7, game::TeamSettings::EnemyPlayer);
    li.expectCall("drawFleetLeader(5,6,7,2)");
    testee.drawMinefield(Point(3000, 4000), 498, 5000, true, game::TeamSettings::EnemyPlayer);
    li.expectCall("drawMinefield(3000,4000,498,5000,1,2)");
    testee.drawUserCircle(Point(7,8), 100, 3);
    li.expectCall("drawUserCircle(7,8,100,3)");
    testee.drawUserRectangle(Point(101,102), Point(201,202), 7);
    li.expectCall("drawUserRectangle(101,102,201,202,7)");
    testee.drawUserLine(Point(22,33), Point(44,55), 9);
    li.expectCall("drawUserLine(22,33,44,55,9)");
    testee.drawUserMarker(Point(55,77), 17, 29, "hi");
    li.expectCall("drawUserMarker(55,77,17,29,'hi')");

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

