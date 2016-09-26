/**
  *  \file game/browser/directoryhandler.hpp
  */
#ifndef C2NG_GAME_BROWSER_DIRECTORYHANDLER_HPP
#define C2NG_GAME_BROWSER_DIRECTORYHANDLER_HPP

#include "game/browser/handler.hpp"
#include "game/v3/rootloader.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    class Browser;

    class DirectoryHandler : public Handler {
     public:
        DirectoryHandler(Browser& b, afl::base::Ptr<afl::io::Directory> defaultSpecificationDirectory, util::ProfileDirectory& profile, afl::io::FileSystem& fs);
        virtual bool handleFolderName(String_t name, afl::container::PtrVector<Folder>& result);
        virtual Folder* createAccountFolder(Account& acc);
        virtual afl::base::Ptr<Root> loadGameRoot(afl::base::Ptr<afl::io::Directory> dir);

     private:
        // Associated browser. FIXME: can we get rid of this?
        Browser& m_browser;

        // RootLoader
        game::v3::RootLoader m_v3loader;
    };

} }

#endif
