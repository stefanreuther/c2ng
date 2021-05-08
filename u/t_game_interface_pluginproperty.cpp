/**
  *  \file u/t_game_interface_pluginproperty.cpp
  *  \brief Test for game::interface::PluginProperty
  */

#include <memory>
#include "game/interface/pluginproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/data/access.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"

namespace gi = game::interface;
using afl::data::Access;

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
    std::auto_ptr<afl::data::Value> p;
    p.reset(gi::getPluginProperty(plug, gi::ipiId));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "PLID");

    p.reset(gi::getPluginProperty(plug, gi::ipiName));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "The Name");

    p.reset(gi::getPluginProperty(plug, gi::ipiDescription));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "Description...");

    p.reset(gi::getPluginProperty(plug, gi::ipiBaseDirectory));
    TS_ASSERT_EQUALS(Access(p.get()).toString(), "/base");
}

