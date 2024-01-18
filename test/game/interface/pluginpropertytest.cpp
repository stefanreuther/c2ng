/**
  *  \file test/game/interface/pluginpropertytest.cpp
  *  \brief Test for game::interface::PluginProperty
  */

#include "game/interface/pluginproperty.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/test/valueverifier.hpp"
#include <memory>

namespace gi = game::interface;

AFL_TEST("game.interface.PluginProperty:getPluginProperty", a)
{
    // Create plug-in
    afl::io::ConstMemoryStream ms(afl::string::toBytes("name = The Name\n"
                                                       "description = Description...\n"));
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    util::plugin::Plugin plug("PLID");
    plug.initFromPluginFile("/base", "pl.c2p", ms, log, tx);

    // Verify
    interpreter::test::verifyNewString(a("ipiId"),            gi::getPluginProperty(plug, gi::ipiId), "PLID");
    interpreter::test::verifyNewString(a("ipiName"),          gi::getPluginProperty(plug, gi::ipiName), "The Name");
    interpreter::test::verifyNewString(a("ipiDescription"),   gi::getPluginProperty(plug, gi::ipiDescription), "Description...");
    interpreter::test::verifyNewString(a("ipiBaseDirectory"), gi::getPluginProperty(plug, gi::ipiBaseDirectory), "/base");
}
