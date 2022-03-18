/**
  *  \file util/plugin/plugin.hpp
  *  \brief Class util::plugin::Plugin
  */
#ifndef C2NG_UTIL_PLUGIN_PLUGIN_HPP
#define C2NG_UTIL_PLUGIN_PLUGIN_HPP

#include <vector>
#include <map>
#include "afl/io/stream.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"

namespace util { namespace plugin {

    /** Plugin.
        Represents meta-information about a single plugin.
        This can be a loaded plugin, or a plugin about to be loaded or installed. */
    class Plugin {
     public:
        enum ItemType {
            PlainFile,              ///< Just a file to copy.
            ScriptFile,             ///< A file to copy and Load.
            ResourceFile,           ///< A file to copy and LoadResource.
            HelpFile,               ///< A file to copy and LoadHelpFile.
            Command                 ///< Not a file, just a command.
        };
        struct Item {
            ItemType type;
            String_t name;
            Item(ItemType type, const String_t& name)
                : type(type), name(name)
                { }
        };
        typedef std::vector<Item> ItemList_t;
        typedef std::map<String_t, String_t> FeatureSet_t;

        /** Constructor.
            \param id Plugin Id (needs to be in upper-case) */
        explicit Plugin(String_t id);

        /** Destructor. */
        ~Plugin();

        /** Load plugin definition file (.c2p).
            Call on a fresh Plugin instance to prepare a plugin from a proper definition.
            \param baseDir      Base directory (see getBaseDirectory())
            \param defFileName  Name of file (see getDefinitionFileName())
            \param file         Stream for reading the file
            \param log          Logger (for error messages)
            \param tx           Translator (for error messages) */
        void initFromPluginFile(String_t baseDir, String_t defFileName, afl::io::Stream& file, afl::sys::LogListener& log, afl::string::Translator& tx);

        /** Create from resource file.
            Call on a fresh Plugin instance to create a virtual plugin that loads a single resource file.
            \param baseDir      Base directory (see getBaseDirectory())
            \param resFileName  Name of file within baseDir
            \param tx           Translator (for generating metainformation) */
        void initFromResourceFile(String_t baseDir, String_t resFileName, afl::string::Translator& tx);

        /** Create from script file.
            Call on a fresh Plugin instance to create a virtual plugin that loads a single script file.
            Tries to extract a sensible description from the file's header comment.
            \param baseDir      Base directory (see getBaseDirectory())
            \param scriptFileName Name of script file within baseDir
            \param file         Stream for reading the script file
            \param tx           Translator (for generating metainformation) */
        void initFromScriptFile(String_t baseDir, String_t scriptFileName, afl::io::Stream& file, afl::string::Translator& tx);

        /** Create from resource configuration file (cc-res.cfg).
            Call on a fresh Plugin instance to create a virtual plugin that loads the resource files given in the cc-res.cfg file.
            cc-res.cfg is the classic (PCC1, PCC2) resource configuration mechanism;
            this file contains just a list of file names or resource specifiers.
            \param baseDir      Base directory (see getBaseDirectory())
            \param pluginName   Name of plug-in
            \param file         Stream for reading the config file
            \param tx           Translator (for generating metainformation) */
        void initFromConfigFile(String_t baseDir, String_t pluginName, afl::io::Stream& file, afl::string::Translator& tx);

        /** Save as plugin (.c2p) file.
            Produces a new .c2p file that, when read again using initFromPluginFile(), represents this plugin.
            Note that this rewrites the file from scratch; if you already have a .c2p file,
            prefer copying that instead of initFromPluginFile() + savePluginFile().
            \param file Stream */
        void savePluginFile(afl::io::Stream& file) const;

        /** Set base directory.
            Files referenced by this plugin will be searched starting here.
            \param baseDir Base directory. */
        void setBaseDirectory(const String_t& baseDir);

        /** Add an item to this plugin.
            \param type Item type
            \param name File name or command, depending on type */
        void addItem(ItemType type, const String_t& name);

        /** Set "loaded" flag.
            \param flag true if plugin is loaded */
        void setLoaded(bool flag);

        /** Get plugin Id.
            \return plugin Id (machine-readable identifier, upper-case) */
        const String_t& getId() const;

        /** Get plugin name.
            \return plugin name (human-readable) */
        const String_t& getName() const;

        /** Get description.
            \return description (human-readable, possibly multi-line) */
        const String_t& getDescription() const;

        /** Get base directory.
            \return base directory
            \see setBaseDirectory */
        const String_t& getBaseDirectory() const;

        /** Get definition file name.
            \return name, if plugin was loaded with initFromPluginFile(); empty otherwise */
        const String_t& getDefinitionFileName() const;

        /** Get items (files, commands) contained in this plugin.
            \return items */
        const ItemList_t& getItems() const;

        /** Check whether this plugin provides a certain feature.
            \param feature Feature Id (upper-case)
            \return true if plugin provides feature */
        bool isProvided(const String_t& feature) const;

        /** Check whether this plugin conflicts with another.
            Two plugins conflict if they provide the same features;
            they cannot be installed together.
            \param other Plugin to check
            \return true on conflict */
        bool isConflict(const Plugin& other) const;

        /** Check whether this plugin qualifies as an update for another plugin.
            A plugin qualifies as update if it has the same or fewer preconditions
            and provides the same or better features.

            Note that it's callers responsibility to select \c other with the same
            Id (i.e. same name).

            \param other Plugin to check
            \return true if qualifies as update */
        bool isUpdateFor(const Plugin& other) const;

        /** Check whether this plugin depends on another one.
            This blocks \c other from uninstalling
            \param other Plugin to check
            \return true if this plugin depends on \c other */
        bool isDependingOn(const Plugin& other) const;

        /** Check whether this plugin is satisfied by an installed feature set.
            The FeatureSet_t represents the union of all installed plugins.
            This function determines whether the plugin can be installed.
            \param have Feature set (see enumProvidedFeatures)
            \return true if plugin can be installed */
        bool isSatisfiedBy(const FeatureSet_t& have) const;

        /** Check whether plugin is loaded.
            \return flag
            \see setLoaded */
        bool isLoaded() const;

        /** List missing features.
            Giving an installed feature set, produces a list of features missing
            to install this plugin.
            \param [in]  have    Available features (see enumProvidedFeatures)
            \param [out] missing Missing features */
        void enumMissingFeatures(const FeatureSet_t& have, FeatureSet_t& missing) const;

        /** List provided features.
            \param [in,out] have Features added here */
        void enumProvidedFeatures(FeatureSet_t& have) const;

     private:
        String_t m_id;
        String_t m_name;
        String_t m_description;
        String_t m_baseDir;
        String_t m_defFileName;
        FeatureSet_t m_provides;
        FeatureSet_t m_requires;
        ItemList_t m_items;
        bool m_isLoaded;
    };

    /** Compare versions.
        \param a,b Version identifiers (i.e. "1.0")
        \return true iff a is older than b */
    bool compareVersions(const String_t& a, const String_t& b);

} }

#endif
