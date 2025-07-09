/**
  *  \file game/browser/accountmanager.cpp
  *  \brief Class game::browser::AccountManager
  */

#include "game/browser/accountmanager.hpp"

#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "util/fileparser.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::io::TextFile;
using afl::string::Format;
using afl::sys::LogListener;
using game::browser::Account;
using game::browser::AccountManager;

namespace {
    const char LOG_NAME[] = "game.browser.account";
    const char INI_FILE[] = "network.ini";
    const char NEW_FILE[] = "network.new";
    const char OLD_FILE[] = "network.bak";

    class AccountFileParser : public util::FileParser {
     public:
        AccountFileParser(AccountManager& mgr)
            : FileParser(";#"),
              m_manager(mgr),
              m_account()
            { }

        virtual void handleLine(const String_t& fileName, int lineNr, String_t line)
            {
                line = afl::string::strTrim(line);
                if (!line.empty() && line[0] == '[') {
                    // "[account name]"
                    finish();
                    line.erase(0, 1);
                    String_t::size_type n = line.rfind(']');
                    if (n != String_t::npos) {
                        line.erase(n);
                    }
                    m_account = Account::create().asPtr();
                    m_account->setName(line);
                } else if (m_account.get() != 0) {
                    // Assignment for account
                    String_t::size_type n = line.find('=');
                    if (n != String_t::npos) {
                        m_account->set(afl::string::strRTrim(line.substr(0, n)), afl::string::strLTrim(line.substr(n+1)), true);
                    } else {
                        m_manager.log().write(LogListener::Warn, LOG_NAME, fileName, lineNr, m_manager.translator()("Syntax error, line has been ignored"));
                    }
                } else {
                    // line before first account; ignore
                }
            }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }

        void finish()
            {
                if (m_account.get() != 0) {
                    if (m_account->isValid()) {
                        m_manager.log().write(LogListener::Debug, LOG_NAME, Format(m_manager.translator()("Adding network account \"%s\""), m_account->getName()));
                        m_manager.addNewAccount(*m_account);
                    } else {
                        m_manager.log().write(LogListener::Warn, LOG_NAME, Format(m_manager.translator()("Incomplete network account \"%s\" has been ignored"), m_account->getName()));
                    }
                }
                m_account.reset();
            }

     private:
        AccountManager& m_manager;
        Ptr<Account> m_account;
    };
}

/*
 *  AccountManager
 */

game::browser::AccountManager::AccountManager(util::ProfileDirectory& profile, afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_accounts(),
      m_profile(profile),
      m_translator(tx),
      m_log(log)
{ }

game::browser::AccountManager::~AccountManager()
{ }

void
game::browser::AccountManager::addNewAccount(const afl::base::Ref<Account>& p)
{
    m_accounts.push_back(p.asPtr());
}

game::browser::Account*
game::browser::AccountManager::findAccount(String_t user, String_t type, String_t host) const
{
    for (size_t i = 0, n = m_accounts.size(); i < n; ++i) {
        Account* p = &*m_accounts[i];
        if (p->getUser() == user && p->getType() == type && p->getHost() == host) {
            return p;
        }
    }
    return 0;
}

size_t
game::browser::AccountManager::getNumAccounts() const
{
    return m_accounts.size();
}

game::browser::Account*
game::browser::AccountManager::getAccount(size_t index) const
{
    return (index < m_accounts.size()
            ? &*m_accounts[index]
            : 0);
}

void
game::browser::AccountManager::load()
{
    Ptr<Stream> file = m_profile.openFileNT(INI_FILE);
    if (file.get() != 0) {
        AccountFileParser p(*this);
        p.parseFile(*file);
        p.finish();
    }
}

void
game::browser::AccountManager::save()
{
    try {
        Ref<afl::io::Directory> dir = m_profile.open();

        // Create new file
        {
            Ref<Stream> file = dir->openFile(NEW_FILE, FileSystem::Create);
            TextFile tf(*file);
            tf.writeLine("; PCC2ng Network Configuration");
            tf.writeLine();
            for (size_t i = 0, n = m_accounts.size(); i < n; ++i) {
                m_accounts[i]->write(tf);
                tf.writeLine();
            }
            tf.flush();
        }

        // Rename
        // First move old files out of the way; if that fails, ok.
        dir->eraseNT(OLD_FILE);
        dir->getDirectoryEntryByName(INI_FILE)->renameToNT(OLD_FILE);

        // This one should not fail. If it does, catch and log it.
        dir->getDirectoryEntryByName(NEW_FILE)->renameTo(INI_FILE);
    }
    catch (std::exception& e) {
        m_log.write(LogListener::Error, LOG_NAME, m_translator("Error updating network accounts file"), e);
    }
}
