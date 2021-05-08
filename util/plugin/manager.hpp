/**
  *  \file util/plugin/manager.hpp
  *  \brief Class util::plugin::Manager
  */
#ifndef C2NG_UTIL_PLUGIN_MANAGER_HPP
#define C2NG_UTIL_PLUGIN_MANAGER_HPP

#include <vector>
#include "afl/container/ptrvector.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/plugin/plugin.hpp"

namespace util { namespace plugin {

    /** Plugin Manager.
        Manages (and owns) a list of Plugin objects and provides methods to work on the list.
        This maintains plugin meta-information, not the plugin content. */
    class Manager {
     public:
        /** Constructor.
            \param tx Translator (for log messages)
            \param log Logger (for log messages) */
        Manager(afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Destructor. */
        ~Manager();

        /** Find plugins in a directory.
            Looks for *.c2p files and loads them.
            \param dir Directory */
        void findPlugins(afl::io::Directory& dir);

        /** Find plugins in a directory.
            \param fs File system
            \param dirName Name of directory */
        void findPlugins(afl::io::FileSystem& fs, String_t dirName);

        /** Add new plugin.
            \param p Newly-allocated Plugin object. Manager takes ownership. */
        void addNewPlugin(Plugin* p);

        /** Enumerate plugins.
            This function has two modes:
            - standard mode: plugins are returned in natural order (sorted when added by findPlugins(),
              as added when added by addNewPlugin()). This mode always lists all plugins.
            - ordered mode: plugins are returned in topological order, so that a plugin's dependencies
              are returned before it. This mode does not list plugins with unsatisfied or cyclic dependencies.
            \param [out] out      result
            \param [in]  ordered  true for ordered mode, false for standard mode */
        void enumPlugins(std::vector<Plugin*>& out, bool ordered) const;

        /** Enumerate conflicting plugins.
            \param [in]  candidate  Candidate plugin (one that we want to add, i.e. newly created)
            \param [out] out        List of plugins that prevent loading the candidate, owned by Manager */
        void enumConflictingPlugins(const Plugin& candidate, std::vector<Plugin*>& out) const;

        /** Enumerate depending plugins.
            \param [in]  candidate  Candidate plugin (one that we want to remove, i.e. from getPluginById())
            \param [out] out        List of plugins that prevent removing the candidate (i.e. depend on it), owned by Manager */
        void enumDependingPlugins(const Plugin& candidate, std::vector<Plugin*>& out) const;

        /** Enumerate provided features.
            Builds the combined set of the features provided by all plugins.
            \param [out] have  Result */
        void enumProvidedFeatures(Plugin::FeatureSet_t& have) const;

        /** Extract plugin.
            Removes the plugin from Manager's ownership.
            \param p Plugin to extract
            \return p if that is a valid plugin (caller assumes ownership), null otherwise */
        Plugin* extractPlugin(Plugin* p);

        /** Look up plugin.
            \param id Plugin Id
            \return plugin (owned by Managr) or null */
        Plugin* getPluginById(const String_t& id) const;

        /** Access log listener.
            \return log listener */
        afl::sys::LogListener& log()
            { return m_log; }

        /** Access translator.
            \return translator */
        afl::string::Translator& translator()
            { return m_translator; }

     private:
        afl::container::PtrVector<Plugin> m_plugins;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };


} }

#endif
