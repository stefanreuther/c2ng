/**
  *  \file test/game/browser/unsupportedaccountfoldertest.cpp
  *  \brief Test for game::browser::UnsupportedAccountFolder
  */

#include "game/browser/unsupportedaccountfolder.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/browser/account.hpp"
#include "game/browser/accountmanager.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/filesystemrootfolder.hpp"
#include "game/browser/optionalusercallback.hpp"

using afl::base::Ref;
using afl::string::NullTranslator;
using game::browser::Account;
using game::browser::AccountManager;
using game::browser::Browser;
using game::browser::FileSystemRootFolder;
using game::browser::Folder;
using game::browser::OptionalUserCallback;
using game::browser::UnsupportedAccountFolder;

namespace {
    class Receiver {
     public:
        Receiver(afl::test::Assert a)
            : m_beenHere(false),
              m_assert(a)
            { }

        void take(afl::base::Ptr<game::Root> r)
            {
                m_assert.checkNull("take: Root", r.get());
                m_beenHere = true;
            }

        bool beenHere() const
            { return m_beenHere; }

     private:
        bool m_beenHere;
        afl::test::Assert m_assert;
    };
}

/** Simple test.
    It's hard to test this class without repeating all the implementation,
    so this mainly tests we can instantiate the class. */
AFL_TEST("game.browser.UnsupportedAccountFolder", a)
{
    NullTranslator tx;
    Ref<Account> account = Account::create();
    account->setName("the acc");
    UnsupportedAccountFolder testee(tx, account);
    Ref<game::config::UserConfiguration> uc = game::config::UserConfiguration::create();

    // Accessors
    a.check         ("01. canEnter",              !testee.canEnter());
    a.checkEqual    ("02. getName",                testee.getName(), "the acc");
    a.checkDifferent("03. getText",                testee.getDescription().getText(), "");
    a.check         ("04. setLocalDirectoryName", !testee.setLocalDirectoryName("foo"));
    a.check         ("05. loadConfiguration",     !testee.loadConfiguration(*uc));
    a.checkEqual    ("06. getKind",                testee.getKind(), Folder::kAccount);

    // loadGameRoot
    Receiver recv(a("Receiver"));
    std::auto_ptr<game::Task_t> t = testee.loadGameRoot(*uc, std::auto_ptr<game::browser::LoadGameRootTask_t>(game::browser::LoadGameRootTask_t::makeBound(&recv, &Receiver::take)));
    a.checkNonNull("11. get", t.get());
    t->call();
    a.check("12. beenHere", recv.beenHere());

    // Dummies
    afl::container::PtrVector<Folder> result;
    testee.loadContent(result);
    a.check("21. loadContent", result.empty());

    AFL_CHECK_SUCCEEDS(a("22. saveConfiguration"), testee.saveConfiguration(*uc));
}

/*
 *  Comparisons
 */

// Compare against itself
AFL_TEST("game.browser.UnsupportedAccountFolder:compare:self", a)
{
    NullTranslator tx;
    Ref<Account> account = Account::create();
    UnsupportedAccountFolder testee(tx, account);

    a.check("01. isSame", testee.isSame(testee));
}

// Compare against another instance for same account
AFL_TEST("game.browser.UnsupportedAccountFolder:compare:same", a)
{
    NullTranslator tx;
    Ref<Account> account = Account::create();
    UnsupportedAccountFolder testee(tx, account);
    UnsupportedAccountFolder other(tx, account);

    a.check("01. isSame", testee.isSame(other));
    a.check("02. isSame", other.isSame(testee));
}

// Compare against another instance for other account
AFL_TEST("game.browser.UnsupportedAccountFolder:compare:different", a)
{
    NullTranslator tx;
    Ref<Account> account1 = Account::create();
    Ref<Account> account2 = Account::create();
    UnsupportedAccountFolder testee(tx, account1);
    UnsupportedAccountFolder other(tx, account2);

    a.check("01. isSame", !testee.isSame(other));
    a.check("02. isSame", !other.isSame(testee));
}

// Compare against other type
AFL_TEST("game.browser.UnsupportedAccountFolder:compare:other", a)
{
    // Environment
    NullTranslator tx;
    afl::io::NullFileSystem fs;
    afl::sys::Log log;
    afl::sys::InternalEnvironment env;
    util::ProfileDirectory profile(env, fs);

    AccountManager acc(profile, tx, log);
    OptionalUserCallback cb;
    Browser browser(fs, tx, log, acc, profile, cb);

    Ref<Account> account = Account::create();
    UnsupportedAccountFolder testee(tx, account);

    FileSystemRootFolder other(browser);

    a.check("01. isSame", !testee.isSame(other));
    a.check("02. isSame", !other.isSame(testee));
}
