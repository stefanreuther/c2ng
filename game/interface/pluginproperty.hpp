/**
  *  \file game/interface/pluginproperty.hpp
  *  \brief Plugin Properties
  */
#ifndef C2NG_GAME_INTERFACE_PLUGINPROPERTY_HPP
#define C2NG_GAME_INTERFACE_PLUGINPROPERTY_HPP

#include "afl/data/value.hpp"
#include "util/plugin/plugin.hpp"

namespace game { namespace interface {

    /** Plugin Property Identifier. */
    enum PluginProperty {
        ipiId,
        ipiName,
        ipiDescription,
        ipiBaseDirectory
    };

    /** Get plugin property.
        \param plugin Plugin
        \param ipi    Property identifier */
    afl::data::Value* getPluginProperty(const util::plugin::Plugin& plugin, PluginProperty ipi);

} }

#endif
