/**
  *  \file test/server/talk/serverapplicationtest.cpp
  *  \brief Test for server::talk::ServerApplication
  */

#include "server/talk/serverapplication.hpp"

#include "afl/async/internalinterrupt.hpp"
#include "afl/base/stoppable.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/net/internalnetworkstack.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "afl/net/redis/internaldatabase.hpp"
#include "afl/net/nullcommandhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/sys/thread.hpp"
#include "afl/net/resp/client.hpp"

using afl::async::InternalInterrupt;
using afl::async::InterruptOperation;
using afl::base::Ref;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::net::InternalNetworkStack;
using afl::sys::InternalEnvironment;

namespace {
    class Factory : public afl::net::ProtocolHandlerFactory {
     public:
        Factory(afl::net::CommandHandler& hdl)
            : m_handler(hdl)
            { }
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_handler); }
     private:
        afl::net::CommandHandler& m_handler;
    };

    /** Test environment.
        Stores all required objects.
        To run the application, use run() directly, or run this in a thread.
        The stop() method triggers a stop signal; why not. */
    struct Environment : public afl::base::Stoppable {
        // Environment for app
        InternalEnvironment env;
        Ref<InternalStream> out;
        InternalFileSystem fs;
        Ref<InternalNetworkStack> net;
        InternalInterrupt intr;

        // External DB server
        afl::net::redis::InternalDatabase db;
        Factory dbFactory;
        std::auto_ptr<afl::net::Server> dbServer;
        std::auto_ptr<afl::sys::Thread> dbThread;

        // External mail server
        afl::net::NullCommandHandler mailout;
        Factory mailoutFactory;
        std::auto_ptr<afl::net::Server> mailoutServer;
        std::auto_ptr<afl::sys::Thread> mailoutThread;

        // Return code
        int exitCode;

        Environment()
            : env(), out(*new InternalStream()), fs(), net(InternalNetworkStack::create()), intr(),
              db(), dbFactory(db), dbServer(), dbThread(),
              mailout(), mailoutFactory(mailout), mailoutServer(), mailoutThread(),
              exitCode(-999)
            {
                env.setChannelStream(afl::sys::Environment::Output, out.asPtr());
                env.setChannelStream(afl::sys::Environment::Error, out.asPtr());
            }

        virtual void run()
            {
                exitCode = server::talk::ServerApplication(env, fs, *net, intr).run();
            }

        virtual void stop()
            {
                intr.post(InterruptOperation::Kinds_t() + InterruptOperation::Break);
            }
    };

    void startDatabaseServer(Environment& env, afl::net::Name listenAddress)
    {
        env.dbServer.reset(new afl::net::Server(env.net->listen(listenAddress, 10), env.dbFactory));
        env.dbThread.reset(new afl::sys::Thread("dbServer", *env.dbServer));
        env.dbThread->start();
    }

    void startMailServer(Environment& env, afl::net::Name listenAddress)
    {
        env.mailoutServer.reset(new afl::net::Server(env.net->listen(listenAddress, 10), env.mailoutFactory));
        env.mailoutThread.reset(new afl::sys::Thread("mailoutServer", *env.mailoutServer));
        env.mailoutThread->start();
    }
}

AFL_TEST("server.talk.ServerApplication:help", a)
{
    // Create environment
    Environment env;

    // - add command line
    afl::data::StringList_t list;
    list.push_back("--help");
    env.env.setCommandLine(list);

    // Testee
    env.run();
    a.checkEqual("01. exit", env.exitCode, 0);

    // Verify result
    a.check("11. content", env.out->getContent().size() > 100);
}


AFL_TEST("server.talk.ServerApplication:net", a)
{
    // Create environment
    Environment env;

    // - add command line
    afl::data::StringList_t list;
    list.push_back("-Dmailout.host=MH");
    list.push_back("-Dmailout.port=MP");
    list.push_back("-Dredis.host=DBH");
    list.push_back("-Dredis.port=DBP");
    list.push_back("-Dtalk.host=TH");
    list.push_back("-Dtalk.port=TP");
    env.env.setCommandLine(list);

    // Environment
    startMailServer(env, afl::net::Name("MH", "MP"));
    startDatabaseServer(env, afl::net::Name("DBH", "DBP"));

    // Testee
    afl::sys::Thread serverThread("serverThread", env);
    serverThread.start();

    afl::sys::Thread::sleep(100);

    // Perform a simple operation
    afl::net::resp::Client client(*env.net, afl::net::Name("TH", "TP"));
    String_t op = client.callString(afl::data::Segment()
                                    .pushBackString("render")
                                    .pushBackString("forum:[b]hello[/b]")
                                    .pushBackString("format")
                                    .pushBackString("html"));
    a.checkEqual("01. result", op, "<p><b>hello</b></p>\n");

    // Stop it
    env.stop();
    serverThread.join();
    a.checkEqual("11. exit", env.exitCode, 0);
}


AFL_TEST("server.talk.ServerApplication:options:good", a)
{
    // Create environment
    Environment env;

    // - add command line
    afl::data::StringList_t list;

    // --- required for server to come up correctly
    list.push_back("-Dmailout.host=MH");
    list.push_back("-Dmailout.port=MP");
    list.push_back("-Dredis.host=DBH");
    list.push_back("-Dredis.port=DBP");

    // --- valid command line options
    list.push_back("--instance=m");
    list.push_back("-Dm.host=TH");
    list.push_back("-Dm.port=TP");
    list.push_back("-Dm.threads=5");
    list.push_back("-Dm.msgid=x@y");
    list.push_back("-Dm.path=a!b");
    list.push_back("-Dm.wwwroot=http://h/");
    list.push_back("-Dm.syntaxdb=/x.txt");
    list.push_back("-Dm.rls.min=0");
    list.push_back("-Dm.rls.max=100");
    list.push_back("-Dm.rls.cooldown=3");
    list.push_back("-Dm.rls.interval=5");
    list.push_back("-Dm.rls.cost.mail=2");
    list.push_back("-Dm.rls.cost.receiver=1");
    list.push_back("-Dm.rls.cost.post=5");
    list.push_back("-Dm.postlsnew.limit=500");
    list.push_back("-Dm.notificationdelay=3");
    env.env.setCommandLine(list);

    // --- file to fulfil the syntaxdb option
    env.fs.openFile("/x.txt", afl::io::FileSystem::Create);

    // Environment
    startMailServer(env, afl::net::Name("MH", "MP"));
    startDatabaseServer(env, afl::net::Name("DBH", "DBP"));

    // Testee
    afl::sys::Thread serverThread("serverThread", env);
    serverThread.start();
    afl::sys::Thread::sleep(100);

    // Stop it
    env.stop();
    serverThread.join();
    a.checkEqual("01. exit", env.exitCode, 0);
}

namespace {
    void testBadOption(afl::test::Assert a, String_t option)
    {
        Environment env;

        afl::data::StringList_t list;
        list.push_back(option);
        env.env.setCommandLine(list);

        env.run();
        a.checkEqual("01. exit", env.exitCode, 1);
    }
}

AFL_TEST("server.talk.ServerApplication:options:bad", a)
{
    testBadOption(a("rls.min"),              "-Dtalk.rls.min=x");
    testBadOption(a("rls.max"),              "-Dtalk.rls.max=x");
    testBadOption(a("rls.cooldown"),         "-Dtalk.rls.cooldown=x");
    testBadOption(a("rls.interval"),         "-Dtalk.rls.interval=x");
    testBadOption(a("rls.cost.mail"),        "-Dtalk.rls.cost.mail=x");
    testBadOption(a("rls.cost.receiver"),    "-Dtalk.rls.cost.receiver=x");
    testBadOption(a("rls.cost.post"),        "-Dtalk.rls.cost.post=x");
    testBadOption(a("rls.postnew.limit"),    "-Dtalk.postlsnew.limit=x");
    testBadOption(a("rls.notificatondelay"), "-Dtalk.notificationdelay=x");
    testBadOption(a("other"),                "-Dother=1");
    testBadOption(a("other option"),         "--other-option");
}
