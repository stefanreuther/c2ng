/**
  *  \file u/t_server_mailin_mailinapplication.cpp
  *  \brief Test for server::mailin::MailInApplication
  */

#include "server/mailin/mailinapplication.hpp"

#include <map>
#include "t_server_mailin.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/sys/environment.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/thread.hpp"


/** Test obtaining help. */
void
TestServerMailinMailInApplication::testHelp()
{
    // Create environment
    afl::sys::InternalEnvironment env;

    // - add command line
    afl::data::StringList_t list;
    list.push_back("--help");
    env.setCommandLine(list);

    // - add output
    afl::base::Ptr<afl::io::InternalStream> out = new afl::io::InternalStream();
    env.setChannelStream(afl::sys::Environment::Output, out);

    // - null FileSystem
    afl::io::NullFileSystem fs;

    // Testee
    int exit = server::mailin::MailInApplication(env, fs, afl::net::NetworkStack::getInstance()).run();
    TS_ASSERT_EQUALS(exit, 0);

    // Verify result
    TS_ASSERT(out->getContent().size() > 100);
}

/** Test a rejected mail that cannot be saved. */
void
TestServerMailinMailInApplication::testReject()
{
    /*
     *  Networking
     */
    afl::net::NetworkStack& net = afl::net::NetworkStack::getInstance();
    const char* PORT_NR = "15053";

    // A simple server that answers all requests with PONG
    class CommandHandler : public afl::net::CommandHandler {
     public:
        Value_t* call(const Segment_t&)
            { return new afl::data::StringValue("PONG"); }
        void callVoid(const Segment_t&)
            { }
    };
    class ProtocolHandlerFactory : public afl::net::ProtocolHandlerFactory {
     public:
        virtual afl::net::ProtocolHandler* create()
            { return new afl::net::resp::ProtocolHandler(m_commandHandler); }
     private:
        CommandHandler m_commandHandler;
    };

    // Run that on a port
    ProtocolHandlerFactory factory;
    afl::net::Server server(net.listen(afl::net::Name("127.0.0.1", PORT_NR), 5), factory);
    afl::sys::Thread serverThread("TestServerMailinMailInApplication", server);
    serverThread.start();

    /*
     *  Test
     */

    // Create environment
    afl::sys::InternalEnvironment env;

    // - add command line
    afl::data::StringList_t list;
    list.push_back(String_t("-Dhost.port=") + PORT_NR);
    list.push_back(String_t("-Dmailout.port=") + PORT_NR);
    list.push_back("-Dmailin.rejectdir=foo");
    env.setCommandLine(list);

    // - add input
    static const char INPUT[] =
        "Subject: hi\n"
        "From: user@host\n"
        "To: admin@server\n"
        "\n"
        "witty text here.\n"
        "\n";
    env.setChannelStream(afl::sys::Environment::Input, new afl::io::ConstMemoryStream(afl::string::toBytes(INPUT)));

    // - capture output
    afl::base::Ptr<afl::io::InternalStream> out = new afl::io::InternalStream();
    env.setChannelStream(afl::sys::Environment::Output, out);
    env.setChannelStream(afl::sys::Environment::Error, out);

    // - null FileSystem
    afl::io::NullFileSystem fs;

    // Testee
    int exit = server::mailin::MailInApplication(env, fs, afl::net::NetworkStack::getInstance()).run();
    TS_ASSERT_EQUALS(exit, 1);

    // Verify required content
    String_t result = afl::string::fromBytes(out->getContent());
    TS_ASSERT_DIFFERS(result.find("no usable content"), String_t::npos);
    TS_ASSERT_DIFFERS(result.find("[error] writing file"), String_t::npos);

    // Stop
    server.stop();
    serverThread.join();
}

