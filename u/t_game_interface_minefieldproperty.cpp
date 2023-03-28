/**
  *  \file u/t_game_interface_minefieldproperty.cpp
  *  \brief Test for game::interface::MinefieldProperty
  */

#include "game/interface/minefieldproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
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
void
TestGameInterfaceMinefieldProperty::testIt()
{
    game::map::Minefield mf(10);
    mf.addReport(game::map::Point(2000, 3000), 7, game::map::Minefield::IsWeb, game::map::Minefield::UnitsKnown, 400, 15, game::map::Minefield::MinefieldSwept);
    mf.internalCheck(15, game::HostVersion(), game::config::HostConfiguration());

    verifyNewInteger("impId",             getMinefieldProperty(mf, game::interface::impId),             10);
    verifyNewInteger("impLastScan",       getMinefieldProperty(mf, game::interface::impLastScan),       15);
    verifyNewInteger("impLocX",           getMinefieldProperty(mf, game::interface::impLocX),           2000);
    verifyNewInteger("impLocY",           getMinefieldProperty(mf, game::interface::impLocY),           3000);
    verifyNewBoolean("impMarked",         getMinefieldProperty(mf, game::interface::impMarked),         false);
    verifyNewInteger("impRadius",         getMinefieldProperty(mf, game::interface::impRadius),         20);
    verifyNewInteger("impScanType",       getMinefieldProperty(mf, game::interface::impScanType),       2);
    verifyNewBoolean("impTypeCode",       getMinefieldProperty(mf, game::interface::impTypeCode),       true);
    verifyNewString ("impTypeStr",        getMinefieldProperty(mf, game::interface::impTypeStr),        "Web Mines");
    verifyNewInteger("impUnits",          getMinefieldProperty(mf, game::interface::impUnits),          400);

    verifyNewString ("impEncodedMessage", getMinefieldProperty(mf, game::interface::impEncodedMessage),
                     "<<< VPA Data Transmission >>>\n"
                     "\n"
                     "OBJECT: Mine field 10\n"
                     "DATA: 422641678\n"
                     "paaaanhaillahaaaajbaaaaabaaa\n");

    // For now, setting always throws
    afl::data::IntegerValue iv(77);
    TS_ASSERT_THROWS(setMinefieldProperty(mf, game::interface::impRadius, &iv), interpreter::Error);
}

/** Verify properties on empty minefield. */
void
TestGameInterfaceMinefieldProperty::testEmpty()
{
    game::map::Minefield mf(10);

    verifyNewNull("impId",             getMinefieldProperty(mf, game::interface::impId));
    verifyNewNull("impLastScan",       getMinefieldProperty(mf, game::interface::impLastScan));
    verifyNewNull("impLocX",           getMinefieldProperty(mf, game::interface::impLocX));
    verifyNewNull("impLocY",           getMinefieldProperty(mf, game::interface::impLocY));
    verifyNewNull("impTypeCode",       getMinefieldProperty(mf, game::interface::impTypeCode));
}

