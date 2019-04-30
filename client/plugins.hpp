/**
  *  \file client/plugins.hpp
  *  \brief Plugin Integration
  *
  *  We load plugins by creating bytecode.
  *  Executing this bytecode the regular way allows us to re-use the existing UI synchronisation methods.
  *  This differs from PCC2 in that it creates only one process per plugin / plugin group,
  *  instead of one for each "ScriptFile", "Command" line in the plugin,
  *  and thus reduces our options for error recovery a little.
  *
  *  Loading of files (namely, core.q) is also implemented here to allow the same re-usage benefits.
  *  In addition, this reduces the number of places where script files are opened to a minimum,
  *  allowing to add things like *.qc file support with minimum effort.
  */
#ifndef C2NG_CLIENT_PLUGINS_HPP
#define C2NG_CLIENT_PLUGINS_HPP

#include "interpreter/bytecodeobject.hpp"
#include "util/plugin/plugin.hpp"
#include "util/plugin/manager.hpp"

namespace client {

    /** Create plugin loader for a single plugin.
        \param plugin The plugin
        \return BytecodeObject that, when executed, will load the plugin */
    interpreter::BCORef_t createPluginLoader(const util::plugin::Plugin& plugin);

    /** Create plugin loader for all unloaded plugins.
        This will mark the plugins loaded.
        \param manager Plugin manager
        \return BytecodeObject that, when executed, will load all unloaded plugins registered with the manager. */
    interpreter::BCORef_t createLoaderForUnloadedPlugins(util::plugin::Manager& manager);

    /** Create a file loader.
        \param fileName Name of file
        \param origin Value to set as origin
        \return BytecodeObject that, when executed, will load the given file or print an error message. */
    interpreter::BCORef_t createFileLoader(const String_t& fileName, const String_t& origin);

}

#endif
