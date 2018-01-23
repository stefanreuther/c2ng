/**
  *  \file server/host/installer.hpp
  *  \brief Class server::host::Installer
  */
#ifndef C2NG_SERVER_HOST_INSTALLER_HPP
#define C2NG_SERVER_HOST_INSTALLER_HPP

#include "game/playerset.hpp"
#include "afl/base/types.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/string/string.hpp"

namespace server { namespace host {

    class Root;
    class Game;

    /** Install files to users' directories.
        The installer manages user game directories.
        If a user configures a directory for a game, c2host will synchronize that directory from the host runs.

        @change This implements a hostfile-to-userfile copy (unlike the classic version which mostly copied
        filesystem-to-userfile). */
    class Installer {
     public:
        /** Constructor.
            \param root Service root */
        explicit Installer(Root& root);

        /** Check for precious file.
            A precious file is a file that is NOT touched by synchronization.
            Users can only manipulate precious files in a managed game directory.
            \param name File name (without path) to check
            \retval true Precious file, user can manipulate it
            \retval false File managed by c2host, user cannot manipulate it */
        bool isPreciousFile(const String_t& name) const;

        /** Install game data.
            This function is used to install all game files after a directory has been configured for a player, or host has run.
            \param game     Game to install from
            \param players  Whose player data to use
            \param userId   User Id
            \param dirName  Directory name

            \throw std::exception File access problems (typically: the dirName is not accessible to userId). */
        void installGameData(Game& game, game::PlayerSet_t players, String_t userId, String_t dirName);

        /** Install single file to multiple players' game directory.
            This function is used to distribute an uploaded turn file to all players of that slot.

            This function will internally determine the target directories.
            It will NOT fail when there are permission problems.
            The summarized result (number of successes/failures) will just be logged.

            \param game     Game to install from
            \param players  List of players
            \param fileName File to install
            \param fileContent Content of file
            \param slot     Slot number this file is associated with.
                            If nonzero, this file belongs to a given slot (which requires conflicting game sessions be closed).
                            If zero, this file is not associated with a slot. */
        void installFileMulti(Game& game, const afl::data::StringList_t& players, String_t fileName, String_t fileContent, int32_t slot);

        /** Process a change due to an (un)subscription.
            If this (un)subscription causes a change to the game directory, updates that:
            install new files or dissociate the game directory.
            \param game   Game
            \param player Changed player
            \param slot   Changed slot
            \param added  true if player was added, false if he was removed */
        void installChangedGameFiles(Game& game, String_t player, int32_t slot, bool added);

        /** Uninstall game data.
            Drops the link from the filer to the host (but keeps the files there for the user).
            \param userId User Id
            \param dirName directory name to uninstall */
        void uninstallGameData(String_t userId, String_t dirName);

     private:
        Root& m_root;
    };

} }

#endif
