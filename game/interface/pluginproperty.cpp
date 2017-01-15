/**
  *  \file game/interface/pluginproperty.cpp
  */

#include "game/interface/pluginproperty.hpp"
#include "interpreter/values.hpp"

using interpreter::makeStringValue;

afl::data::Value*
game::interface::getPluginProperty(const util::plugin::Plugin& plugin, PluginProperty ipi)
{
    // ex int/if/plugif.h:getPluginProperty
    switch (ipi) {
     case ipiId:
        /* @q Id:Str (Plugin Property)
           Id of the plugin.
           @since PCC2 1.99.25 */
        return makeStringValue(plugin.getId());

     case ipiName:
        /* @q Name:Str (Plugin Property)
           Human-readable name of the plugin.
           @since PCC2 1.99.25 */
        return makeStringValue(plugin.getName());

     case ipiDescription:
        /* @q Description:Str (Plugin Property)
           Description of the plugin. This can possibly be multiple paragraphs of text.
           @since PCC2 1.99.25 */
        return makeStringValue(plugin.getDescription());

     case ipiBaseDirectory:
        /* @q Directory:Str (Plugin Property)
           Base directory of the plugin.
           This directory contains files installed with the plugin.
           @since PCC2 1.99.25 */
        return makeStringValue(plugin.getBaseDirectory());
    }
    return 0;
}
