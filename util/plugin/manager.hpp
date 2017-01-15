/**
  *  \file util/plugin/manager.hpp
  */
#ifndef C2NG_UTIL_PLUGIN_MANAGER_HPP
#define C2NG_UTIL_PLUGIN_MANAGER_HPP

#include <vector>
#include "afl/io/directory.hpp"
#include "afl/container/ptrvector.hpp"
#include "util/plugin/plugin.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/string/translator.hpp"

namespace util { namespace plugin {

    class Manager {
     public:
        Manager(afl::string::Translator& tx, afl::sys::LogListener& log);
        ~Manager();

        void findPlugins(afl::io::Directory& dir);

        void findPlugins(afl::io::FileSystem& fs, String_t dirName);

        void addNewPlugin(Plugin* p);

        void enumPlugins(std::vector<Plugin*>& out, bool ordered = true) const;

        void enumConflictingPlugins(const Plugin& candidate, std::vector<Plugin*>& out) const;

        void enumDependingPlugins(const Plugin& candidate, std::vector<Plugin*>& out) const;

        void enumFeatures(Plugin::FeatureSet& fset) const;

        Plugin* extractPlugin(Plugin* p);

        Plugin* getPluginById(const String_t& id) const;

        afl::sys::LogListener& log()
            { return m_log; }

     private:
        afl::container::PtrVector<Plugin> m_plugins;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };


} }

#endif
