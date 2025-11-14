/**
  *  \file test/game/interface/minefieldpropertytest.cpp
  *  \brief Test for game::interface::MinefieldProperty
  */

#include "game/interface/minefieldproperty.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"
#include "game/map/minefield.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** Verify properties on valid minefield. */
AFL_TEST("game.interface.MinefieldProperty:full", a)
{
    game::map::Minefield mf(10);
    mf.addReport(game::map::Point(2000, 3000), 7, game::map::Minefield::IsWeb, game::map::Minefield::UnitsKnown, 400, 15, game::map::Minefield::MinefieldSwept);
    mf.internalCheck(15, game::HostVersion(), *game::config::HostConfiguration::create());

    verifyNewInteger(a("impId"),             getMinefieldProperty(mf, game::interface::impId),             10);
    verifyNewInteger(a("impLastScan"),       getMinefieldProperty(mf, game::interface::impLastScan),       15);
    verifyNewInteger(a("impLocX"),           getMinefieldProperty(mf, game::interface::impLocX),           2000);
    verifyNewInteger(a("impLocY"),           getMinefieldProperty(mf, game::interface::impLocY),           3000);
    verifyNewBoolean(a("impMarked"),         getMinefieldProperty(mf, game::interface::impMarked),         false);
    verifyNewInteger(a("impRadius"),         getMinefieldProperty(mf, game::interface::impRadius),         20);
    verifyNewInteger(a("impScanType"),       getMinefieldProperty(mf, game::interface::impScanType),       2);
    verifyNewBoolean(a("impTypeCode"),       getMinefieldProperty(mf, game::interface::impTypeCode),       true);
    verifyNewString (a("impTypeStr"),        getMinefieldProperty(mf, game::interface::impTypeStr),        "Web Mines");
    verifyNewInteger(a("impUnits"),          getMinefieldProperty(mf, game::interface::impUnits),          400);

    verifyNewString (a("impEncodedMessage"), getMinefieldProperty(mf, game::interface::impEncodedMessage),
                     "<<< VPA Data Transmission >>>\n"
                     "\n"
                     "OBJECT: Mine field 10\n"
                     "DATA: 422641678\n"
                     "paaaanhaillahaaaajbaaaaabaaa\n");

    // For now, setting always throws
    afl::data::IntegerValue iv(77);
    AFL_CHECK_THROWS(a("set"), setMinefieldProperty(mf, game::interface::impRadius, &iv), interpreter::Error);
}

/** Verify properties on empty minefield. */
AFL_TEST("game.interface.MinefieldProperty:empty", a)
{
    game::map::Minefield mf(10);

    verifyNewNull(a("impId"),             getMinefieldProperty(mf, game::interface::impId));
    verifyNewNull(a("impLastScan"),       getMinefieldProperty(mf, game::interface::impLastScan));
    verifyNewNull(a("impLocX"),           getMinefieldProperty(mf, game::interface::impLocX));
    verifyNewNull(a("impLocY"),           getMinefieldProperty(mf, game::interface::impLocY));
    verifyNewNull(a("impTypeCode"),       getMinefieldProperty(mf, game::interface::impTypeCode));
}
