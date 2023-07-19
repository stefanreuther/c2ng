/**
  *  \file u/t_gfx_threed_positionlist.cpp
  *  \brief Test for gfx::threed::PositionList
  */

#include <cstdio>
#include "gfx/threed/positionlist.hpp"

#include "t_gfx_threed.hpp"
#include "afl/base/memory.hpp"

using gfx::threed::Vec3f;

namespace {
    /* Canned test case for findPoints() */
    void checkPositions(const gfx::threed::PositionList& pl,
                        const gfx::threed::PositionList::Definition& def,
                        size_t n,
                        afl::base::Memory<const float> expect)
    {
        char tmp[100];
        std::sprintf(tmp, "(ask %d, expect %d)", int(n), int(expect.size()));
        std::vector<Vec3f> res = pl.findPoints(def, n);
        TSM_ASSERT_EQUALS(tmp, res.size(), expect.size());
        for (size_t i = 0; i < expect.size(); ++i) {
            std::sprintf(tmp, "(ask %d, expect %d, slot %d)", int(n), int(expect.size()), int(i));
            TSM_ASSERT_EQUALS(tmp, res[i](0), *expect.at(i));
        }
    }
}


/** Test initial state. */
void
TestGfxThreedPositionList::testInit()
{
    gfx::threed::PositionList testee;
    TS_ASSERT_EQUALS(testee.getNumPositions(), 0U);
    TS_ASSERT_EQUALS(testee.getIdByIndex(0), 0U);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(0)(0), 0);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(0)(1), 0);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(0)(2), 0);
}


/** Test general access. */
void
TestGfxThreedPositionList::testAccess()
{
    gfx::threed::PositionList testee;
    testee.add(1, Vec3f(1,     2, 3));
    testee.add(2, Vec3f(10,    2, 3));
    testee.add(3, Vec3f(100,   2, 3));
    testee.add(1, Vec3f(1000,  2, 3));
    testee.add(4, Vec3f(10000, 2, 3));

    TS_ASSERT_EQUALS(testee.getNumPositions(), 5U);

    TS_ASSERT_EQUALS(testee.getIdByIndex(0), 1U);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(0)(0), 1);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(0)(1), 2);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(0)(2), 3);

    TS_ASSERT_EQUALS(testee.getIdByIndex(2), 3U);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(2)(0), 100);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(2)(1), 2);
    TS_ASSERT_EQUALS(testee.getPositionByIndex(2)(2), 3);

    TS_ASSERT_EQUALS(testee.findId(1).orElse(77),    0U);
    TS_ASSERT_EQUALS(testee.findId(1, 1).orElse(77), 3U);
    TS_ASSERT_EQUALS(testee.findId(1, 4).orElse(77), 77U);
}

/** Test findPoints() with two interpolatable ranges and three individual mountpoints.
    This could be a "wing" type ship with a mountpoint at the wingtips, one at the cockpit, and two beam batteries. */
void
TestGfxThreedPositionList::testFindPointsTwoRanges()
{
    // Define: <beam> <range> <beam> <range> <beam>
    gfx::threed::PositionList testee;
    testee.add(30, Vec3f(1, 0, 0));
    testee.add(40, Vec3f(10, 0, 0));
    testee.add(41, Vec3f(20, 0, 0));
    testee.add(30, Vec3f(100, 0, 0));
    testee.add(40, Vec3f(110, 0, 0));
    testee.add(41, Vec3f(120, 0, 0));
    testee.add(30, Vec3f(200, 0, 0));

    const gfx::threed::PositionList::Definition def = { 30, 40, 41 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());

    // Single beam
    {
        static const float rs[] = { 100 };
        checkPositions(testee, def, 1, rs);
    }

    // 2 beams
    {
        static const float rs[] = { 1, 200 };
        checkPositions(testee, def, 2, rs);
    }

    // 3 beams
    {
        static const float rs[] = { 1, 100, 200 };
        checkPositions(testee, def, 3, rs);
    }

    // 4 beams
    {
        static const float rs[] = { 1, 15, 115, 200 };
        checkPositions(testee, def, 4, rs);
    }

    // 5 beams
    {
        static const float rs[] = { 1, 15, 100, 115, 200 };
        checkPositions(testee, def, 5, rs);
    }

    // 6 beams
    {
        static const float rs[] = { 1, 10, 20, 110, 120, 200 };
        checkPositions(testee, def, 6, rs);
    }

    // 7 beams
    {
        static const float rs[] = { 1, 10, 20, 100, 110, 120, 200 };
        checkPositions(testee, def, 7, rs);
    }

    // 8 beams
    {
        static const float rs[] = { 1, 10, 15, 20, 110, 115, 120, 200 };
        checkPositions(testee, def, 8, rs);
    }

    // 9 beams
    {
        static const float rs[] = { 1, 10, 15, 20, 100, 110, 115, 120, 200 };
        checkPositions(testee, def, 9, rs);
    }
}

/** Test findPoints() with one interpolatable range and two individual mountpoints.
    This could be a ship with a larger bridge. */
void
TestGfxThreedPositionList::testFindPointsOneRange()
{
    // Define: <beam> <range> <beam>
    gfx::threed::PositionList testee;
    testee.add(30, Vec3f(1, 0, 0));
    testee.add(40, Vec3f(10, 0, 0));
    testee.add(41, Vec3f(20, 0, 0));
    testee.add(30, Vec3f(100, 0, 0));

    const gfx::threed::PositionList::Definition def = { 30, 40, 41 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());

    // Single beam
    {
        static const float rs[] = { 1 };   // 15 would be better!
        checkPositions(testee, def, 1, rs);
    }

    // 2 beams
    {
        static const float rs[] = { 1, 100 };
        checkPositions(testee, def, 2, rs);
    }

    // 3 beams
    {
        static const float rs[] = { 1, 15, 100 };
        checkPositions(testee, def, 3, rs);
    }

    // 4 beams
    {
        static const float rs[] = { 1, 10, 20, 100 };
        checkPositions(testee, def, 4, rs);
    }

    // 5 beams
    {
        static const float rs[] = { 1, 10, 15, 20, 100 };
        checkPositions(testee, def, 5, rs);
    }

    // 7 beams
    {
        static const float rs[] = { 1, 10, 12.5, 15, 17.5, 20, 100 };
        checkPositions(testee, def, 7, rs);
    }
}

/** Test findPoints() with just a single range.
    This could be a simple ship providing just the bare minimum metainformation. */
void
TestGfxThreedPositionList::testFindPointsSingleRange()
{
    // Define a single range
    gfx::threed::PositionList testee;
    testee.add(30, Vec3f(10, 0, 0));
    testee.add(31, Vec3f(100, 0, 0));

    const gfx::threed::PositionList::Definition def = { 7, 30, 31 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());

    // Single beam
    {
        static const float rs[] = { 55 };
        checkPositions(testee, def, 1, rs);
    }

    // 2 beams
    {
        static const float rs[] = { 10, 100 };
        checkPositions(testee, def, 2, rs);
    }

    // 3 beams
    {
        static const float rs[] = { 10, 55, 100 };
        checkPositions(testee, def, 3, rs);
    }

    // 4 beams
    {
        static const float rs[] = { 10, 40, 70, 100 };
        checkPositions(testee, def, 4, rs);
    }

    // 5 beams
    {
        static const float rs[] = { 10, 32.5, 55, 77.5, 100 };
        checkPositions(testee, def, 5, rs);
    }
}

/** Test findPoints() with just an odd number of fixed points and no ranges. */
void
TestGfxThreedPositionList::testFindPointsFixedOnlyOdd()
{
    gfx::threed::PositionList testee;
    testee.add(30, Vec3f(10, 0, 0));
    testee.add(30, Vec3f(20, 0, 0));
    testee.add(30, Vec3f(30, 0, 0));
    testee.add(30, Vec3f(40, 0, 0));
    testee.add(30, Vec3f(50, 0, 0));

    const gfx::threed::PositionList::Definition def = { 30, 31, 32 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());

    // Single beam
    {
        static const float rs[] = { 30 };
        checkPositions(testee, def, 1, rs);
    }

    // 2 beams
    {
        static const float rs[] = { 10, 50 };
        checkPositions(testee, def, 2, rs);
    }

    // 3 beams
    {
        static const float rs[] = { 20, 30, 40 };
        checkPositions(testee, def, 3, rs);
    }

    // 4 beams
    {
        static const float rs[] = { 10, 20, 40, 50 };
        checkPositions(testee, def, 4, rs);
    }

    // 5 beams
    {
        static const float rs[] = { 10, 20, 30, 40, 50 };
        checkPositions(testee, def, 5, rs);
    }

    // 6 beams - only 5 returned
    {
        static const float rs[] = { 10, 20, 30, 40, 50 };
        checkPositions(testee, def, 6, rs);
    }
}

/** Test findPoints() with just an even number of fixed points and no ranges. */
void
TestGfxThreedPositionList::testFindPointsFixedOnlyEven()
{
    gfx::threed::PositionList testee;
    testee.add(30, Vec3f(10, 0, 0));
    testee.add(30, Vec3f(20, 0, 0));
    testee.add(30, Vec3f(30, 0, 0));
    testee.add(30, Vec3f(40, 0, 0));

    const gfx::threed::PositionList::Definition def = { 30, 31, 32 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());

    // Single beam
    {
        static const float rs[] = { 20 };
        checkPositions(testee, def, 1, rs);
    }

    // 2 beams
    {
        static const float rs[] = { 20, 30 };
        checkPositions(testee, def, 2, rs);
    }

    // 3 beams
    {
        static const float rs[] = { 10, 30, 40 };
        checkPositions(testee, def, 3, rs);
    }

    // 4 beams
    {
        static const float rs[] = { 10, 20, 30, 40 };
        checkPositions(testee, def, 4, rs);
    }

    // 5 beams - only 4 returned
    {
        static const float rs[] = { 10, 20, 30, 40 };
        checkPositions(testee, def, 5, rs);
    }
}

/** Test findPoints() with no points. */
void
TestGfxThreedPositionList::testFindPointsEmpty()
{
    gfx::threed::PositionList testee;

    const gfx::threed::PositionList::Definition def = { 30, 31, 32 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());
    TS_ASSERT(testee.findPoints(def, 1).empty());
    TS_ASSERT(testee.findPoints(def, 2).empty());
    TS_ASSERT(testee.findPoints(def, 3).empty());
    TS_ASSERT(testee.findPoints(def, 4).empty());
}

/** Test findPoints() with just a single point. */
void
TestGfxThreedPositionList::testFindPointsOne()
{
    gfx::threed::PositionList testee;
    testee.add(30, Vec3f(10, 0, 0));

    const gfx::threed::PositionList::Definition def = { 30, 31, 32 };

    // No beams
    TS_ASSERT(testee.findPoints(def, 0).empty());

    // Single beam
    {
        static const float rs[] = { 10 };
        checkPositions(testee, def, 1, rs);
    }

    // 2 beams
    {
        static const float rs[] = { 10 };
        checkPositions(testee, def, 2, rs);
    }
}
