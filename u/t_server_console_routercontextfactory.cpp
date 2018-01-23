/**
  *  \file u/t_server_console_routercontextfactory.cpp
  *  \brief Test for server::console::RouterContextFactory
  */

#include "server/console/routercontextfactory.hpp"

#include "t_server_console.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/line/linehandler.hpp"
#include "afl/net/line/protocolhandler.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/server.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/thread.hpp"
#include "afl/test/callreceiver.hpp"
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
void
TestServerConsoleRouterContextFactory::testIt()
{
    // This guy will talk network, so set one up
    afl::net::NetworkStack& ns = afl::net::NetworkStack::getInstance();
    afl::net::Name name("127.0.0.1", uint16_t(std::rand() % 10000 + 20000));
    afl::base::Ref<afl::net::Listener> listener = ns.listen(name, 10);

    // Create testee and configure it
    server::console::RouterContextFactory testee("ru", ns);
    TS_ASSERT_EQUALS(testee.handleConfiguration("RU.HOST", name.getName()), true);
    TS_ASSERT_EQUALS(testee.handleConfiguration("RU.PORT", name.getService()), true);
    TS_ASSERT_EQUALS(testee.handleConfiguration("ROUTER.HOST", "1.2.3.4"), false);
    TS_ASSERT_EQUALS(testee.handleConfiguration("RU.OTHER", "XYZ"), false);

    // Verify name
    TS_ASSERT_EQUALS(testee.getCommandName(), "ru");

    // Set up environment
    server::console::Environment env;
    server::console::NullTerminal term;
    afl::io::NullFileSystem fs;
    NullCommandHandler nch;
    server::console::Parser parser(env, term, fs, nch);

    // Start a server
    ServerMock mock("testIt");
    afl::net::Server server(listener, mock);
    afl::sys::Thread serverThread("TestServerConsoleRouterContextFactory", server);
    serverThread.start();

    // Make context
    std::auto_ptr<server::console::Context> ctx(testee.create());
    TS_ASSERT(ctx.get() != 0);
    TS_ASSERT_EQUALS(ctx->getName(), "ru");

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
        TS_ASSERT(ctx->call("foo", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT_EQUALS(afl::data::Access(value).toString(), "result\n");
    }

    // Select session
    {
        afl::data::Segment seg;
        std::auto_ptr<afl::data::Value> value;
        seg.pushBackString("7");
        TS_ASSERT(ctx->call("s", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT(value.get() == 0);
        TS_ASSERT_EQUALS(ctx->getName(), "ru:7");
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
        TS_ASSERT(ctx->call("s", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT_EQUALS(afl::data::Access(value).toString(), "thing 1\nthing 2\n");

        // We talked to a different session than the configured one; selected still ok
        TS_ASSERT_EQUALS(ctx->getName(), "ru:7");
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
        TS_ASSERT(ctx->call("get", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT_EQUALS(afl::data::Access(value).toString(), "the X\n");

        // We talked to a different session than the configured one; selected still ok
        TS_ASSERT_EQUALS(ctx->getName(), "ru:7");
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
        TS_ASSERT(ctx->call("save", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT_EQUALS(afl::data::Access(value).toString(), "");
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
        TS_ASSERT(ctx->call("save", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT_EQUALS(afl::data::Access(value).toString(), "");
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
        TS_ASSERT(ctx->call("repeat", interpreter::Arguments(seg, 0, seg.size()), parser, value));
        TS_ASSERT(afl::data::Access(value).toString().find("second") != String_t::npos);
    }

    // Stop
    server.stop();
    serverThread.join();
    mock.checkFinish();
}

