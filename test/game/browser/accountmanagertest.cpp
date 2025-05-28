/**
  *  \file test/game/browser/accountmanagertest.cpp
  *  \brief Test for game::browser::AccountManager
  */

#include "game/browser/accountmanager.hpp"

#include "afl/io/filesystem.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"

using afl::base::Ref;
using afl::io::FileSystem;
using afl::io::Stream;
using afl::io::TextFile;
using game::browser::Account;
using game::browser::AccountManager;

namespace {
    struct Environment {
        afl::sys::InternalEnvironment env;
        afl::io::InternalFileSystem fs;
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        std::auto_ptr<util::ProfileDirectory> pProfile;

        Environment()
            : env(), fs(), tx(), log()
            {
                fs.createDirectory("/set");
                env.setSettingsDirectoryName("/set");
                pProfile.reset(new util::ProfileDirectory(env, fs));
            }
    };
}

// Initialisation smoke test
AFL_TEST("game.browser.AccountManager:init", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);

    a.checkEqual("01. getNumAccounts", testee.getNumAccounts(), 0U);
    a.checkEqual("02. translator",    &testee.translator(), &env.tx);
    a.checkEqual("03. log",           &testee.log(), &env.log);
    a.checkNull ("04. find",           testee.findAccount("", "", ""));
    a.checkNull ("05. get",            testee.getAccount(0));
}

// Adding and finding an account
AFL_TEST("game.browser.AccountManager:addNewAccount", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);

    Ref<Account> acc = Account::create();
    acc->setUser("u");
    acc->setType("t");
    acc->setHost("h");
    testee.addNewAccount(acc);

    a.checkEqual("01. getNumAccounts", testee.getNumAccounts(), 1U);
    a.checkEqual("02. find ok",        testee.findAccount("u", "t", "h"), &*acc);
    a.checkEqual("03. get ok",         testee.getAccount(0), &*acc);

    a.checkNull("11. find mismatch",   testee.findAccount("u", "t", "x"));
    a.checkNull("12. find mismatch",   testee.findAccount("u", "x", "h"));
    a.checkNull("13. find mismatch",   testee.findAccount("x", "t", "h"));
    a.checkNull("14. find mismatch",   testee.findAccount("U", "T", "H"));  // case sensitive!
    a.checkNull("15. get mismatch",    testee.getAccount(1));
}

// Save
AFL_TEST("game.browser.AccountManager:save", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);

    Ref<Account> acc = Account::create();
    acc->setUser("u");
    acc->setType("t");
    acc->setHost("h");
    acc->setName("n");
    acc->setGameFolderName("id", "/dir");
    testee.addNewAccount(acc);

    AFL_CHECK_SUCCEEDS(a("01. save"),  testee.save());

    // Read file
    // Ignore ";" and empty lines so comments/spacing is not contractual
    Ref<Stream> in = env.fs.openFile("/set/network.ini", FileSystem::OpenRead);
    TextFile tf(*in);
    String_t total;
    String_t line;
    while (tf.readLine(line)) {
        if (!line.empty() && line[0] != ';') {
            total += line;
            total += '\n';
        }
    }

    // Verify
    a.checkEqual("11. save result", total,
                 "[n]\n"
                 "game:id=/dir\n"
                 "host=h\n"
                 "type=t\n"
                 "user=u\n");
}

// Save, error: colliding name
AFL_TEST("game.browser.AccountManager:save:error", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);
    afl::test::LogListener counter;
    env.log.addListener(counter);

    // Create colliding directories.
    // Currently, AccountManager is not smart enough to get rid of these.
    env.fs.createDirectory("/set/network.bak");
    env.fs.createDirectory("/set/network.bak/sub");
    env.fs.createDirectory("/set/network.ini");
    env.fs.createDirectory("/set/network.ini/sub");

    Ref<Account> acc = Account::create();
    acc->setUser("u");
    acc->setType("t");
    acc->setHost("h");
    acc->setName("n");
    acc->setGameFolderName("id", "/dir");
    testee.addNewAccount(acc);

    AFL_CHECK_SUCCEEDS(a("01. save"), testee.save());
    a.check("02. messages", counter.getNumErrors() > 0);
}

// Load
AFL_TEST("game.browser.AccountManager:load", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);
    env.fs.openFile("/set/network.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes(";comment\n"
                                         "[n]\n"
                                         "game:id=/dir\n"
                                         "host=h\n"
                                         "type=t\n"
                                         "user=u\n"
                                         "[mm]\n"
                                         "type=t2\n"
                                         "host=h2\n"
                                         "user=u2\n"));
    testee.load();

    // Verify
    a.checkEqual("01. num", testee.getNumAccounts(), 2U);

    Account* p1 = testee.findAccount("u", "t", "h");
    a.checkNonNull("11. find", p1);
    a.checkEqual("12. get", p1, testee.getAccount(0));
    a.checkEqual("13. name", p1->getName(), "n");
    a.checkEqual("14. host", p1->getHost(), "h");
    a.checkEqual("15. type", p1->getType(), "t");
    a.checkEqual("16. user", p1->getUser(), "u");
    a.checkEqual("17. path", *p1->getGameFolderName("id"), "/dir");

    Account* p2 = testee.findAccount("u2", "t2", "h2");
    a.checkNonNull("21. find", p2);
    a.checkEqual("22. get", p2, testee.getAccount(1));
    a.checkEqual("23. name", p2->getName(), "mm");
    a.checkEqual("24. host", p2->getHost(), "h2");
    a.checkEqual("25. type", p2->getType(), "t2");
    a.checkEqual("26. user", p2->getUser(), "u2");
    a.checkNull("27. path", p2->getGameFolderName("id"));
}

// Load, incomplete account
AFL_TEST("game.browser.AccountManager:load:error:incomplete", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);
    env.fs.openFile("/set/network.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes("[n]\n"
                                         "game:id=/dir\n"
                                         // no "user="
                                         "host=h\n"
                                         "type=t\n"));
    testee.load();
    a.checkEqual("01. num", testee.getNumAccounts(), 0U);
}

// Load, missing section header
AFL_TEST("game.browser.AccountManager:load:error:no-section", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);
    env.fs.openFile("/set/network.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes(";comment\n"
                                         "game:id=/dir\n"
                                         "host=h\n"
                                         "type=t\n"
                                         "user=u\n"));
    testee.load();
    a.checkEqual("01. num", testee.getNumAccounts(), 0U);
}

// Load, syntax error
AFL_TEST("game.browser.AccountManager:load:error:syntax", a)
{
    Environment env;
    AccountManager testee(*env.pProfile, env.tx, env.log);
    env.fs.openFile("/set/network.ini", FileSystem::Create)
        ->fullWrite(afl::string::toBytes(";comment\n"
                                         "[n]\n"
                                         "host\n"
                                         "type\n"
                                         "user\n"));
    testee.load();
    a.checkEqual("01. num", testee.getNumAccounts(), 0U);
}
