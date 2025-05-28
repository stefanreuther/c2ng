/**
  *  \file game/pcc/accountfolder.hpp
  *  \brief Class game::pcc::AccountFolder
  */
#ifndef C2NG_GAME_PCC_ACCOUNTFOLDER_HPP
#define C2NG_GAME_PCC_ACCOUNTFOLDER_HPP

#include "game/browser/folder.hpp"
#include "game/browser/account.hpp"

namespace game { namespace pcc {

    class BrowserHandler;

    /** Account folder.
        Displays the games in one account.
        For now, a flat structure. */
    class AccountFolder : public game::browser::Folder {
     public:
        /** Constructor.
            @param handler Parent BrowserHandler
            @param acc     Account */
        AccountFolder(BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& acc);

        // Folder:
        virtual std::auto_ptr<Task_t> loadContent(std::auto_ptr<game::browser::LoadContentTask_t> then);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual std::auto_ptr<Task_t> loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t> then);
        virtual String_t getName() const;
        virtual util::rich::Text getDescription() const;
        virtual bool isSame(const Folder& other) const;
        virtual bool canEnter() const;
        virtual Kind getKind() const;

     private:
        BrowserHandler& m_handler;
        const afl::base::Ref<game::browser::Account> m_account;
    };

} }

#endif
