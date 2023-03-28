/**
  *  \file u/t_game_interface_drawingmethod.cpp
  *  \brief Test for game::interface::DrawingMethod
  */

#include "game/interface/drawingmethod.hpp"

#include "t_game_interface.hpp"

void
TestGameInterfaceDrawingMethod::testUpdate()
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
    TS_ASSERT_EQUALS((*it)->getColor(), 7);
    TS_ASSERT_EQUALS((*it)->getComment(), "ho");
}

void
TestGameInterfaceDrawingMethod::testDelete()
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
    TS_ASSERT(*it == 0);
    TS_ASSERT(cont.begin() == cont.end());
}

