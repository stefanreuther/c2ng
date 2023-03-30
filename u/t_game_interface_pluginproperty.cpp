/**
  *  \file u/t_game_interface_pluginproperty.cpp
  *  \brief Test for game::interface::PluginProperty
  */

#include <memory>
#include "game/interface/pluginproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/test/valueverifier.hpp"

namespace gi = game::interface;

void
TestGameInterfacePluginProperty::testGet()
{
    // Create plug-in
    afl::io::ConstMemoryStream ms(afl::string::toBytes("name = The Name\n"
                                                       "description = Description...\n"));
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    util::plugin::Plugin plug("PLID");
    plug.initFromPluginFile("/base", "pl.c2p", ms, log, tx);

    // Verify
    interpreter::test::verifyNewString("ipiId",            gi::getPluginProperty(plug, gi::ipiId), "PLID");
    interpreter::test::verifyNewString("ipiName",          gi::getPluginProperty(plug, gi::ipiName), "The Name");
    interpreter::test::verifyNewString("ipiDescription",   gi::getPluginProperty(plug, gi::ipiDescription), "Description...");
    interpreter::test::verifyNewString("ipiBaseDirectory", gi::getPluginProperty(plug, gi::ipiBaseDirectory), "/base");
}

