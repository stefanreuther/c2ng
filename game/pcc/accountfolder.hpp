/**
  *  \file game/pcc/accountfolder.hpp
  */
#ifndef C2NG_GAME_PCC_ACCOUNTFOLDER_HPP
#define C2NG_GAME_PCC_ACCOUNTFOLDER_HPP

#include "game/browser/folder.hpp"
#include "game/browser/account.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    class AccountFolder : public game::browser::Folder {
     public:
        AccountFolder(BrowserHandler& handler, game::browser::Account& acc);

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
    };

} }

#endif
