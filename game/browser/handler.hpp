/**
  *  \file game/browser/handler.hpp
  *  \brief Interface game::browser::Handler
  */
#ifndef C2NG_GAME_BROWSER_HANDLER_HPP
#define C2NG_GAME_BROWSER_HANDLER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ptr.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "game/browser/types.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"

namespace game { namespace browser {

    class Folder;
    class Account;

    /** Definition of a type of supported game storage.
        A Browser's set of Handler instances defines the supported game storage types, local and server-based. */
    class Handler : public afl::base::Deletable {
     public:
        /** Handle folder name or URL.
            This function is used to resolve a user-supplied folder name or URL
            into a sequence of Folder instances representing a path thither.

            This function must check if it can handle the folder name, and if so, populate the result.

            Possible inputs and outputs:
            - local folder names (produces path on local system, i.e. "My Computer > Root > Path > Path")
            - network game URLs (produces path into user account, i.e. "Account > Games > The Game")
            - network storage URLs (produces path into user account, i.e. "Account > Files > Path")

            A network game URL could be a direct URL ("http://example.com/game/1") or a system-specific pseudo URL ("example://1").

            This function is only allowed to throw if it has positively identified the name/URL as being its responsibility.
            Exceptions will stop the handler search and prevent other handlers from being given a chance to handle the name/URL.

            @param name   [in] Name or URL
            @param result [out] Result
            @retval false Folder name or URL not recognized
            @retval true Folder name or URL recognized, result has been populated */
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<Folder>& result) = 0;

        /** Create account folder.
            This function is used to create browser elements for all existing accounts.

            This function must check if it can handle the given account, and if so, produce a Folder instance that describes the account root.

            @param acc Account (mutable because it can eventually be modified through the created Folder)
            @return Folder or null */
        virtual Folder* createAccountFolder(Account& acc) = 0;

        /** Load game root for physical folder.
            This function is used by FileSystemFolder to load a game root.
            The physical folder may represent a local game, or a network game's local folder.

            This routine must check if it can handle the given folder.
            If the folder contains an entirely local game, this must create a plain local Root.
            If the folder contains a network game, this must create a network Root.

            The configuration passed will be the content of the "pcc2.ini" file from the directory.
            It shall be consulted for the "Game_" options.
            - Game_Type / Game_User / Game_Host / Game_Id for a network game association
            - Game_Finished / Game_ReadOnly to determine the access level of loaded data
            - Game_AccessHostFiles to configure access to host files
            - Game_Charset to configure the character set

            If this function has identified the folder as being its responsibility, it shall return a task that produces the Root;
            If the task has problems producing the root, it shall complete with a null result.

            If this function has identified the folder as not being its responsiblity, it shall return null
            to give other handlers a chance to handle the directory.

            @param dir    [in]     Directory
            @param config [in]     Configuration. Must live until the result callback completes.
            @param then   [in,out] Result callback (moved-away if responsible, otherwise unchanged)
            @return Non-null task if the directory was understood. */
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then) = 0;
    };

} }

#endif
