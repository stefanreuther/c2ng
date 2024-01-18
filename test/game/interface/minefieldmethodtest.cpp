/**
  *  \file test/game/interface/minefieldmethodtest.cpp
  *  \brief Test for game::interface::MinefieldMethod
  */

#include "game/interface/minefieldmethod.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/minefield.hpp"
#include "game/map/universe.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"

/** Test Mark/Unmark. */
AFL_TEST("game.interface.MinefieldMethod:Mark", a)
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
        a.check("01. isMarked", mf.isMarked());
    }

    // Unmark
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callMinefieldMethod(mf, game::interface::immUnmark, args, univ);
        a.check("11. isMarked", !mf.isMarked());
    }
}

/** Test Delete command. */
AFL_TEST("game.interface.MinefieldMethod:Delete", a)
{
    game::map::Universe univ;
    game::map::Minefield& mf = *univ.minefields().create(10);
    mf.addReport(game::map::Point(2000, 3000), 7, game::map::Minefield::IsWeb, game::map::Minefield::UnitsKnown, 400, 15, game::map::Minefield::MinefieldSwept);
    mf.internalCheck(15, game::HostVersion(), game::config::HostConfiguration());

    a.checkEqual("01. countObjects", univ.minefields().countObjects(), 1);

    // Delete, error case (not executed)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(99);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("11. arity error"), callMinefieldMethod(mf, game::interface::immDelete, args, univ), interpreter::Error);
    }

    a.checkEqual("21. countObjects", univ.minefields().countObjects(), 1);

    // Delete
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        callMinefieldMethod(mf, game::interface::immDelete, args, univ);
    }

    a.checkEqual("31. countObjects", univ.minefields().countObjects(), 0);
}
