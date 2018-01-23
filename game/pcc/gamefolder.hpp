/**
  *  \file game/pcc/gamefolder.hpp
  */
#ifndef C2NG_GAME_PCC_GAMEFOLDER_HPP
#define C2NG_GAME_PCC_GAMEFOLDER_HPP

#include "game/browser/folder.hpp"
#include "game/browser/account.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "game/v3/rootloader.hpp"
#include "afl/data/access.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    class GameFolder : public game::browser::Folder {
     public:
        GameFolder(BrowserHandler& handler, game::browser::Account& acc, String_t path, size_t hint);

        // Folder:
        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual afl::base::Ptr<game::Root> loadGameRoot();
        virtual String_t getName() const;
        virtual util::rich::Text getDescription() const;
        virtual bool isSame(const Folder& other) const;
        virtual bool canEnter() const;
        virtual Kind getKind() const;

     private:
        BrowserHandler& m_handler;
        game::browser::Account& m_account;
        String_t m_path;
        mutable size_t m_hint;
        afl::io::NullFileSystem m_nullFS;
        game::v3::RootLoader m_v3Loader;

        afl::data::Access getGameListEntry() const;
    };

} }

#endif
