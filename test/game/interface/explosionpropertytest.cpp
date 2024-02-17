/**
  *  \file test/game/interface/explosionpropertytest.cpp
  *  \brief Test for game::interface::ExplosionProperty
  */

#include "game/interface/explosionproperty.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

AFL_TEST("game.interface.ExplosionProperty:1", a)
{
    afl::string::NullTranslator tx;

    game::map::Explosion expl(99, game::map::Point(2000, 3000));
    expl.setShipName("Boomer");
    expl.setShipId(42);

    verifyNewInteger(a("iepId"),       getExplosionProperty(expl, game::interface::iepId,       tx), 99);
    verifyNewInteger(a("iepShipId"),   getExplosionProperty(expl, game::interface::iepShipId,   tx), 42);
    verifyNewString (a("iepShipName"), getExplosionProperty(expl, game::interface::iepShipName, tx), "Boomer");
    verifyNewInteger(a("iepLocX"),     getExplosionProperty(expl, game::interface::iepLocX,     tx), 2000);
    verifyNewInteger(a("iepLocY"),     getExplosionProperty(expl, game::interface::iepLocY,     tx), 3000);
    verifyNewString (a("iepName"),     getExplosionProperty(expl, game::interface::iepName,     tx), "Explosion of Boomer (#42)");
    verifyNewString (a("iepTypeStr"),  getExplosionProperty(expl, game::interface::iepTypeStr,  tx), "Explosion");
    verifyNewString (a("iepTypeChar"), getExplosionProperty(expl, game::interface::iepTypeChar, tx), "E");
}

AFL_TEST("game.interface.ExplosionProperty:2", a)
{
    afl::string::NullTranslator tx;

    game::map::Explosion expl(99, game::map::Point(2000, 3000));

    verifyNewInteger(a("iepId"),       getExplosionProperty(expl, game::interface::iepId,       tx), 99);
    verifyNewInteger(a("iepShipId"),   getExplosionProperty(expl, game::interface::iepShipId,   tx), 0);
    verifyNewString (a("iepShipName"), getExplosionProperty(expl, game::interface::iepShipName, tx), "");
    verifyNewInteger(a("iepLocX"),     getExplosionProperty(expl, game::interface::iepLocX,     tx), 2000);
    verifyNewInteger(a("iepLocY"),     getExplosionProperty(expl, game::interface::iepLocY,     tx), 3000);
    verifyNewString (a("iepName"),     getExplosionProperty(expl, game::interface::iepName,     tx), "Explosion");
    verifyNewString (a("iepTypeStr"),  getExplosionProperty(expl, game::interface::iepTypeStr,  tx), "Explosion");
    verifyNewString (a("iepTypeChar"), getExplosionProperty(expl, game::interface::iepTypeChar, tx), "E");
}
