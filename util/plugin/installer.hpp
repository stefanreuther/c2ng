/**
  *  \file util/plugin/installer.hpp
  *  \brief Class util::plugin::Installer
  */
#ifndef C2NG_UTIL_PLUGIN_INSTALLER_HPP
#define C2NG_UTIL_PLUGIN_INSTALLER_HPP

#include <memory>
#include "afl/io/directory.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/base/optional.hpp"
#include "afl/string/translator.hpp"

namespace util { namespace plugin {

    class Manager;
    class Plugin;

    /** Plugin Installer.
        Contains logic for installing and removing plugins.

        <b>Sequence for installing plugins:</b>

        - call prepareInstall()
        - call checkInstallAmbiguity() to detect possible user errors [optional]
        - call checkInstallPreconditions() to detect dependency problems [optional]
        - call doInstall() to perform the installation

        <b>Sequence for removing plugins:</b>

        - call checkRemovePreconditions() [optional]
        - call doRemove() */
    class Installer {
     public:
        enum ScanResult {
            NoPlugin,
            OnePlugin,
            MultiplePlugins
        };

        /** Constructor.
            \param mgr Manager (containing existing plugins)
            \param fs FileSystem
            \param rootDir Plugin root directory (points to mgr's \c root, resides within \c fs) */
        Installer(Manager& mgr, afl::io::FileSystem& fs, afl::io::Directory& rootDir);

        /** Destructor. */
        ~Installer();

        /** Prepare installation.
            Checks whether the file name refers to a file that can be installed as a plugin.
            If so, makes a proto-plugin and returns it.
            The proto-plugin remains owned by the PluginInstaller.
            The caller can examine it.
            It can be installed by calling doInstall().

            \param fileName File to install
            \param tx Translator
            \return proto-plugin or null */
        Plugin* prepareInstall(String_t fileName, afl::string::Translator& tx);

        /** Check for installation ambiguities.
            Call after successful prepareInstall().

            An ambiguity is when the user chose a file to auto-convert, but there is a *.c2p he should probably use instead.

            \param out [out] Name of alternative *.c2p file
            \retval NoPlugin no ambiguity
            \retval OnePlugin there is one *.c2p file that could be installed instead, its name given in %out.
            \retval MultiplePlugins there are multiple *.c2p files that could be installed instead */
        ScanResult checkInstallAmbiguity(String_t& out);

        /** Check preconditions for installation.
            Call after successful prepareInstall().

            This checks for unsatisified dependencies and conflicts.
            It generates a human-readable message if there is a problem.
            The message will have multiple lines and can be written to a console or GUI window.

            \param tx Translator to generate the message
            \return error message; Nothing if there is no problem */
        afl::base::Optional<String_t> checkInstallPreconditions(afl::string::Translator& tx);

        /** Install the prepared plugin.
            Call after successful prepareInstall().

            This installs the plugin into the user's profile.
            A previous plugin of the same name is uninstalled before.
            The plugin will be registered in this Installer's Manager instance.

            Throws exceptions on errors.

            \param dry true to simulate only (update Manager, but do not modify disk) */
        void doInstall(bool dry = false);

        /** Check preconditions for removal.

            This checks whether removing the given plugin will create unresolved dependencies or conflicts.
            It generates a human-readable message if there is a problem.
            The message will have multiple lines and can be written to a console or GUI window.

            \param tx Translator to generate the message
            \return error message; Nothing if there is no problem */
        afl::base::Optional<String_t> checkRemovePreconditions(const Plugin& plug, afl::string::Translator& tx);

        /** Remove a plugin.
            Deletes all associated files.
            \param pPlug Plugin (must come from this Installer's Manager). Call is ignored if this parameter is invalid.
            \param dry true to simulate only (update Manager, but do not modify disk)
            \return true on success */
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
