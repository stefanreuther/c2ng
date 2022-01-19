/**
  *  \file game/browser/folder.hpp
  *  \brief Base class game::browser::Folder
  */
#ifndef C2NG_GAME_BROWSER_FOLDER_HPP
#define C2NG_GAME_BROWSER_FOLDER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ptr.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "game/browser/types.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"
#include "util/rich/text.hpp"

namespace game { namespace browser {

    /** Base class for a folder. */
    class Folder : public afl::base::Deletable {
     public:
        /** Folder kind. */
        enum Kind {
            kRoot,              ///< Top-most directory.
            kFolder,            ///< General folder.
            kAccount,           ///< Entry point to an account.
            kLocal,             ///< Root of a local file system.
            kGame,              ///< Game.
            kFavorite,          ///< Entry in favorite list.
            kFavoriteList       ///< Favorite list.
        };

        /** Load content of this folder.
            Produces a list of new folders.
            If canEnter() returns true, this function can still be called but should return an empty (unmodified) list.

            \param then Task to receive the result; never null

            \return newly-allocated task; never null. Call it to start, will call \c then with the result. */
        virtual std::auto_ptr<Task_t> loadContent(std::auto_ptr<LoadContentTask_t> then) = 0;

        /** Load folder configuration.
            Loads this folder's pcc2.ini file (if any).

            - If this is a local storage folder, should load the configuration file
              using UserConfiguration::loadGameConfiguration() and return true.
            - If this is a network game that has a local folder, should load the configuration file for that folder and return true.
            - If this is a network game without local folder, should generate a configuration file on the fly and return true.
              The generated configuration should have
              - Game_ReadOnly set to 1.
              - Game_User / Game_Host / Game_Type / Game_Id set accordingly.
            - Otherwise, return false.

            \param config [out] Configuration
            \return true if configuration loaded */
        virtual bool loadConfiguration(game::config::UserConfiguration& config) = 0;

        /** Save folder configuration.

            If this is a local storage folder or a network game that has a local folder,
            should save the configuration file using UserConfiguration::saveGameConfiguration().

            \param config [in] Configuration */
        virtual void saveConfiguration(const game::config::UserConfiguration& config) = 0;

        /** Set local directory name.
            This function can be called if this directory's Root reports aLocalSetup.
            If should use Account::setGameFolderName() for the associated account.

            The local directory can be queried by loadGameRoot(), then gameDirectory(), then getDirectoryName().
            If no Root can be produced, or getDirectoryName() returns an empty string, the game has no associated local directory.

            \param directoryName Name
            \return true on success, false on error */
        virtual bool setLocalDirectoryName(String_t directoryName) = 0;

        /** Load game root.
            Presence of a Root will make this a game directory.
            Root will have a mandatory specification loader and host version.

            - If this is a local storage folder or a network game that has a local folder,
              should load the game for editing or viewing as configured.
              For a local folder, it should call Browser::loadGameRoot which determines whether there is an associated network folder.
            - If this is a network game without a local folder, should load the game for viewing
              (as if Game_ReadOnly was set) and point the game directory to an internal/temporary folder.

            The passed configuration must be merged into the Root::userConfiguration() of the produced root.
            The loaded root should evaluate at least the Game_Finished and Game_ReadOnly options.

            This function must be implemented in a way that the returned task can outlive the Folder instance.

            \param then Task to receive the result; never null
            \param config Folder configuration, obtained using loadConfiguration and possibly modified.

            \return newly-allocated task; never null. Call it to start, will call \c then with the result (which may be null). */
        virtual std::auto_ptr<Task_t> loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t> then) = 0;

        /** Get name.
            The name is displayed in lists.
            \return name */
        virtual String_t getName() const = 0;

        /** Get description.
            This can be a longer description of the game.
            \return description */
        virtual util::rich::Text getDescription() const = 0;

        /** Compare folders.
            Used to find a folder in a list of folders.
            \param other Other folder
            \return true if equal */
        virtual bool isSame(const Folder& other) const = 0;

        /** Check whether this folder can be entered.
            \retval false This folder will not contain subfolders, it does not make sense to call loadContent().
                          Use this for folders representing network games.
            \retval true  This folder may contain subfolders, call loadContent() to find it
                          Use this for regular folders. */
        virtual bool canEnter() const = 0;

        /** Get kind of folder.
            This determines the icon associated with it.
            \return kind */
        virtual Kind getKind() const = 0;


        /** Default (dummy) implementation of loadGameRoot().
            Use when the folder does not provide a root: completes the operation with a null root.
            \param t Task to receive the result
            \return newly-allocated task */
        static std::auto_ptr<Task_t> defaultLoadGameRoot(std::auto_ptr<LoadGameRootTask_t> t);
    };

} }

#endif
