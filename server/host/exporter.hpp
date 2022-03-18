/**
  *  \file server/host/exporter.hpp
  *  \brief Class server::host::Exporter
  */
#ifndef C2NG_SERVER_HOST_EXPORTER_HPP
#define C2NG_SERVER_HOST_EXPORTER_HPP

#include "afl/io/directory.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/net/commandhandler.hpp"
#include "afl/net/redis/hashkey.hpp"
#include "afl/sys/loglistener.hpp"
#include "server/host/configurationbuilder.hpp"

namespace server { namespace host {

    class Game;
    class Root;

    /** Exporter.
        Unlike c2host-classic, c2host-ng will not rely on sharing a dataspace with the host filer.
        Thus, the host filer will not necessarily need to use a 1:1 data organisation.
        Therefore, before we can run a program on the data, we will have to export it into the OS file system.
        This class contains the logic to do that.

        Host scripts take their configuration from an ini file (c2host.ini).
        All paths are translated during the export.

        Mapping:
        <pre>
                    in host filer           on disk            direction
                    ---------------------   ----------         ---------
                    bin                     bin                export only
                    defaults                defaults           export only
                    games/0000              game               import only
                    games/0000/data         game/data          bidir
                    games/0000/in           game/in            bidir
                    games/0000/out          game/out           bidir
                    games/0000/c2host.ini   game/c2host.ini    export only
                    tools/host-xyz          host               export only
                    tools/master-xyz        master             export only
                    tools/shiplist-xyz      shiplist           export only
                    tools/xyz               toolX              export only
        </pre>

        This export will be performed for every host action (checkturn, master, host).
        On a well-populated game, export will move give-or-take 5 megabytes.
        Using c2file-classic, this will take a few seconds; using c2file-ng, it will be <1 second. */
    class Exporter {
     public:
        /** Constructor.
            \param source Interface to host filer
            \param fs File system
            \param log Logger */
        Exporter(afl::net::CommandHandler& source, afl::io::FileSystem& fs, afl::sys::LogListener& log);

        /** Export a game.
            Creates a copy of the game data in the OS file system.

            \param game Game to export
            \param root Service root
            \param fsDirName Target directory.
            This directory needs to be the current working directory when a game script is being started.

            \return Relative path to game. This name must be passed to the game script as the game path.
            Combined with the fsDirName, it will produce the absolute name of the game directory. */
        String_t exportGame(Game& game, Root& root, String_t fsDirName);

        /** Import a game.
            Re-imports the exported data from the OS file system, into the host filer.
            The parameters are the same as were used for exportGame().

            \param game Game to import
            \param root Service root
            \param fsDirName Source directory. */
        void importGame(Game& game, Root& root, String_t fsDirName);

     private:
        /** Export a tool.
            Updates the configuration and exports the files.
            \param ini     [in/out] Configuration
            \param parent  [in] Root directory for export (same as fsDirName)
            \param dirName [in] Name of directory to export to (relative to parent), will be created
            \param prefix  [in] Prefix for configuration variables
            \param hash    [in] Tool configuration hash from database */
        void exportTool(ConfigurationBuilder& ini, afl::io::Directory& parent, const String_t& dirName, const String_t& prefix, afl::net::redis::HashKey hash);

        /** Export a subdirectory.
            Exports the content of \c source into a directory \c targetSub, created beneath \c targetBase.
            \param source     [in] Source directory in host filer
            \param targetBase [in] Output base directory
            \param targetSub  [in] Output directory, relative to \c targetBase, will be created */
        void exportSubdirectory(const String_t& source, const String_t& targetBase, const String_t& targetSub);

        /** Store configuration file.
            \param ini    [in] Configuration
            \param parent [in] Target directory */
        void storeConfigurationFile(const ConfigurationBuilder& ini, afl::io::Directory& parent);

        /** Import a subdirectory.
            This is the inverse operation to exportSubdirectory.
            \param source     [in] Source (target) directory in host filer
            \param targetBase [in] Output (input) base directory
            \param targetSub  [in] Output (input) directory, relative to \c targetBase */
        void importSubdirectory(const String_t& source, const String_t& targetBase, const String_t& targetSub);

        /** Import logfiles.
            Logfiles are only imported, stale/outdated logfiles not overwritten.
            \param source     [in] Source (target) directory in host filer
            \param targetName [in] Output (input) base directory */
        void importLogFiles(const String_t& source, const String_t& targetName);

        /** Import backups.
            Backups are never exported, only imported.
            On import, backup files are either merged (=added) to the filer directory as files, or as unpacked directories.
            \param source     [in] Source (target) directory in host filer
            \param targetBase [in] Output (input) base directory
            \param targetSub  [in] Output (input) directory, relative to \c targetBase
            \param unpackBackups [in] true to unpack backups, false to keep them as files */
        void importBackups(const String_t& source, const String_t& targetBase, const String_t& targetSub, bool unpackBackups);

        /** Import tarball.
            Unpacks the tarball on the fly and places it in a directory.
            \param source      [in] Source (target) directory in host filer
            \param tarballBase [in] Basename of tarball, will become subdirectory of \c source
            \param tarball     [in] Tarball */
        void importTarball(const String_t& source, const String_t& tarballBase, afl::base::Ref<afl::io::Stream> tarball);

        afl::net::CommandHandler& m_source;
        afl::io::FileSystem& m_fileSystem;
        afl::sys::LogListener& m_log;
    };

} }

#endif
