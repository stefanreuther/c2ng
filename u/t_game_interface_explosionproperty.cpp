/**
  *  \file u/t_game_interface_explosionproperty.cpp
  *  \brief Test for game::interface::ExplosionProperty
  */

#include "game/interface/explosionproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/interpreterinterface.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

void
TestGameInterfaceExplosionProperty::testIt()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    game::map::Explosion expl(99, game::map::Point(2000, 3000));
    expl.setShipName("Boomer");
    expl.setShipId(42);

    verifyNewInteger("iepId",       getExplosionProperty(expl, game::interface::iepId,       tx, iface), 99);
    verifyNewInteger("iepShipId",   getExplosionProperty(expl, game::interface::iepShipId,   tx, iface), 42);
    verifyNewString ("iepShipName", getExplosionProperty(expl, game::interface::iepShipName, tx, iface), "Boomer");
    verifyNewInteger("iepLocX",     getExplosionProperty(expl, game::interface::iepLocX,     tx, iface), 2000);
    verifyNewInteger("iepLocY",     getExplosionProperty(expl, game::interface::iepLocY,     tx, iface), 3000);
    verifyNewString ("iepName",     getExplosionProperty(expl, game::interface::iepName,     tx, iface), "Explosion of Boomer (#42)");
    verifyNewString ("iepTypeStr",  getExplosionProperty(expl, game::interface::iepTypeStr,  tx, iface), "Explosion");
    verifyNewString ("iepTypeChar", getExplosionProperty(expl, game::interface::iepTypeChar, tx, iface), "E");
}

void
TestGameInterfaceExplosionProperty::testIt2()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    game::map::Explosion expl(99, game::map::Point(2000, 3000));

    verifyNewInteger("iepId",       getExplosionProperty(expl, game::interface::iepId,       tx, iface), 99);
    verifyNewInteger("iepShipId",   getExplosionProperty(expl, game::interface::iepShipId,   tx, iface), 0);
    verifyNewString ("iepShipName", getExplosionProperty(expl, game::interface::iepShipName, tx, iface), "");
    verifyNewInteger("iepLocX",     getExplosionProperty(expl, game::interface::iepLocX,     tx, iface), 2000);
    verifyNewInteger("iepLocY",     getExplosionProperty(expl, game::interface::iepLocY,     tx, iface), 3000);
    verifyNewString ("iepName",     getExplosionProperty(expl, game::interface::iepName,     tx, iface), "Explosion");
    verifyNewString ("iepTypeStr",  getExplosionProperty(expl, game::interface::iepTypeStr,  tx, iface), "Explosion");
    verifyNewString ("iepTypeChar", getExplosionProperty(expl, game::interface::iepTypeChar, tx, iface), "E");
}

