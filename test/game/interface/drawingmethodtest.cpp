/**
  *  \file test/game/interface/drawingmethodtest.cpp
  *  \brief Test for game::interface::DrawingMethod
  */

#include "game/interface/drawingmethod.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.interface.DrawingMethod:set", a)
{
    game::map::DrawingContainer cont;
    game::map::DrawingContainer::Iterator_t it = cont.addNew(new game::map::Drawing(game::map::Point(1000, 1200), game::map::Drawing::MarkerDrawing));
    (*it)->setColor(5);
    (*it)->setComment("hi");

    // Update
    afl::data::Segment seg1;
    seg1.pushBackInteger(7);
    interpreter::Arguments args1(seg1, 0, 1);
    callDrawingMethod(cont, it, game::interface::idmSetColor, args1);

    afl::data::Segment seg2;
    seg2.pushBackString("ho");
    interpreter::Arguments args2(seg2, 0, 1);
    callDrawingMethod(cont, it, game::interface::idmSetComment, args2);

    // Verify that update has been done
    a.checkEqual("getColor",   (*it)->getColor(), 7);
    a.checkEqual("getComment", (*it)->getComment(), "ho");
}

AFL_TEST("game.interface.DrawingMethod:delete", a)
{
    game::map::DrawingContainer cont;
    game::map::DrawingContainer::Iterator_t it = cont.addNew(new game::map::Drawing(game::map::Point(1000, 1200), game::map::Drawing::MarkerDrawing));
    (*it)->setColor(5);
    (*it)->setComment("hi");

    // Delete it
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    callDrawingMethod(cont, it, game::interface::idmDelete, args);

    // Verify that marker has been deleted
    a.checkNull("iterator null", *it);
    a.check("container empty", cont.begin() == cont.end());
}
