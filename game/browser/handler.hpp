/**
  *  \file game/browser/handler.hpp
  */
#ifndef C2NG_GAME_BROWSER_HANDLER_HPP
#define C2NG_GAME_BROWSER_HANDLER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ptr.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/root.hpp"

namespace game { namespace browser {

    class Folder;
    class Account;

    class Handler : public afl::base::Deletable {
     public:
        /** Handle folder name or URL.
            Function must check if it can handle the folder name, and if so, populate the result.

            Possible inputs and outputs:
            - local folder names (produces path on local system, i.e. "My Computer > Root > Path > Path")
            - network game URLs (produces path into user account, i.e. "Account > Games > The Game")
            - network storage URLs (produces path into user account, i.e. "Account > Files > Path")

            A network game URL could be a direct URL ("http://example.com/game/1") or a system-specific pseudo URL ("example://1").

            This function is only allowed to throw if it has positively identified the name/URL as being its responsibility.
            Exceptions will stop the handler search and prevent other handlers from being given a chance to handle the name/URL.

            \param name   [in] Name or URL
            \param result [out] Result
            \retval false Folder name or URL not recognized
            \retval true Folder name or URL recognized, result has been populated */
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<Folder>& result) = 0;

        /** Create account folder.
            Function must check if it can handle the given account, and if so, produce a Folder instance that describes the account root.

            \param acc Account
            \return Folder or null */
        virtual Folder* createAccountFolder(Account& acc) = 0;

        /** Load game root for physical folder.
            Routine must check if it can handle the given folder, and if so, produce a Root.

            If the folder contains an entirely local game, this must create a plain local Root.

            If the folder contains a network game, this must create a network Root.

            The configuration passed will be the content of the "pcc2.ini" file from the directory.
            It shall be consulted for the "Game_" options.
            - Game_Type / Game_User / Game_Host / Game_Id for a network game association
            - Game_Finished / Game_ReadOnly to determine the access level of loaded data
            - Game_AccessHostFiles to configure access to host files
            - Game_Charset to configure the character set

            This function is only allowed to throw if it has positively identified the directory as being its responsibility.
            Exceptions will stop the handler search and prevent other handlers from being given a chance to handle the directory.

            \param dir directory
            \param config configuration
            \return Non-null root if the directory was understood. */
        virtual afl::base::Ptr<Root> loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config) = 0;

    };

} }

#endif
