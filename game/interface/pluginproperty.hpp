/**
  *  \file game/interface/pluginproperty.hpp
  */
#ifndef C2NG_GAME_INTERFACE_PLUGINPROPERTY_HPP
#define C2NG_GAME_INTERFACE_PLUGINPROPERTY_HPP

#include "afl/data/value.hpp"
#include "util/plugin/plugin.hpp"

namespace game { namespace interface {

    enum PluginProperty {
        ipiId,
        ipiName,
        ipiDescription,
        ipiBaseDirectory
    };

    afl::data::Value* getPluginProperty(const util::plugin::Plugin& plugin, PluginProperty ipi);

} }

#endif
