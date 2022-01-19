/**
  *  \file game/browser/unsupportedaccountfolder.hpp
  *  \brief Class game::browser::UnsupportedAccountFolder
  */
#ifndef C2NG_GAME_BROWSER_UNSUPPORTEDACCOUNTFOLDER_HPP
#define C2NG_GAME_BROWSER_UNSUPPORTEDACCOUNTFOLDER_HPP

#include "afl/string/translator.hpp"
#include "game/browser/synchronousfolder.hpp"

namespace game { namespace browser {

    class Account;

    /** Unsupported account.
        Used to represent account entries that are not recognized by any of our Handlers. */
    class UnsupportedAccountFolder : public SynchronousFolder {
     public:
        /** Constructor.
            @param tx       Translator
            @param account  Account (for getName(), isSame()) */
        UnsupportedAccountFolder(afl::string::Translator& tx, const Account& account);

        /** Destructor. */
        ~UnsupportedAccountFolder();

        // Folder:
        virtual void loadContent(afl::container::PtrVector<Folder>& result);
        virtual bool loadConfiguration(game::config::UserConfiguration& config);
        virtual void saveConfiguration(const game::config::UserConfiguration& config);
        virtual bool setLocalDirectoryName(String_t directoryName);
        virtual std::auto_ptr<Task_t> loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t> then);
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
