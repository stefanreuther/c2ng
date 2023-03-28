/**
  *  \file u/t_game_interface_minefieldmethod.cpp
  *  \brief Test for game::interface::MinefieldMethod
  */

#include "game/interface/minefieldmethod.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/minefield.hpp"
#include "game/map/universe.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"

/** Test Mark/Unmark. */
void
TestGameInterfaceMinefieldMethod::testMark()
{
    game::map::Universe univ;
    game::map::Minefield& mf = *univ.minefields().create(10);
    mf.addReport(game::map::Point(2000, 3000), 7, game::map::Minefield::IsWeb, game::map::Minefield::UnitsKnown, 400, 15, game::map::Minefield::MinefieldSwept);
    mf.internalCheck(15, game::HostVersion(), game::config::HostConfiguration());

    // Mark
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callMinefieldMethod(mf, game::interface::immMark, args, univ);
        TS_ASSERT(mf.isMarked());
    }

    // Unmark
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callMinefieldMethod(mf, game::interface::immUnmark, args, univ);
        TS_ASSERT(!mf.isMarked());
    }
}

/** Test Delete command. */
void
TestGameInterfaceMinefieldMethod::testDelete()
{
    game::map::Universe univ;
    game::map::Minefield& mf = *univ.minefields().create(10);
    mf.addReport(game::map::Point(2000, 3000), 7, game::map::Minefield::IsWeb, game::map::Minefield::UnitsKnown, 400, 15, game::map::Minefield::MinefieldSwept);
    mf.internalCheck(15, game::HostVersion(), game::config::HostConfiguration());

    TS_ASSERT_EQUALS(univ.minefields().countObjects(), 1);

    // Delete, error case (not executed)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(99);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(callMinefieldMethod(mf, game::interface::immDelete, args, univ), interpreter::Error);
    }

    TS_ASSERT_EQUALS(univ.minefields().countObjects(), 1);

    // Delete
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callMinefieldMethod(mf, game::interface::immDelete, args, univ);
    }

    TS_ASSERT_EQUALS(univ.minefields().countObjects(), 0);
}

