/**
  *  \file test/game/map/drawingcontainertest.cpp
  *  \brief Test for game::map::DrawingContainer
  */

#include "game/map/drawingcontainer.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/test/counter.hpp"
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

    void checkIncomplete(afl::test::Assert a, const MessageInformation& info)
    {
        DrawingContainer t;
        AtomTable atoms;
        a.checkEqual("checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::Invalid);
        t.addMessageInformation(info, atoms);
        a.check("result empty", t.begin() == t.end());
    }
}

/** Basic tests. */
AFL_TEST("game.map.DrawingContainer:basics", a)
{
    DrawingContainer t;
    t.addNew(make(1000, 10));
    t.addNew(make(1001, 10));

    DrawingContainer::Iterator_t it = t.begin();
    a.check("01. not empty", it != t.end());
    a.checkNonNull("02. not null", *it);
    a.checkEqual("03. getTag", (*it)->getTag(), 1000U);

    ++it;
    a.check("11. not end", it != t.end());
    a.checkNonNull("12. not null", *it);
    a.checkEqual("13. getTag", (*it)->getTag(), 1001U);

    ++it;
    a.check("21. end", it == t.end());
}

/** Test erase and iterators. */
AFL_TEST("game.map.DrawingContainer:erase", a)
{
    DrawingContainer t;
    DrawingContainer::Iterator_t it1 = t.addNew(make(1000, 10));
    DrawingContainer::Iterator_t it2 = t.addNew(make(1001, 10));
    DrawingContainer::Iterator_t it3 = t.addNew(make(1002, 10));

    a.check("01. addNew iterator", *it1);
    a.check("02. addNew iterator", *it2);
    a.check("03. addNew iterator", *it3);

    t.erase(it2);
    a.checkNull("11. null pointer", *it2);

    // NOTE: PtrMultiList has the interesting property that an iterator returned
    // by a pushBackNew-alike operation doesn't "see" elements added after it.
    // We therefore need to obtain a new begin() iterator instead of using it1.
    // Also, compare content, not iterators.
    DrawingContainer::Iterator_t beg = t.begin();
    ++beg;
    a.check("21. skip over deleted", *beg == *it3);
}

/** Test eraseExpiredDrawings and iterator validity. */
AFL_TEST("game.map.DrawingContainer:eraseExpiredDrawings", a)
{
    DrawingContainer t;
    t.addNew(make(1000, 10));
    t.addNew(make(1001, 8));
    t.addNew(make(1002, 8));
    t.addNew(make(1003, 10));

    // Point iterator at second element
    DrawingContainer::Iterator_t it = t.begin();
    ++it;
    a.check("01. not empty", it != t.end());
    a.checkNonNull("02. not null", *it);
    a.checkEqual("03. getTag", (*it)->getTag(), 1001U);

    // Erase expired stuff
    t.eraseExpiredDrawings(9);

    // Iterator now points at null
    a.checkNull("11. null", *it);

    // Iteration produces two elements
    DrawingContainer::Iterator_t it2 = t.begin();
    a.check("21. first", it2 != t.end());
    a.checkNonNull("22. first", *it2);
    a.checkEqual("23. first getTag", (*it2)->getTag(), 1000U);

    ++it2;
    a.check("31. second", it2 != t.end());
    a.checkNonNull("32. second", *it2);
    a.checkEqual("33. second getTag", (*it2)->getTag(), 1003U);

    ++it2;
    a.check("41. end", it2 == t.end());

    // Continuing iteration with it
    ++it;
    a.check("51. not end", it != t.end());
    a.checkNonNull("52. not null", *it);
    a.checkEqual("53. getTag", (*it)->getTag(), 1003U);

    ++it;
    a.check("61. end", it == t.end());
}

/** Test findNearestVisibleDrawing(). */
AFL_TEST("game.map.DrawingContainer:findNearestVisibleDrawing", a)
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
        a.check("01. findNearestVisibleDrawing", it != t.end());
        a.checkNonNull("02. non-null", *it);
        a.checkEqual("03. x", (*it)->getPos().getX(), 1200);
        a.checkEqual("04. y", (*it)->getPos().getY(), 1100);
    }

    // No result because maxDistance exceeded
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(Point(1200, 1200), config, 99, afl::base::Nothing);
        a.check("11. findNearestVisibleDrawing", it == t.end());
    }

    // With tag filter
    {
        DrawingContainer::Iterator_t it = t.findNearestVisibleDrawing(Point(1200, 1200), config, 1e6, 77);
        a.check("21. findNearestVisibleDrawing", it != t.end());
        a.check("22. non-null", *it);
        a.checkEqual("23. x", (*it)->getPos().getX(), 1400);
        a.checkEqual("24. y", (*it)->getPos().getY(), 1100);
    }
}

/** Test eraseAdjacentLines(). */
AFL_TEST("game.map.DrawingContainer:eraseAdjacentLines", a)
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

    a.checkEqual("01. one signal", 1, ctr.get());

    DrawingContainer::Iterator_t it = t.begin();
    a.check("11. not empty", it != t.end());
    a.checkNonNull("12. not null", *it);
    a.checkEqual("13. getType", (**it).getType(), Drawing::LineDrawing);
    a.checkEqual("14. getTag", (**it).getTag(), 4U);

    ++it;
    a.check("21. end", it == t.end());
}

/** Test setAdjacentLinesColor(). */
AFL_TEST("game.map.DrawingContainer:setAdjacentLinesColor", a)
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

    a.check("01. signal", ctr.get() >= 1);

    int n = 0;
    for (DrawingContainer::Iterator_t it = t.begin(); it != t.end(); ++it, ++n) {
        a.checkNonNull("02. not null", *it);
        a.checkEqual("03. getColor", int((**it).getColor()), int((**it).getTag()));
    }
    a.checkEqual("04. count", n, 6);
}

/** Test setAdjacentLinesTag(). */
AFL_TEST("game.map.DrawingContainer:setAdjacentLinesTag", a)
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

    a.check("01. signal", ctr.get() >= 1);

    int n = 0;
    for (DrawingContainer::Iterator_t it = t.begin(); it != t.end(); ++it, ++n) {
        a.checkNonNull("02. not null", *it);
        a.checkEqual("03. getTag", int((**it).getColor()), int((**it).getTag()));
    }
    a.checkEqual("04. count", n, 6);
}

/** Test findMarkerAt. */
AFL_TEST("game.map.DrawingContainer:findMarkerAt", a)
{
    DrawingContainer t;
    t.addNew(makeCircle(1000, 1000, 1));
    t.addNew(makeAt(1000, 1000, 2));
    t.addNew(makeAt(1000, 1100, 3));
    t.addNew(makeCircle(1000, 1200, 4));

    DrawingContainer::Iterator_t it = t.findMarkerAt(Point(1000, 1000), afl::base::Nothing);
    a.check("01. result", *it);
    a.checkEqual("02. getColor", (**it).getColor(), 2);

    it = t.findMarkerAt(Point(1000, 1100), afl::base::Nothing);
    a.check("11. result", *it);
    a.checkEqual("12. getColor", (**it).getColor(), 3);

    it = t.findMarkerAt(Point(1000, 1200), afl::base::Nothing);
    a.check("21. result", !*it);

    it = t.findMarkerAt(Point(1000, 1100), 77);
    a.check("31. result", !*it);

    it = t.findMarkerAt(Point(1000, 1100), 0);
    a.check("41. result", *it);
    a.checkEqual("42. getColor", (**it).getColor(), 3);
}

/** Test addMessageInformation, marker. */
AFL_TEST("game.map.DrawingContainer:addMessageInformation:MarkerDrawing", a)
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

    a.checkEqual("01. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    a.checkEqual("02. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    a.check("11. not empty", t.begin() != t.end());
    a.checkEqual("12. getType",       (*t.begin())->getType(), Drawing::MarkerDrawing);
    a.checkEqual("13. x",             (*t.begin())->getPos().getX(), 2000);
    a.checkEqual("14. y",             (*t.begin())->getPos().getY(), 2300);
    a.checkEqual("15. getColor",      (*t.begin())->getColor(), 14);
    a.checkEqual("16. getMarkerKind", (*t.begin())->getMarkerKind(), 3);
    a.checkEqual("17. getComment",    (*t.begin())->getComment(), "note");
    a.checkEqual("18. getTag",        atoms.getStringFromAtom((*t.begin())->getTag()), "montag");
    a.checkEqual("19. getExpire",     (*t.begin())->getExpire(), 0);
}

/** Test addMessageInformation, line. */
AFL_TEST("game.map.DrawingContainer:addMessageInformation:LineDrawing", a)
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

    a.checkEqual("01. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    a.checkEqual("02. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    a.check("11. not empty", t.begin() != t.end());
    a.checkEqual("12. getType",   (*t.begin())->getType(), Drawing::LineDrawing);
    a.checkEqual("13. x",         (*t.begin())->getPos().getX(), 2000);
    a.checkEqual("14. y",         (*t.begin())->getPos().getY(), 2300);
    a.checkEqual("15. x2",        (*t.begin())->getPos2().getX(), 1500);
    a.checkEqual("16. y2",        (*t.begin())->getPos2().getY(), 1900);
    a.checkEqual("17. getColor",  (*t.begin())->getColor(), 7);
    a.checkEqual("18. getExpire", (*t.begin())->getExpire(), 30);
}

/** Test addMessageInformation, rectangle. */
AFL_TEST("game.map.DrawingContainer:addMessageInformation:RectangleDrawing", a)
{
    DrawingContainer t;
    MessageInformation info(MessageInformation::RectangleDrawing, 0, 10);
    AtomTable atoms;

    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_Color, 7);
    info.addValue(game::parser::mi_EndX, 2400);
    info.addValue(game::parser::mi_EndY, 1100);

    a.checkEqual("01. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    a.checkEqual("02. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    a.check("11. not empty", t.begin() != t.end());
    a.checkEqual("12. getType",   (*t.begin())->getType(), Drawing::RectangleDrawing);
    a.checkEqual("13. x",         (*t.begin())->getPos().getX(), 2000);
    a.checkEqual("14. y",         (*t.begin())->getPos().getY(), 2300);
    a.checkEqual("15. x2",        (*t.begin())->getPos2().getX(), 2400);
    a.checkEqual("16. y2",        (*t.begin())->getPos2().getY(), 1100);
    a.checkEqual("17. getColor",  (*t.begin())->getColor(), 7);
    a.checkEqual("18. getExpire", (*t.begin())->getExpire(), 0);
}

/** Test addMessageInformation, circle. */
AFL_TEST("game.map.DrawingContainer:addMessageInformation:CircleDrawing", a)
{
    DrawingContainer t;
    MessageInformation info(MessageInformation::CircleDrawing, 0, 10);
    AtomTable atoms;

    info.addValue(game::parser::mi_X, 2000);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_Color, 9);
    info.addValue(game::parser::mi_Radius, 50);

    a.checkEqual("01. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::NotFound);
    t.addMessageInformation(info, atoms);
    a.checkEqual("02. checkMessageInformation", t.checkMessageInformation(info, atoms), DrawingContainer::Found);

    a.check("11. not empty", t.begin() != t.end());
    a.checkEqual("12. getType",         (*t.begin())->getType(), Drawing::CircleDrawing);
    a.checkEqual("13. x",               (*t.begin())->getPos().getX(), 2000);
    a.checkEqual("14. y",               (*t.begin())->getPos().getY(), 2300);
    a.checkEqual("15. getColor",        (*t.begin())->getColor(), 9);
    a.checkEqual("16. getCircleRadius", (*t.begin())->getCircleRadius(), 50);
}

/*
 *  addMessageInformation, missing properties
 */

// Marker, missing X
AFL_TEST("game.map.DrawingContainer:addMessageInformation:missing-x", a)
{
    MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_DrawingShape, 3);
    checkIncomplete(a, info);
}

// Marker, missing Y
AFL_TEST("game.map.DrawingContainer:addMessageInformation:missing-y", a)
{
    MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
    info.addValue(game::parser::mi_X, 1100);
    info.addValue(game::parser::mi_DrawingShape, 3);
    checkIncomplete(a, info);
}

// Marker, missing shape
AFL_TEST("game.map.DrawingContainer:addMessageInformation:missing-shape", a)
{
    MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
    info.addValue(game::parser::mi_X, 1400);
    info.addValue(game::parser::mi_Y, 2300);
    checkIncomplete(a, info);
}

// Marker, bad shape
AFL_TEST("game.map.DrawingContainer:addMessageInformation:bad-shape", a)
{
    MessageInformation info(MessageInformation::MarkerDrawing, 0, 10);
    info.addValue(game::parser::mi_X, 1400);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_DrawingShape, -55);
    checkIncomplete(a, info);
}

// Line, missing X2
AFL_TEST("game.map.DrawingContainer:addMessageInformation:missing-x2", a)
{
    MessageInformation info(MessageInformation::LineDrawing, 0, 10);
    info.addValue(game::parser::mi_X, 1400);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_EndY, 2300);
    checkIncomplete(a, info);
}

// Line, missing Y2
AFL_TEST("game.map.DrawingContainer:addMessageInformation:missing-y2", a)
{
    MessageInformation info(MessageInformation::LineDrawing, 0, 10);
    info.addValue(game::parser::mi_X, 1400);
    info.addValue(game::parser::mi_Y, 2300);
    info.addValue(game::parser::mi_EndX, 2400);
    checkIncomplete(a, info);
}

// Circle, missing radius
AFL_TEST("game.map.DrawingContainer:addMessageInformation:missing-radius", a)
{
    MessageInformation info(MessageInformation::CircleDrawing, 0, 10);
    info.addValue(game::parser::mi_X, 1400);
    info.addValue(game::parser::mi_Y, 2300);
    checkIncomplete(a, info);
}

/** Test findDrawing(). */
AFL_TEST("game.map.DrawingContainer:findDrawing", a)
{
    // Some markers
    DrawingContainer c;
    c.addNew(new Drawing(Point(1000, 1000), Drawing::MarkerDrawing));
    c.addNew(new Drawing(Point(2000, 1000), Drawing::MarkerDrawing));
    c.addNew(new Drawing(Point(3000, 1000), Drawing::MarkerDrawing));

    // Success case
    DrawingContainer::Iterator_t f1 = c.findDrawing(Drawing(Point(2000, 1000), Drawing::MarkerDrawing));
    a.check("01. result", f1 != c.end());
    a.checkEqual("02. getX", (*f1)->getPos().getX(), 2000);

    // Failure case
    DrawingContainer::Iterator_t f2 = c.findDrawing(Drawing(Point(1000, 2000), Drawing::MarkerDrawing));
    a.check("11. result", f2 == c.end());
}

