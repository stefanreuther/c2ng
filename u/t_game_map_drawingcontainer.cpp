/**
  *  \file u/t_game_map_drawingcontainer.cpp
  *  \brief Test for game::map::DrawingContainer
  */

#include "game/map/drawingcontainer.hpp"

#include "t_game_map.hpp"
#include "game/map/configuration.hpp"
#include "game/test/counter.hpp"
#include "game/parser/messageinformation.hpp"
#include "util/atomtable.hpp"

using game::map::Point;
using game::map::Drawing;
using game::map::DrawingContainer;
using game::parser::MessageInformation;
using util::AtomTable;

namespace {
    Drawing* make(Drawing::Atom_t tag, int expire)
    {
        Drawing* d = new Drawing(Point(1000, 1000), Drawing::MarkerDrawing);
        d->setTag(tag);
        d->setExpire(expire);
        return d;
    }

    Drawing* makeAt(int x, int y, uint8_t color)
    {
        Drawing* d = new Drawing(Point(x, y), Drawing::MarkerDrawing);
        d->setColor(color);
        return d;
    }

    Drawing* makeCircle(int x, int y, uint8_t color)
    {
        Drawing* d = new Drawing(Point(x, y), Drawing::CircleDrawing);
        d->setCircleRadius(10);
        d->setColor(color);
        return d;
    }

    Drawing* makeLine(int x, int y, int x2, int y2, uint8_t color, util::Atom_t tag)
    {
        Drawing* d = new Drawing(Point(x, y), Drawing::LineDrawing);
        d->setPos2(Point(x2, y2));
        d->setColor(color);
        d->setTag(tag);
        return d;
    }

    void checkIncomplete(const char* msg, const MessageInformation& info)
    {
        DrawingContainer t;
        AtomTable atoms;
        TSM_ASSERT_EQUALS(msg, t.checkMessageInformation(info, atoms), DrawingContainer::Invalid);
        t.addMessageInformation(info, atoms);
        TSM_ASSERT_EQUALS(msg, t.begin(), t.end());
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
    (*t.addNew(makeAt(1400, 1100, 1)))
        ->setTag(77);

    // Closest will be (1200,1100) which is 100 ly away. (1200,1200) is not visible.
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(Point(1200, 1200), config, 1e6, afl::base::Nothing);
        TS_ASSERT_DIFFERS(it, t.end());
        TS_ASSERT(*it);
        TS_ASSERT_EQUALS((*it)->getPos().getX(), 1200);
        TS_ASSERT_EQUALS((*it)->getPos().getY(), 1100);
    }

    // No result because maxDistance exceeded
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(Point(1200, 1200), config, 99, afl::base::Nothing);
        TS_ASSERT_EQUALS(it, t.end());
    }

    // With tag filter
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(Point(1200, 1200), config, 1e6, 77);
        TS_ASSERT_DIFFERS(it, t.end());
        TS_ASSERT(*it);
        TS_ASSERT_EQUALS((*it)->getPos().getX(), 1400);
        TS_ASSERT_EQUALS((*it)->getPos().getY(), 1100);
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

    t.eraseAdjacentLines(Point(1000, 1000), config);

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

    t.setAdjacentLinesColor(Point(1000, 1000), 4, config);

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

    t.setAdjacentLinesTag(Point(1000, 1000), 4, config);

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

    DrawingContainer::Iterator_t it = t.findMarkerAt(Point(1000, 1000), afl::base::Nothing);
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((**it).getColor(), 2);

    it = t.findMarkerAt(Point(1000, 1100), afl::base::Nothing);
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((**it).getColor(), 3);

    it = t.findMarkerAt(Point(1000, 1200), afl::base::Nothing);
    TS_ASSERT(!*it);

    it = t.findMarkerAt(Point(1000, 1100), 77);
    TS_ASSERT(!*it);

    it = t.findMarkerAt(Point(1000, 1100), 0);
    TS_ASSERT(*it);
    TS_ASSERT_EQUALS((**it).getColor(), 3);
}

/** Test addMessageInformation, marker. */
void
TestGameMapDrawingContainer::testAddMIMarker()
{
    DrawingContainer t;
    MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
    AtomTable atoms;

    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_Color, 14);
    info.addValue(game::parser::mi_DrawingShape, 3);
    info.addValue(game::parser::ms_DrawingComment, "note");
    info.addValue(game::parser::ms_DrawingTag, "montag");

    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    TS_ASSERT_DIFFERS(t.begin(), t.end());
    TS_ASSERT_EQUALS((*t.begin())->getType(), Drawing::MarkerDrawing);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getX(), 2000);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getY(), 2300);
    TS_ASSERT_EQUALS((*t.begin())->getColor(), 14);
    TS_ASSERT_EQUALS((*t.begin())->getMarkerKind(), 3);
    TS_ASSERT_EQUALS((*t.begin())->getComment(), "note");
    TS_ASSERT_EQUALS(atoms.getStringFromAtom((*t.begin())->getTag()), "montag");
    TS_ASSERT_EQUALS((*t.begin())->getExpire(), 0);
}

/** Test addMessageInformation, line. */
void
TestGameMapDrawingContainer::testAddMILine()
{
    DrawingContainer t;
    MessageInformation info(MessageInformation::LineDrawing, 0, 10);
    AtomTable atoms;

    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_Color, 7);
    info.addValue(game::parser::mi_EndX, 1500);
    info.addValue(game::parser::mi_EndY, 1900);
    info.addValue(game::parser::mi_DrawingExpire, 30);

    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    TS_ASSERT_DIFFERS(t.begin(), t.end());
    TS_ASSERT_EQUALS((*t.begin())->getType(), Drawing::LineDrawing);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getX(), 2000);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getY(), 2300);
    TS_ASSERT_EQUALS((*t.begin())->getPos2().getX(), 1500);
    TS_ASSERT_EQUALS((*t.begin())->getPos2().getY(), 1900);
    TS_ASSERT_EQUALS((*t.begin())->getColor(), 7);
    TS_ASSERT_EQUALS((*t.begin())->getExpire(), 30);
}

/** Test addMessageInformation, rectangle. */
void
TestGameMapDrawingContainer::testAddMIRectangle()
{
    DrawingContainer t;
    MessageInformation info(MessageInformation::RectangleDrawing, 0, 10);
    AtomTable atoms;

    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_Color, 7);
    info.addValue(game::parser::mi_EndX, 2400);
    info.addValue(game::parser::mi_EndY, 1100);

    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    TS_ASSERT_DIFFERS(t.begin(), t.end());
    TS_ASSERT_EQUALS((*t.begin())->getType(), Drawing::RectangleDrawing);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getX(), 2000);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getY(), 2300);
    TS_ASSERT_EQUALS((*t.begin())->getPos2().getX(), 2400);
    TS_ASSERT_EQUALS((*t.begin())->getPos2().getY(), 1100);
    TS_ASSERT_EQUALS((*t.begin())->getColor(), 7);
    TS_ASSERT_EQUALS((*t.begin())->getExpire(), 0);
}

/** Test addMessageInformation, circle. */
void
TestGameMapDrawingContainer::testAddMICircle()
{
    DrawingContainer t;
    MessageInformation info(MessageInformation::CircleDrawing, 0, 10);
    AtomTable atoms;

    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_Color, 9);
    info.addValue(game::parser::mi_Radius, 50);

    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    TS_ASSERT_EQUALS(t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    TS_ASSERT_DIFFERS(t.begin(), t.end());
    TS_ASSERT_EQUALS((*t.begin())->getType(), Drawing::CircleDrawing);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getX(), 2000);
    TS_ASSERT_EQUALS((*t.begin())->getPos().getY(), 2300);
    TS_ASSERT_EQUALS((*t.begin())->getColor(), 9);
    TS_ASSERT_EQUALS((*t.begin())->getCircleRadius(), 50);
}

/** Test addMessageInformation, missing properties. */
void
TestGameMapDrawingContainer::testAddMIMissing()
{
    // Marker, missing X
    {
        MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
        info.addValue(game::parser::mi_Y, 2300);
        info.addValue(game::parser::mi_DrawingShape, 3);
        checkIncomplete("marker no X", info);
    }

    // Marker, missing Y
    {
        MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
        info.addValue(game::parser::mi_X, 1100);
        info.addValue(game::parser::mi_DrawingShape, 3);
        checkIncomplete("marker no Y", info);
    }

    // Marker, missing shape
    {
        MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
        info.addValue(game::parser::mi_X, 1400);
        info.addValue(game::parser::mi_Y, 2300);
        checkIncomplete("marker no shape", info);
    }

    // Marker, bad shape
    {
        MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
        info.addValue(game::parser::mi_X, 1400);
        info.addValue(game::parser::mi_Y, 2300);
        info.addValue(game::parser::mi_DrawingShape, -55);
        checkIncomplete("marker bad shape", info);
    }

    // Line, missing X2
    {
        MessageInformation info(MessageInformation::LineDrawing, 0, 10);
        info.addValue(game::parser::mi_X, 1400);
        info.addValue(game::parser::mi_Y, 2300);
        info.addValue(game::parser::mi_EndY, 2300);
        checkIncomplete("line missing X", info);
    }

    // Line, missing Y2
    {
        MessageInformation info(MessageInformation::LineDrawing, 0, 10);
        info.addValue(game::parser::mi_X, 1400);
        info.addValue(game::parser::mi_Y, 2300);
        info.addValue(game::parser::mi_EndX, 2400);
        checkIncomplete("line missing X", info);
    }

    // Circle, missing radius
    {
        MessageInformation info(MessageInformation::CircleDrawing, 0, 10);
        info.addValue(game::parser::mi_X, 1400);
        info.addValue(game::parser::mi_Y, 2300);
        checkIncomplete("circle missing radius", info);
    }
}

/** Test findDrawing(). */
void
TestGameMapDrawingContainer::testFindDrawing()
{
    // Some markers
    DrawingContainer c;
    c.addNew(new Drawing(Point(1000, 1000), Drawing::MarkerDrawing));
    c.addNew(new Drawing(Point(2000, 1000), Drawing::MarkerDrawing));
    c.addNew(new Drawing(Point(3000, 1000), Drawing::MarkerDrawing));

    // Success case
    DrawingContainer::Iterator_t f1 = c.findDrawing(Drawing(Point(2000, 1000), Drawing::MarkerDrawing));
    TS_ASSERT_DIFFERS(f1, c.end());
    TS_ASSERT_EQUALS((*f1)->getPos().getX(), 2000);

    // Failure case
    DrawingContainer::Iterator_t f2 = c.findDrawing(Drawing(Point(1000, 2000), Drawing::MarkerDrawing));
    TS_ASSERT_EQUALS(f2, c.end());
}

