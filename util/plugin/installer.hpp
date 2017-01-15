/**
  *  \file util/plugin/installer.hpp
  */
#ifndef C2NG_UTIL_PLUGIN_INSTALLER_HPP
#define C2NG_UTIL_PLUGIN_INSTALLER_HPP

#include <memory>
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/filesystem.hpp"

namespace util { namespace plugin {

    class Manager;
    class Plugin;

    // /** Plugin Installer.
    //     Allows installing and removing plugins. */
    class Installer {
     public:
        enum ScanResult {
            NoPlugin,
            OnePlugin,
            MultiplePlugins
        };

        Installer(Manager& mgr, afl::io::FileSystem& fs, afl::io::Directory& rootDir);

        ~Installer();

        Plugin* prepareInstall(String_t fileName);

        void doInstall(bool dry = false);

        ScanResult checkInstallAmbiguity(String_t& out);

        bool doRemove(Plugin* pPlug, bool dry = false);

     private:
        // Integration
        Manager& manager;
        afl::io::FileSystem& m_fileSystem;
        afl::io::Directory& rootDir;

        // State
        afl::base::Ptr<afl::io::Directory> srcDir;
        afl::base::Ptr<afl::io::Stream> srcFile;
        std::auto_ptr<Plugin> apPlug;
    };

} }

#endif
