/**
  *  \file test/game/interface/ufomethodtest.cpp
  *  \brief Test for game::interface::UfoMethod
  */

#include "game/interface/ufomethod.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "interpreter/arguments.hpp"

/** Test iumMark/iumUnmark. */
AFL_TEST("game.interface.UfoMethod", a)
{
    game::map::Ufo ufo(51);
    ufo.setColorCode(7);
    ufo.postprocess(42, game::map::Configuration());
    a.check("01", !ufo.isMarked());

    // Mark it using 'Mark'
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callUfoMethod(ufo, game::interface::iumMark, args);
        a.check("11", ufo.isMarked());
    }

    // Unmark it using 'Unmark'
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callUfoMethod(ufo, game::interface::iumUnmark, args);
        a.check("21", !ufo.isMarked());
    }

    // Mark it using 'Mark "X"'
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        callUfoMethod(ufo, game::interface::iumMark, args);
        a.check("31", ufo.isMarked());
    }

    // Unmark it using 'Mark 0'
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        callUfoMethod(ufo, game::interface::iumMark, args);
        a.check("41", !ufo.isMarked());
    }
}
