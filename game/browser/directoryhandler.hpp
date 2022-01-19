/**
  *  \file game/browser/directoryhandler.hpp
  *  \brief Class game::browser::DirectoryHandler
  */
#ifndef C2NG_GAME_BROWSER_DIRECTORYHANDLER_HPP
#define C2NG_GAME_BROWSER_DIRECTORYHANDLER_HPP

#include "game/browser/handler.hpp"
#include "game/v3/rootloader.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    class Browser;

    /** Handler for file system directories.
        This class makes it possible to load local v3 games. */
    class DirectoryHandler : public Handler {
     public:
        /** Constructor.
            @param b                              Browser instance
            @param defaultSpecificationDirectory  Default specification directory (for files not present in game directory)
            @param profile                        Profile directory (for global configuration files, i.e. lru.ini/expr.ini) */
        DirectoryHandler(Browser& b, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory, util::ProfileDirectory& profile);

        /** Handle folder name or URL.
            Implementation of method from Handler:
            Checks whether the name refers to a local directory and, if so, produces a path for it.

            @param name   [in] Name or URL
            @param result [out] Result
            @retval false Folder name or URL not recognized
            @retval true Folder name or URL recognized, result has been populated */
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<Folder>& result);

        /** Create account folder.
            Implementation of method from Handler:
            DirectoryHandler cannot handle accounts.

            @param acc Account (mutable because it can eventually be modified through the created Folder)
            @return null */
        virtual Folder* createAccountFolder(Account& acc);

        /** Load game root for physical folder.
            Implementation of method from Handler:
            Produces a Root only if the directory has no game type (Game_Type) configured.

            @param dir    [in]     Directory
            @param config [in]     Configuration. Must live until the result callback completes.
            @param then   [in,out] Result callback (moved-away if responsible, otherwise unchanged)
            @return Non-null task if the directory was understood. */
        virtual std::auto_ptr<Task_t> loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then);

     private:
        // Associated browser
        Browser& m_browser;

        // RootLoader
        game::v3::RootLoader m_v3Loader;
    };

} }

#endif
