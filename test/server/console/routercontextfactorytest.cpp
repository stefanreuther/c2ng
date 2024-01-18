/**
  *  \file test/server/console/routercontextfactorytest.cpp
  *  \brief Test for server::console::RouterContextFactory
  */

#include "server/console/routercontextfactory.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/line/linehandler.hpp"
#include "afl/net/line/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/console/commandhandler.hpp"
#include "server/console/environment.hpp"
#include "server/console/nullterminal.hpp"
#include "server/console/parser.hpp"

namespace {
    class ServerMock : public afl::net::line::LineHandler,
                       public afl::test::CallReceiver,
                       public afl::net::ProtocolHandlerFactory
    {
     public:
        ServerMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::line::ProtocolHandler(*this); }
        virtual bool handleOpening(afl::net::line::LineSink& /*response*/)
            {
                checkCall("handleOpening");
                return false;
            }
        virtual bool handleLine(const String_t& line, afl::net::line::LineSink& response)
            {
                checkCall("handleLine:" + line);
                int n = consumeReturnValue<int>();
                while (n > 0) {
                    response.handleLine(consumeReturnValue<String_t>());
                    --n;
                }
                return consumeReturnValue<bool>();
            }
        virtual void handleConnectionClose()
            { }
    };

    class NullCommandHandler : public server::console::CommandHandler {
     public:
        virtual bool call(const String_t& /*cmd*/, interpreter::Arguments /*args*/, server::console::Parser& /*parser*/, std::auto_ptr<afl::data::Value>& /*result*/)
            { return false; }
    };
}

/** Test all variations. */
AFL_TEST("server.console.RouterContextFactory", a)
{
    // This guy will talk network, so set one up
    afl::net::NetworkStack& ns = afl::net::NetworkStack::getInstance();
    afl::net::Name name("127.0.0.1", uint16_t(std::rand() % 10000 + 20000));
    afl::base::Ref<afl::net::Listener> listener = ns.listen(name, 10);

    // Create testee and configure it
    server::console::RouterContextFactory testee("ru", ns);
    a.checkEqual("01. config host", testee.handleConfiguration("RU.HOST", name.getName()), true);
    a.checkEqual("02. config port", testee.handleConfiguration("RU.PORT", name.getService()), true);
    a.checkEqual("03. config host", testee.handleConfiguration("ROUTER.HOST", "1.2.3.4"), false);
    a.checkEqual("04. config other", testee.handleConfiguration("RU.OTHER", "XYZ"), false);

    // Verify name
    a.checkEqual("11. getCommandName", testee.getCommandName(), "ru");

    // Set up environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler nch;
    server::console::Parser parser(env, term, fs, nch);

    // Start a server
    ServerMock mock(a);
    afl::net::Server server(listener, mock);
    afl::sys::Thread serverThread(a.getLocation(), server);
    serverThread.start();

    // Make context
    std::auto_ptr<server::console::Context> ctx(testee.create());
    a.checkNonNull("21. create", ctx.get());
    a.checkEqual("22. getName", ctx->getName(), "ru");

    // Test simple command
    {
        mock.expectCall("handleOpening");
        mock.expectCall("handleLine:foo bar");
        mock.provideReturnValue(1);
        mock.provideReturnValue(String_t("result"));
        mock.provideReturnValue(true);
        afl::data::Segment seg;
        std::auto_ptr<afl::data::Value> value;
        seg.pushBackString("bar");
        a.check("31. call", ctx->call("foo", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.checkEqual("32. result", afl::data::Access(value).toString(), "result\n");
    }

    // Select session
    {
        afl::data::Segment seg;
        std::auto_ptr<afl::data::Value> value;
        seg.pushBackString("7");
        a.check("41. call", ctx->call("s", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.checkNull("42. result", value.get());
        a.checkEqual("43. getName", ctx->getName(), "ru:7");
    }

    // Talk to a session
    {
        mock.expectCall("handleOpening");

        // First line (does not produce result yet)
        mock.expectCall("handleLine:S 3");
        mock.provideReturnValue(0);
        mock.provideReturnValue(false);

        // Second line (does produce result)
        mock.expectCall("handleLine:get thing");
        mock.provideReturnValue(2);
        mock.provideReturnValue(String_t("thing 1"));
        mock.provideReturnValue(String_t("thing 2"));
        mock.provideReturnValue(true);

        afl::data::Segment seg;
        std::auto_ptr<afl::data::Value> value;
        seg.pushBackString("3");
        seg.pushBackString("get");
        seg.pushBackString("thing");
        a.check("51. call", ctx->call("s", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.checkEqual("52. result", afl::data::Access(value).toString(), "thing 1\nthing 2\n");

        // We talked to a different session than the configured one; selected still ok
        a.checkEqual("61. getName", ctx->getName(), "ru:7");
    }

    // Talk to a session, implicitly
    {
        mock.expectCall("handleOpening");

        // First line (does not produce result yet)
        mock.expectCall("handleLine:S 7");
        mock.provideReturnValue(0);
        mock.provideReturnValue(false);

        // Second line (does produce result)
        mock.expectCall("handleLine:get X");
        mock.provideReturnValue(1);
        mock.provideReturnValue(String_t("the X"));
        mock.provideReturnValue(true);

        afl::data::Segment seg;
        std::auto_ptr<afl::data::Value> value;
        seg.pushBackString("X");
        a.check("71. call", ctx->call("get", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.checkEqual("72. result", afl::data::Access(value).toString(), "the X\n");

        // We talked to a different session than the configured one; selected still ok
        a.checkEqual("81. getName", ctx->getName(), "ru:7");
    }

    // Nullary save, goes to session
    {
        mock.expectCall("handleOpening");

        // First line (does not produce result yet)
        mock.expectCall("handleLine:S 7");
        mock.provideReturnValue(0);
        mock.provideReturnValue(false);

        // Second line (does produce result)
        mock.expectCall("handleLine:save");
        mock.provideReturnValue(0);
        mock.provideReturnValue(true);

        afl::data::Segment seg;
        std::auto_ptr<afl::data::Value> value;
        a.check("91. call", ctx->call("save", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.checkEqual("92. result", afl::data::Access(value).toString(), "");
    }

    // Non-nullary save, goes to router
    {
        mock.expectCall("handleOpening");
        mock.expectCall("handleLine:save 48");
        mock.provideReturnValue(0);
        mock.provideReturnValue(true);

        afl::data::Segment seg;
        seg.pushBackString("48");
        std::auto_ptr<afl::data::Value> value;
        a.check("101. call", ctx->call("save", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.checkEqual("102. result", afl::data::Access(value).toString(), "");
    }

    // Repeat
    {
        for (int i = 0; i < 5; ++i) {
            mock.expectCall("handleOpening");
            mock.expectCall("handleLine:list");
            mock.provideReturnValue(0);
            mock.provideReturnValue(true);
        }

        afl::data::Segment seg;
        seg.pushBackString("5");
        seg.pushBackString("list");
        std::auto_ptr<afl::data::Value> value;
        a.check("111. call", ctx->call("repeat", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        a.check("112. result", afl::data::Access(value).toString().find("second") != String_t::npos);
    }

    // Stop
    server.stop();
    serverThread.join();
    mock.checkFinish();
}
