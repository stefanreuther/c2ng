/**
  *  \file u/t_client_applicationparameters.cpp
  *  \brief Test for client::ApplicationParameters
  */

#include "client/applicationparameters.hpp"

#include "t_client.hpp"
#include "afl/base/vectorenumerator.hpp"
#include "afl/except/commandlineexception.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/callreceiver.hpp"
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
void
TestClientApplicationParameters::testInit()
{
    Environment env("testInit");
    client::ApplicationParameters testee(env.app, "title");
    // Not contractual if no directory is given: TS_ASSERT_EQUALS(testee.getDirectoryMode(), client::ApplicationParameters::OpenGame);
    TS_ASSERT_EQUALS(testee.getGameDirectory().isValid(), false);
    TS_ASSERT_EQUALS(testee.getCommandLineResources().size(), 0U);
    TS_ASSERT_EQUALS(testee.getProxyAddress().isValid(), false);
    TS_ASSERT_EQUALS(testee.getPassword().isValid(), false);
    TS_ASSERT_EQUALS(testee.getTraceConfiguration().size(), 0U);
    TS_ASSERT_EQUALS(testee.getRequestThreadDelay(), 0);
    TS_ASSERT_EQUALS(testee.getPlayerNumber(), 0);
}

/** Test directory parameter. */
void
TestClientApplicationParameters::testDirectory()
{
    static const char*const ARGS[] = {"/dir"};
    Environment env("testDirectory");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getGameDirectory().orElse(""), "/dir");
    TS_ASSERT_EQUALS(testee.getDirectoryMode(), client::ApplicationParameters::OpenGame);
}

/** Test player number parameter. */
void
TestClientApplicationParameters::testPlayer()
{
    static const char*const ARGS[] = {"11","/dir"};
    Environment env("testPlayer");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getGameDirectory().orElse(""), "/dir");
    TS_ASSERT_EQUALS(testee.getDirectoryMode(), client::ApplicationParameters::OpenGame);
    TS_ASSERT_EQUALS(testee.getPlayerNumber(), 11);
}

/** Test "-size" option (WindowParameters). */
void
TestClientApplicationParameters::testSize()
{
    static const char*const ARGS[] = {"-size", "700x1300"};
    Environment env("testSize");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getWindowParameters().size, gfx::Point(700, 1300));
}

/** Test "-debug-request-delay" option. */
void
TestClientApplicationParameters::testRequestDelay()
{
    static const char*const ARGS[] = {"-debug-request-delay=335"};
    Environment env("testRequestDelay");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getRequestThreadDelay(), 335);
}

/** Test bad "-debug-request-delay" option. */
void
TestClientApplicationParameters::testBadRequestDelay()
{
    static const char*const ARGS[] = {"-debug-request-delay=booh"};
    Environment env("testRequestDelay");
    client::ApplicationParameters testee(env.app, "title");
    TS_ASSERT_THROWS(testee.parse(makeCommandLine(ARGS)), afl::except::CommandLineException);
}

/** Test "-dir" option. */
void
TestClientApplicationParameters::testDir()
{
    static const char*const ARGS[] = {"-dir", "/dir"};
    Environment env("testDir");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getGameDirectory().orElse(""), "/dir");
    TS_ASSERT_EQUALS(testee.getDirectoryMode(), client::ApplicationParameters::OpenBrowser);
}

/** Test "-log" option. */
void
TestClientApplicationParameters::testLog()
{
    static const char*const ARGS[] = {"-log=foo=show", "-log", "bar=hide"};
    Environment env("testLog");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getTraceConfiguration(), "foo=show:bar=hide");
}

/** Test "-password" option. */
void
TestClientApplicationParameters::testPassword()
{
    static const char*const ARGS[] = {"-password", "joshua"};
    Environment env("testPassword");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getPassword().orElse(""), "joshua");
}

/** Test "-proxy" option. */
void
TestClientApplicationParameters::testProxy()
{
    static const char*const ARGS[] = {"-proxy=127.0.0.1:5555"};
    Environment env("testProxy");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getProxyAddress().orElse(""), "127.0.0.1:5555");
}

/** Test "-resource" option. */
void
TestClientApplicationParameters::testResource()
{
    static const char*const ARGS[] = {"-resource", "a.res", "-resource=b.res"};
    Environment env("testResource");
    client::ApplicationParameters testee(env.app, "title");
    testee.parse(makeCommandLine(ARGS));

    TS_ASSERT_EQUALS(testee.getCommandLineResources().size(), 2U);
    TS_ASSERT_EQUALS(testee.getCommandLineResources()[0], "a.res");
    TS_ASSERT_EQUALS(testee.getCommandLineResources()[1], "b.res");
}

/** Test "-help" option. */
void
TestClientApplicationParameters::testHelp()
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
void
TestClientApplicationParameters::testBadOption()
{
    static const char*const ARGS[] = {"-notsupported"};
    Environment env("testBadOption");
    client::ApplicationParameters testee(env.app, "title");

    TS_ASSERT_THROWS(testee.parse(makeCommandLine(ARGS)), afl::except::CommandLineException);
}

/** Test bad parameters. */
void
TestClientApplicationParameters::testBadParameter()
{
    static const char*const ARGS[] = {"/dir", "7", "extra"};
    Environment env("testBadParameter");
    client::ApplicationParameters testee(env.app, "title");

    TS_ASSERT_THROWS(testee.parse(makeCommandLine(ARGS)), afl::except::CommandLineException);
}

