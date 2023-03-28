/**
  *  \file u/t_game_interface_ufomethod.cpp
  *  \brief Test for game::interface::UfoMethod
  */

#include "game/interface/ufomethod.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "game/map/configuration.hpp"
#include "interpreter/arguments.hpp"

/** Test iumMark/iumUnmark. */
void
TestGameInterfaceUfoMethod::testIt()
{
    game::map::Ufo ufo(51);
    ufo.setColorCode(7);
    ufo.postprocess(42, game::map::Configuration());
    TS_ASSERT(!ufo.isMarked());

    // Mark it using 'Mark'
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callUfoMethod(ufo, game::interface::iumMark, args);
        TS_ASSERT(ufo.isMarked());
    }

    // Unmark it using 'Unmark'
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callUfoMethod(ufo, game::interface::iumUnmark, args);
        TS_ASSERT(!ufo.isMarked());
    }

    // Mark it using 'Mark "X"'
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        callUfoMethod(ufo, game::interface::iumMark, args);
        TS_ASSERT(ufo.isMarked());
    }

    // Unmark it using 'Mark 0'
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        callUfoMethod(ufo, game::interface::iumMark, args);
        TS_ASSERT(!ufo.isMarked());
    }
}

