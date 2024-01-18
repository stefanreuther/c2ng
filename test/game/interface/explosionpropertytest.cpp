/**
  *  \file test/game/interface/explosionpropertytest.cpp
  *  \brief Test for game::interface::ExplosionProperty
  */

#include "game/interface/explosionproperty.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/interpreterinterface.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

AFL_TEST("game.interface.ExplosionProperty:1", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    game::map::Explosion expl(99, game::map::Point(2000, 3000));
    expl.setShipName("Boomer");
    expl.setShipId(42);

    verifyNewInteger(a("iepId"),       getExplosionProperty(expl, game::interface::iepId,       tx, iface), 99);
    verifyNewInteger(a("iepShipId"),   getExplosionProperty(expl, game::interface::iepShipId,   tx, iface), 42);
    verifyNewString (a("iepShipName"), getExplosionProperty(expl, game::interface::iepShipName, tx, iface), "Boomer");
    verifyNewInteger(a("iepLocX"),     getExplosionProperty(expl, game::interface::iepLocX,     tx, iface), 2000);
    verifyNewInteger(a("iepLocY"),     getExplosionProperty(expl, game::interface::iepLocY,     tx, iface), 3000);
    verifyNewString (a("iepName"),     getExplosionProperty(expl, game::interface::iepName,     tx, iface), "Explosion of Boomer (#42)");
    verifyNewString (a("iepTypeStr"),  getExplosionProperty(expl, game::interface::iepTypeStr,  tx, iface), "Explosion");
    verifyNewString (a("iepTypeChar"), getExplosionProperty(expl, game::interface::iepTypeChar, tx, iface), "E");
}

AFL_TEST("game.interface.ExplosionProperty:2", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    game::map::Explosion expl(99, game::map::Point(2000, 3000));

    verifyNewInteger(a("iepId"),       getExplosionProperty(expl, game::interface::iepId,       tx, iface), 99);
    verifyNewInteger(a("iepShipId"),   getExplosionProperty(expl, game::interface::iepShipId,   tx, iface), 0);
    verifyNewString (a("iepShipName"), getExplosionProperty(expl, game::interface::iepShipName, tx, iface), "");
    verifyNewInteger(a("iepLocX"),     getExplosionProperty(expl, game::interface::iepLocX,     tx, iface), 2000);
    verifyNewInteger(a("iepLocY"),     getExplosionProperty(expl, game::interface::iepLocY,     tx, iface), 3000);
    verifyNewString (a("iepName"),     getExplosionProperty(expl, game::interface::iepName,     tx, iface), "Explosion");
    verifyNewString (a("iepTypeStr"),  getExplosionProperty(expl, game::interface::iepTypeStr,  tx, iface), "Explosion");
    verifyNewString (a("iepTypeChar"), getExplosionProperty(expl, game::interface::iepTypeChar, tx, iface), "E");
}
