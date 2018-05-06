/**
  *  \file game/browser/unsupportedaccountfolder.hpp
  */
#ifndef C2NG_GAME_BROWSER_UNSUPPORTEDACCOUNTFOLDER_HPP
#define C2NG_GAME_BROWSER_UNSUPPORTEDACCOUNTFOLDER_HPP

#include "game/browser/folder.hpp"
#include "afl/string/translator.hpp"

namespace game { namespace browser {

    class Account;

    class UnsupportedAccountFolder : public Folder {
     public:
        UnsupportedAccountFolder(afl::string::Translator& tx, const Account& account);

        ~UnsupportedAccountFolder();

        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual afl::base::Ptr<Root> loadGameRoot(const game::config::UserConfiguration& config);
        virtual String_t getName() const;
        virtual util::rich::Text getDescription() const;
        virtual bool isSame(const Folder& other) const;
        virtual bool canEnter() const;
        virtual Kind getKind() const;

     private:
        afl::string::Translator& m_translator;
        const Account& m_account;
    };

} }


#endif
