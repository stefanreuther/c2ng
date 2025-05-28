/**
  *  \file game/browser/accountmanager.hpp
  *  \brief Class game::browser::AccountManager
  */
#ifndef C2NG_GAME_BROWSER_ACCOUNTMANAGER_HPP
#define C2NG_GAME_BROWSER_ACCOUNTMANAGER_HPP

#include <vector>

#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/loglistener.hpp"
#include "game/browser/account.hpp"
#include "util/profiledirectory.hpp"

namespace game { namespace browser {

    /** Account manager.
        Provides a container for a set of accounts,
        and methodos to load and save it to the profile. */
    class AccountManager {
     public:
        /** Constructor.
            @param profile   Profile directory
            @param tx        Translator
            @param log       Logger */
        AccountManager(util::ProfileDirectory& profile, afl::string::Translator& tx, afl::sys::LogListener& log);

        /** Destructor. */
        ~AccountManager();

        /** Access translator.
            @return Translator as given to constructor. */
        afl::string::Translator& translator();

        /** Access logger.
            @return Logger as given to constructor. */
        afl::sys::LogListener& log();

        /** Add a new account.
            The given Account object must be newly-allocated and valid (Account::isValid()).
            @param p New account */
        void addNewAccount(const afl::base::Ref<Account>& p);

        /** Find an account, given parameters.
            @param user User (match Account::getUser())
            @param type Type (match Account::getType())
            @param host Host (match Account::getHost())
            @return Account with matching parameters; null if none */
        Account* findAccount(String_t user, String_t type, String_t host) const;

        /** Get number of accounts.
            @return Number */
        size_t getNumAccounts() const;

        /** Get account, given an index.
            @param index Index [0,getNumAccounts())
            @return Account; null if none */
        Account* getAccount(size_t index) const;

        /** Load from profile directory.
            Loads the accounts as given in the network.ini file.
            Errors are logged. */
        void load();

        /** Save to profile directory.
            Saves the accounts to the network.ini file.
            Errors are logged. */
        void save();

     private:
        std::vector<afl::base::Ref<Account> > m_accounts;
        util::ProfileDirectory& m_profile;
        afl::string::Translator& m_translator;
        afl::sys::LogListener& m_log;
    };

} }

inline afl::string::Translator&
game::browser::AccountManager::translator()
{
    return m_translator;
}

inline afl::sys::LogListener&
game::browser::AccountManager::log()
{
    return m_log;
}

#endif
