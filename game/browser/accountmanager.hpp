/**
  *  \file game/browser/accountmanager.hpp
  */
#ifndef C2NG_GAME_BROWSER_ACCOUNTMANAGER_HPP
#define C2NG_GAME_BROWSER_ACCOUNTMANAGER_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    class Account;

    class AccountManager {
     public:
        explicit AccountManager(util::ProfileDirectory& profile, afl::string::Translator& tx, afl::sys::LogListener& log);
        ~AccountManager();

        void addNewAccount(Account* p);

        Account* findAccount(String_t user, String_t type, String_t host) const;

        size_t getNumAccounts() const;

        Account* getAccount(size_t index) const;

        void load();

        void save();

        // FIXME: must add some hooks; most importantly, a pre-delete one to tell people who hold a reference to an Account to go away.

     private:
        afl::container::PtrVector<Account> m_accounts;
        util::ProfileDirectory& m_profile;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

#endif
