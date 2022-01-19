/**
  *  \file game/nu/accountfolder.hpp
  */
#ifndef C2NG_GAME_NU_ACCOUNTFOLDER_HPP
#define C2NG_GAME_NU_ACCOUNTFOLDER_HPP

#include "game/browser/synchronousfolder.hpp"
#include "game/browser/account.hpp"

namespace game { namespace nu {

    class BrowserHandler;

    class AccountFolder : public game::browser::SynchronousFolder {
     public:
        AccountFolder(BrowserHandler& handler, game::browser::Account& acc);

        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual std::auto_ptr<game::browser::Task_t> loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t> then);
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
