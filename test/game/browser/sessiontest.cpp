/**
  *  \file test/game/browser/sessiontest.cpp
  *  \brief Test for game::browser::Session
  */

#include "game/browser/session.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "util/profiledirectory.hpp"

using afl::io::InternalFileSystem;
using afl::string::NullTranslator;
using afl::sys::InternalEnvironment;
using afl::sys::Log;
using util::ProfileDirectory;

namespace {
    std::auto_ptr<game::Task_t> makeTask(String_t& acc, String_t value)
    {
        class Task : public game::Task_t {
         public:
            Task(String_t& acc, String_t value)
                : m_acc(acc), m_value(value)
                { }
            void call()
                { m_acc += m_value; }
         private:
            String_t& m_acc;
            String_t m_value;
        };
        return std::auto_ptr<game::Task_t>(new Task(acc, value));
    }
}

AFL_TEST("game.browser.Session", a)
{
    // Environment
    InternalEnvironment env;
    env.setSettingsDirectoryName("/home");
    InternalFileSystem fs;
    fs.createDirectory("/home");
    ProfileDirectory profile(env, fs);
    NullTranslator tx;
    Log log;

    // Testee
    game::browser::Session testee(fs, tx, log, profile);

    // Linkage
    a.checkEqual("01. tx", &testee.translator(), &tx);
    a.checkEqual("02. log", &testee.log(), &log);
    a.checkNonNull("03. br", &testee.browser());
    a.checkNonNull("04. am", &testee.accountManager());
    a.checkNonNull("05. cb", &testee.callback());

    // Tasking
    String_t acc;
    testee.addTask(makeTask(acc, "a"));
    testee.addTask(makeTask(acc, "b"));
    testee.addTask(makeTask(acc, "c"));
    a.checkEqual("11. first task executed", acc, "a");

    testee.finishTask();
    a.checkEqual("12. second task executed", acc, "ab");

    testee.finishTask();
    a.checkEqual("13. third task executed", acc, "abc");
    testee.finishTask();

    testee.addTask(makeTask(acc, "d"));
    a.checkEqual("14. fourth task executed", acc, "abcd");
    testee.finishTask();
}
