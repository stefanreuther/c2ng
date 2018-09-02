/**
  *  \file util/plugin/plugin.hpp
  */
#ifndef C2NG_UTIL_PLUGIN_PLUGIN_HPP
#define C2NG_UTIL_PLUGIN_PLUGIN_HPP

#include <vector>
#include <map>
#include "afl/string/string.hpp"
#include "afl/io/stream.hpp"
#include "afl/sys/loglistener.hpp"

namespace util { namespace plugin {

    class Plugin {
     public:
        // FIXME: move these embedded types into a separate file so we can drop the dependency World->(Manager)->Plugin
        enum ItemType {
            PlainFile,              // Just a file to copy
            ScriptFile,             // A file to copy and Load()
            ResourceFile,           // A file to copy and LoadResource()
            HelpFile,               // A file to copy and LoadHelpFile()
            Command                 // Not a file, just a command
        };
        struct Item {
            ItemType type;
            String_t name;
            Item(ItemType type, const String_t& name)
                : type(type), name(name)
                { }
        };
        typedef std::vector<Item> ItemList;
        typedef std::map<String_t, String_t> FeatureSet;
    
        Plugin(String_t id);
        ~Plugin();

        void initFromPluginFile(String_t baseDir, String_t defFileName, afl::io::Stream& file, afl::sys::LogListener& log);
        void initFromResourceFile(String_t baseDir, String_t resFileName);
        void initFromScriptFile(String_t baseDir, String_t scriptFileName, afl::io::Stream& file);
        void initFromConfigFile(String_t baseDir, String_t pluginName, afl::io::Stream& file);

        void savePluginFile(afl::io::Stream& file) const;
        void setBaseDirectory(const String_t& baseDir);

        void addItem(ItemType type, const String_t& name);

        void setLoaded(bool flag);

        const String_t& getId() const;
        const String_t& getName() const;
        const String_t& getDescription() const;
        const String_t& getBaseDirectory() const;
        const String_t& getDefinitionFileName() const;
        const ItemList& getItems() const;
        bool isProvided(const String_t& feature) const;
        bool isConflict(const Plugin& other) const;
        bool isUpdateFor(const Plugin& other) const;
        bool isDependingOn(const Plugin& other) const;
        bool isSatisfied(const FeatureSet& fset) const;
        bool isLoaded() const;
        void enumMissingFeatures(const FeatureSet& have, FeatureSet& missing) const;

        void addProvidedFeaturesTo(FeatureSet& fset);

     private:
        String_t id;
        String_t name;
        String_t description;
        String_t baseDir;
        String_t defFileName;
        FeatureSet provides;
        FeatureSet requires;
        ItemList items;
        bool loaded;
    };

    bool compareVersions(const String_t& a, const String_t& b);

} }

#endif
