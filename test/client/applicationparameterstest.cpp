/**
  *  \file test/client/applicationparameterstest.cpp
  *  \brief Test for client::ApplicationParameters
  */

#include "client/applicationparameters.hpp"

#include "afl/base/vectorenumerator.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/application.hpp"

namespace {
    class Application : public gfx::Application {
     public:
        Application(afl::sys::Dialog& dialog, afl::string::Translator& tx)
            : gfx::Application(dialog, tx, "Test")
            { }
        virtual void appMain(gfx::Engine&)
            { }
    };

    class DialogMock : public afl::sys::Dialog, public afl::test::CallReceiver {
     public:
        DialogMock(afl::test::Assert a)
            : Dialog(), CallReceiver(a)
            { }
        virtual void showInfo(String_t /*info*/, String_t /*title*/)
            { checkCall("showInfo"); }
        virtual void showError(String_t /*info*/, String_t /*title*/)
            { checkCall("showError"); }
        virtual bool askYesNo(String_t /*info*/, String_t /*title*/)
            {
                checkCall("askYesNo");
                return consumeReturnValue<bool>();
            }
    };

    struct Environment {
        DialogMock dlg;
        afl::string::NullTranslator tx;
        Application app;

        Environment(afl::test::Assert a)
            : dlg(a), tx(), app(dlg, tx)
            { }
    };

    afl::base::Ref<afl::sys::Environment::CommandLine_t> makeCommandLine(afl::base::Memory<const char*const> args)
    {
        afl::base::Ref<afl::base::VectorEnumerator<String_t> > argVec = *new afl::base::VectorEnumerator<String_t>();
        while (const char*const* arg = args.eat()) {
            argVec->add(*arg);
        }
        return argVec;
    }
}

/** Test initialisation. */
AFL_TEST("client.ApplicationParameters:init", a)
{
    Environment env("testInit");
    client::ApplicationParameters testee(env.app, "title");
    // Not contractual if no directory is given: a.checkEqual("01. getDirectoryMode", testee.getDirectoryMode(), client::ApplicationParameters::OpenGame);
    a.checkEqual("02. getGameDirectory",        testee.getGameDirectory().isValid(), false);
    a.checkEqual("03. getCommandLineResources", testee.getCommandLineResources().size(), 0U);
    a.checkEqual("04. getProxyAddress",         testee.getProxyAddress().isValid(), false);
    a.checkEqual("05. getPassword",             testee.getPassword().isValid(), false);
    a.checkEqual("06. getTraceConfiguration",   testee.getTraceConfiguration().size(), 0U);
    a.checkEqual("07. getRequestThreadDelay",   testee.getRequestThreadDelay(), 0);
    a.checkEqual("08. getPlayerNumber",         testee.getPlayerNumber(), 0);
}

/** Test directory parameter. */
AFL_TEST("client.ApplicationParameters:directory", a)
{
    static const char*const ARGS[] = {"/dir"};
    Environment env("testDirectory");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getGameDirectory", testee.getGameDirectory().orElse(""), "/dir");
    a.checkEqual("02. getDirectoryMode", testee.getDirectoryMode(), client::ApplicationParameters::OpenGame);
}

/** Test player number parameter. */
AFL_TEST("client.ApplicationParameters:player", a)
{
    static const char*const ARGS[] = {"11","/dir"};
    Environment env("testPlayer");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getGameDirectory", testee.getGameDirectory().orElse(""), "/dir");
    a.checkEqual("02. getDirectoryMode", testee.getDirectoryMode(), client::ApplicationParameters::OpenGame);
    a.checkEqual("03. getPlayerNumber", testee.getPlayerNumber(), 11);
}

/** Test "-size" option (WindowParameters). */
AFL_TEST("client.ApplicationParameters:option:size", a)
{
    static const char*const ARGS[] = {"-size", "700x1300"};
    Environment env("testSize");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getWindowParameters X", testee.getWindowParameters().size.getX(), 700);
    a.checkEqual("02. getWindowParameters Y", testee.getWindowParameters().size.getY(), 1300);
}

/** Test "-debug-request-delay" option. */
AFL_TEST("client.ApplicationParameters:option:debug-request-delay", a)
{
    static const char*const ARGS[] = {"-debug-request-delay=335"};
    Environment env("testRequestDelay");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getRequestThreadDelay", testee.getRequestThreadDelay(), 335);
}

/** Test bad "-debug-request-delay" option. */
AFL_TEST("client.ApplicationParameters:option:debug-request-delay:bad", a)
{
    static const char*const ARGS[] = {"-debug-request-delay=booh"};
    Environment env("testRequestDelay");
    client::ApplicationParameters testee(env.app, "title");
    AFL_CHECK_THROWS(a(""), testee.parse(makeCommandLine(ARGS)), afl::except::CommandLineException);
}

/** Test "-dir" option. */
AFL_TEST("client.ApplicationParameters:option:dir", a)
{
    static const char*const ARGS[] = {"-dir", "/dir"};
    Environment env("testDir");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getGameDirectory", testee.getGameDirectory().orElse(""), "/dir");
    a.checkEqual("02. getDirectoryMode", testee.getDirectoryMode(), client::ApplicationParameters::OpenBrowser);
}

/** Test "-log" option. */
AFL_TEST("client.ApplicationParameters:option:log", a)
{
    static const char*const ARGS[] = {"-log=foo=show", "-log", "bar=hide"};
    Environment env("testLog");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getTraceConfiguration", testee.getTraceConfiguration(), "foo=show:bar=hide");
}

/** Test "-password" option. */
AFL_TEST("client.ApplicationParameters:option:password", a)
{
    static const char*const ARGS[] = {"-password", "joshua"};
    Environment env("testPassword");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getPassword", testee.getPassword().orElse(""), "joshua");
}

/** Test "-proxy" option. */
AFL_TEST("client.ApplicationParameters:option:proxy", a)
{
    static const char*const ARGS[] = {"-proxy=127.0.0.1:5555"};
    Environment env("testProxy");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01. getProxyAddress", testee.getProxyAddress().orElse(""), "127.0.0.1:5555");
}

/** Test "-resource" option. */
AFL_TEST("client.ApplicationParameters:option:resource", a)
{
    static const char*const ARGS[] = {"-resource", "a.res", "-resource=b.res"};
    Environment env("testResource");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    a.checkEqual("01", testee.getCommandLineResources().size(), 2U);
    a.checkEqual("02", testee.getCommandLineResources()[0], "a.res");
    a.checkEqual("03", testee.getCommandLineResources()[1], "b.res");
}

/** Test "-help" option. */
AFL_TEST_NOARG("client.ApplicationParameters:option:help")
{
    static const char*const ARGS[] = {"--help"};
    Environment env("testHelp");
    client::ApplicationParameters testee(env.app, "title");
    env.dlg.expectCall("showInfo");

    // parse() will exit by throwing an exception, but we do not know which one.
    try { testee.parse(makeCommandLine(ARGS)); }
    catch (...) { }

    env.dlg.checkFinish();
}

/** Test bad option. */
AFL_TEST("client.ApplicationParameters:bad-option", a)
{
    static const char*const ARGS[] = {"-notsupported"};
    Environment env("testBadOption");
    client::ApplicationParameters testee(env.app, "title");

    AFL_CHECK_THROWS(a(""), testee.parse(makeCommandLine(ARGS)), afl::except::CommandLineException);
}

/** Test bad parameters. */
AFL_TEST("client.ApplicationParameters:bad-parameter", a)
{
    static const char*const ARGS[] = {"/dir", "7", "extra"};
    Environment env("testBadParameter");
    client::ApplicationParameters testee(env.app, "title");

    AFL_CHECK_THROWS(a(""), testee.parse(makeCommandLine(ARGS)), afl::except::CommandLineException);
}
