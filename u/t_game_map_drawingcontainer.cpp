/**
  *  \file u/t_game_map_drawingcontainer.cpp
  *  \brief Test for game::map::DrawingContainer
  */

#include "game/map/drawingcontainer.hpp"

#include "t_game_map.hpp"
#include "game/map/configuration.hpp"
#include "game/test/counter.hpp"

using game::map::Drawing;
using game::map::DrawingContainer;

namespace {
    Drawing* make(Drawing::Atom_t tag, int expire)
    {
        Drawing* d = new Drawing(game::map::Point(1000, 1000), Drawing::MarkerDrawing);
        d->setTag(tag);
        d->setExpire(expire);
        return d;
    }

    Drawing* makeAt(int x, int y, uint8_t color)
    {
        Drawing* d = new Drawing(game::map::Point(x, y), Drawing::MarkerDrawing);
        d->setColor(color);
        return d;
    }

    Drawing* makeCircle(int x, int y, uint8_t color)
    {
        Drawing* d = new Drawing(game::map::Point(x, y), Drawing::CircleDrawing);
        d->setCircleRadius(10);
        d->setColor(color);
        return d;
    }

    Drawing* makeLine(int x, int y, int x2, int y2, uint8_t color, util::Atom_t tag)
    {
        Drawing* d = new Drawing(game::map::Point(x, y), Drawing::LineDrawing);
        d->setPos2(game::map::Point(x2, y2));
        d->setColor(color);
        d->setTag(tag);
        return d;
    }
}

/** Basic tests. */
void
TestGameMapDrawingContainer::testIt()
{
    DrawingContainer t;
    t.addNew(make(1000, 10));
    t.addNew(make(1001, 10));

    DrawingContainer::Iterator_t it = t.begin();
    TS_ASSERT_DIFFERS(it, t.end());
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((*it)->getTag(), 1000U);

    ++it;
    TS_ASSERT_DIFFERS(it, t.end());
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((*it)->getTag(), 1001U);

    ++it;
    TS_ASSERT_EQUALS(it, t.end());
}

/** Test erase and iterators. */
void
TestGameMapDrawingContainer::testErase()
{
    DrawingContainer t;
    DrawingContainer::Iterator_t it1 = t.addNew(make(1000, 10));
    DrawingContainer::Iterator_t it2 = t.addNew(make(1001, 10));
    DrawingContainer::Iterator_t it3 = t.addNew(make(1002, 10));

    TS_ASSERT(*it1);
    TS_ASSERT(*it2);
    TS_ASSERT(*it3);

    t.erase(it2);
    TS_ASSERT(!*it2);

    // NOTE: PtrMultiList has the interesting property that an iterator returned
    // by a pushBackNew-alike operation doesn't "see" elements added after it.
    // We therefore need to obtain a new begin() iterator instead of using it1.
    // Also, compare content, not iterators.
    DrawingContainer::Iterator_t beg = t.begin();
    ++beg;
    TS_ASSERT_EQUALS(*beg, *it3);
}

/** Test eraseExpiredDrawings and iterator validity. */
void
TestGameMapDrawingContainer::testEraseExpired()
{
    DrawingContainer t;
    t.addNew(make(1000, 10));
    t.addNew(make(1001, 8));
    t.addNew(make(1002, 8));
    t.addNew(make(1003, 10));

    // Point iterator at second element
    DrawingContainer::Iterator_t it = t.begin();
    ++it;
    TS_ASSERT_DIFFERS(it, t.end());
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((*it)->getTag(), 1001U);

    // Erase expired stuff
    t.eraseExpiredDrawings(9);

    // Iterator now points at null
    TS_ASSERT(!*it);

    // Iteration produces two elements
    DrawingContainer::Iterator_t it2 = t.begin();
    TS_ASSERT_DIFFERS(it2, t.end());
    TS_ASSERT(*it2);
    TS_ASSERT_EQUALS((*it2)->getTag(), 1000U);

    ++it2;
    TS_ASSERT_DIFFERS(it2, t.end());
    TS_ASSERT(*it2);
    TS_ASSERT_EQUALS((*it2)->getTag(), 1003U);

    ++it2;
    TS_ASSERT_EQUALS(it2, t.end());

    // Continuing iteration with it
    ++it;
    TS_ASSERT_DIFFERS(it, t.end());
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((*it)->getTag(), 1003U);

    ++it;
    TS_ASSERT_EQUALS(it, t.end());
}

/** Test findNearestVisibleDrawing(). */
void
TestGameMapDrawingContainer::testFindNearest()
{
    game::map::Configuration config;

    DrawingContainer t;
    t.addNew(makeAt(1000, 1000, 1));
    t.addNew(makeAt(1100, 1100, 1));
    t.addNew(makeAt(1200, 1200, 0));
    t.addNew(makeAt(1200, 1100, 1));
    t.addNew(makeAt(1400, 1100, 1));

    // Closest will be (1200,1100) which is 100 ly away. (1200,1200) is not visible.
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(game::map::Point(1200, 1200), config, 1e6);
        TS_ASSERT_DIFFERS(it, t.end());
        TS_ASSERT(*it);
        TS_ASSERT_EQUALS((*it)->getPos().getX(), 1200);
        TS_ASSERT_EQUALS((*it)->getPos().getY(), 1100);
    }

    // No result because maxDistance exceeded
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(game::map::Point(1200, 1200), config, 99);
        TS_ASSERT_EQUALS(it, t.end());
    }
}

/** Test eraseAdjacentLines(). */
void
TestGameMapDrawingContainer::testEraseAdjacent()
{
    game::map::Configuration config;
    game::test::Counter ctr;

    DrawingContainer t;
    t.addNew(makeLine(1000, 1000, 1000, 1010, 9, 1));     // A > B
    t.addNew(makeLine(1000, 1010, 1000, 1020, 9, 2));     // B > C
    t.addNew(makeLine(1000, 1020, 1000, 1030, 9, 3));     // C > D
    t.addNew(makeLine(1000, 1020, 1010, 1020, 9, 4));     // C > E (fork). This one will remain.
    t.addNew(makeLine(1000, 1040, 1000, 1030, 9, 5));     // F > D (backward)
    t.sig_change.add(&ctr, &game::test::Counter::increment);

    t.eraseAdjacentLines(game::map::Point(1000, 1000), config);

    TS_ASSERT_EQUALS(1, ctr.get());

    DrawingContainer::Iterator_t it = t.begin();
    TS_ASSERT_DIFFERS(it, t.end());
    TS_ASSERT(*it != 0);
    TS_ASSERT_EQUALS((**it).getType(), Drawing::LineDrawing);
    TS_ASSERT_EQUALS((**it).getTag(), 4U);

    ++it;
    TS_ASSERT_EQUALS(it, t.end());
}

/** Test setAdjacentLinesColor(). */
void
TestGameMapDrawingContainer::testColorAdjacent()
{
    game::map::Configuration config;
    game::test::Counter ctr;

    // For simplicity, we use the 'tag' slot as target color
    DrawingContainer t;
    t.addNew(makeLine(1000, 1010, 1000, 1020, 9, 4));    // B > C
    t.addNew(makeLine(1000, 1000, 1000, 1010, 7, 4));    // A > B
    t.addNew(makeLine(1000, 1020, 1000, 1030, 4, 4));    // C > D (already final color)
    t.addNew(makeLine(1000, 1020, 1010, 1020, 8, 4));    // C > E
    t.addNew(makeLine(1000, 1040, 1000, 1030, 9, 9));    // D > F (will not be reached because only adjacent to different color)
    t.addNew(makeLine(1010, 1040, 1010, 1020, 8, 4));    // G > E (reverse)
    t.sig_change.add(&ctr, &game::test::Counter::increment);

    t.setAdjacentLinesColor(game::map::Point(1000, 1000), 4, config);

    TS_ASSERT_LESS_THAN_EQUALS(1, ctr.get());

    int n = 0;
    for (DrawingContainer::Iterator_t it = t.begin(); it != t.end(); ++it, ++n) {
        TS_ASSERT(*it != 0);
        TS_ASSERT_EQUALS(int((**it).getColor()), int((**it).getTag()));
    }
    TS_ASSERT_EQUALS(n, 6);
}

/** Test setAdjacentLinesTag(). */
void
TestGameMapDrawingContainer::testTagAdjacent()
{
    game::map::Configuration config;
    game::test::Counter ctr;

    // For now, same test case as testColorAdjacent(), with the roles of tag/color swapped
    DrawingContainer t;
    t.addNew(makeLine(1000, 1010, 1000, 1020, 4, 9));    // B > C
    t.addNew(makeLine(1000, 1000, 1000, 1010, 4, 7));    // A > B
    t.addNew(makeLine(1000, 1020, 1000, 1030, 4, 4));    // C > D (already final tag)
    t.addNew(makeLine(1000, 1020, 1010, 1020, 4, 8));    // C > E
    t.addNew(makeLine(1000, 1040, 1000, 1030, 9, 9));    // D > F (will not be reached because only adjacent to different tag)
    t.addNew(makeLine(1010, 1040, 1010, 1020, 4, 8));    // G > E (reverse)
    t.sig_change.add(&ctr, &game::test::Counter::increment);

    t.setAdjacentLinesTag(game::map::Point(1000, 1000), 4, config);

    TS_ASSERT_LESS_THAN_EQUALS(1, ctr.get());

    int n = 0;
    for (DrawingContainer::Iterator_t it = t.begin(); it != t.end(); ++it, ++n) {
        TS_ASSERT(*it != 0);
        TS_ASSERT_EQUALS(int((**it).getColor()), int((**it).getTag()));
    }
    TS_ASSERT_EQUALS(n, 6);
}

/** Test findMarkerAt. */
void
TestGameMapDrawingContainer::testFindMarker()
{
    DrawingContainer t;
    t.addNew(makeCircle(1000, 1000, 1));
    t.addNew(makeAt(1000, 1000, 2));
    t.addNew(makeAt(1000, 1100, 3));
    t.addNew(makeCircle(1000, 1200, 4));

    DrawingContainer::Iterator_t it = t.findMarkerAt(game::map::Point(1000, 1000));
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((**it).getColor(), 2);

    it = t.findMarkerAt(game::map::Point(1000, 1100));
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((**it).getColor(), 3);

    it = t.findMarkerAt(game::map::Point(1000, 1200));
    TS_ASSERT(!*it);
}

