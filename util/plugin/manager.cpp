/**
  *  \file util/plugin/manager.cpp
  */

#include <memory>
#include "util/plugin/manager.hpp"
#include "version.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/format.hpp"

namespace {
    const char LOG_NAME[] = "plugin.mgr";

    bool comparePlugins(const util::plugin::Plugin& a, const util::plugin::Plugin& b)
    {
        return a.getId() < b.getId();
    }

    void initFeatures(util::plugin::Plugin::FeatureSet& features)
    {
        features["PCC"] = PCC2_VERSION;
    }
}

util::plugin::Manager::Manager(afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_plugins(),
      m_translator(tx),
      m_log(log)
{
    // ex PluginManager::PluginManager
}
util::plugin::Manager::~Manager()
{ }

void
util::plugin::Manager::findPlugins(afl::io::Directory& dir)
{
    // Open the directory. This might throw, but is not a problem;
    // it just means the directory does not exist.
    afl::base::Ptr<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > > entries;
    try {
        entries = dir.getDirectoryEntries().asPtr();
    }
    catch (...) { }
    if (entries.get() == 0) {
        return;
    }

    try {
        // Load everything
        int count = 0;
        afl::base::Ptr<afl::io::DirectoryEntry> elem;
        while (entries->getNextElement(elem) && elem.get() != 0) {
            String_t name = elem->getTitle();
            if (name.size() > 4
                && afl::string::strCaseCompare(name.substr(name.size() - 4), ".c2p") == 0
                && name[0] != '.'
                && elem->getFileType() == afl::io::DirectoryEntry::tFile)
            {
                // Looks like a plugin
                String_t pluginName = name.substr(0, name.size() - 4);
                try {
                    std::auto_ptr<Plugin> plugin(new Plugin(afl::string::strUCase(pluginName)));
                    afl::base::Ref<afl::io::Stream> file = elem->openFile(afl::io::FileSystem::OpenRead);
                    plugin->initFromPluginFile(dir.getDirectoryEntryByName(pluginName)->getPathName(), name, *file, m_log);
                    m_plugins.pushBackNew(plugin.release());
                    ++count;
                }
                catch (std::exception& e) {
                    m_log.write(afl::sys::LogListener::Error, LOG_NAME,
                                afl::string::Format(m_translator.translateString("Error loading plugin %s").c_str(), pluginName), e);
                }
            }
        }

        // Sort for determinism
        m_plugins.sort(comparePlugins);
        m_log.write(afl::sys::LogListener::Trace, LOG_NAME,
                    afl::string::Format(m_translator.translateString("Found %d plugin%!1{s%}").c_str(), count));
    }
    catch (std::exception& e) {
        m_log.write(afl::sys::LogListener::Error, LOG_NAME,
                    m_translator.translateString("Cannot load plugins"), e);
    }
}

void
util::plugin::Manager::findPlugins(afl::io::FileSystem& fs, String_t dirName)
{
    try {
        afl::base::Ref<afl::io::Directory> dir = fs.openDirectory(dirName);
        findPlugins(*dir);
    }
    catch (...) { }
}

void
util::plugin::Manager::addNewPlugin(Plugin* p)
{
    // ex PluginManager::addNewPlugin
    if (p != 0) {
        m_plugins.pushBackNew(p);
    }
}

// /** Enumerate all plugins in dependency order. */
void
util::plugin::Manager::enumPlugins(std::vector<Plugin*>& out, bool ordered) const
{
    // ex PluginManager::enumPlugins
    // Initial feature set
    Plugin::FeatureSet features;
    initFeatures(features);

    // Marker for all plugins
    std::vector<char> did;
    did.resize(m_plugins.size());

    // Enumerate
    while (1) {
        bool didOne = false;
        for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
            if (!did[i] && (!ordered || m_plugins[i]->isSatisfied(features))) {
                didOne = true;
                did[i] = 1;
                out.push_back(m_plugins[i]);
                m_plugins[i]->addProvidedFeaturesTo(features);
            }
        }
        if (!didOne) {
            break;
        }
    }

    // Anything missing?
    for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
        if (!did[i]) {
            m_log.write(afl::sys::LogListener::Error, LOG_NAME,
                        afl::string::Format(m_translator.translateString("Plugin %s cannot be loaded because of missing preconditions").c_str(), m_plugins[i]->getId()));
        }
    }
}

// /** Enumerate all plugins that conflict with the given candidate. */
void
util::plugin::Manager::enumConflictingPlugins(const Plugin& candidate, std::vector<Plugin*>& out) const
{
    // ex PluginManager::enumConflictingPlugins
    for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
        if (candidate.getId() == m_plugins[i]->getId()
            ? !candidate.isUpdateFor(*m_plugins[i])
            : candidate.isConflict(*m_plugins[i]))
        {
            out.push_back(m_plugins[i]);
        }
    }
}

// /** Enumerate all plugins that depend on the candidate plugin, i.e.\ prevent its uninstallation. */
void
util::plugin::Manager::enumDependingPlugins(const Plugin& candidate, std::vector<Plugin*>& out) const
{
    // ex PluginManager::enumDependingPlugins
    for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
        if (m_plugins[i]->isDependingOn(candidate)) {
            out.push_back(m_plugins[i]);
        }
    }
}

// /** Enumerate all available features. */
void
util::plugin::Manager::enumFeatures(Plugin::FeatureSet& fset) const
{
    // ex PluginManager::enumFeatures
    initFeatures(fset);
    for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
        m_plugins[i]->addProvidedFeaturesTo(fset);
    }
}

// /** Extract plugin, given a pointer.
//     Caller must delete it.
//     \return pointer on success, otherwise 0 */
util::plugin::Plugin*
util::plugin::Manager::extractPlugin(Plugin* p)
{
    // ex PluginManager::extractPlugin
    for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
        if (p == m_plugins[i]) {
            Plugin* retVal = m_plugins.extractElement(i);
            m_plugins.erase(m_plugins.begin() + i);
            return retVal;
        }
    }
    return 0;
}

// /** Get plugin, given an Id. */
util::plugin::Plugin*
util::plugin::Manager::getPluginById(const String_t& id) const
{
    // ex PluginManager::getPluginById
    for (size_t i = 0, n = m_plugins.size(); i < n; ++i) {
        if (m_plugins[i]->getId() == id) {
            return m_plugins[i];
        }
    }
    return 0;
}
