/**
  *  \file u/t_server_mailin_mailinapplication.cpp
  *  \brief Test for server::mailin::MailInApplication
  */

#include "server/mailin/mailinapplication.hpp"

#include <map>
#include "t_server_mailin.hpp"
#include "afl/data/stringlist.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/io/textfile.hpp"
#include "afl/sys/environment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/net/resp/protocolhandler.hpp"
#include "afl/net/server.hpp"
#include "afl/net/protocolhandlerfactory.hpp"
#include "afl/sys/thread.hpp"

namespace {
    /*
     *  As of 20171001, this is a pretty messy implementation of an Environment mock
     *  to show what needs to be done to mock an environment.
     */
    class EnvironmentMock : public afl::sys::Environment {
     public:
        EnvironmentMock()
            : m_in(*new afl::io::ConstMemoryStream(afl::base::Nothing)),
              m_out(*new afl::io::InternalStream()),
              m_error(m_out),
              m_invocationName("mock"),
              m_installationDirectory("."),
              m_commandLine(),
              m_environment()
            { }

        virtual afl::base::Ref<CommandLine_t> getCommandLine()
            { return *new StringListIterator(m_commandLine); }
        virtual String_t getInvocationName()
            { return m_invocationName; }
        virtual String_t getEnvironmentVariable(const String_t& name)
            { return m_environment[name]; }
        virtual String_t getSettingsDirectoryName(const String_t& /*appName*/)
            { return "."; }
        virtual String_t getInstallationDirectoryName()
            { return m_installationDirectory; }
        virtual afl::string::LanguageCode getUserLanguage()
            { return afl::string::LanguageCode(); }
        virtual afl::base::Ref<afl::io::TextWriter> attachTextWriter(Channel ch)
            { return *new afl::io::TextFile(*attachStream(ch)); }
        virtual afl::base::Ref<afl::io::TextReader> attachTextReader(Channel ch)
            { return *new afl::io::TextFile(*attachStream(ch)); }
        virtual afl::base::Ref<afl::io::Stream> attachStream(Channel ch)
            {
                switch (ch) {
                 case Input: return m_in;
                 case Output: return m_out;
                 case Error: return m_error;
                }
                throw std::runtime_error("fail");
            }

        void setInvocationName(String_t name)
            { m_invocationName = name; }
        void setInstallationDirectoryName(String_t dir)
            { m_installationDirectory = dir; }
        void setCommandLine(const afl::data::StringList_t& cmdl)
            { m_commandLine = cmdl; }
        void setChannelStream(Channel ch, afl::base::Ref<afl::io::Stream> s)
            {
                switch (ch) {
                 case Input: m_in.reset(*s); break;
                 case Output: m_out.reset(*s); break;
                 case Error: m_error.reset(*s); break;
                }
            }

     private:
        afl::base::Ref<afl::io::Stream> m_in;
        afl::base::Ref<afl::io::Stream> m_out;
        afl::base::Ref<afl::io::Stream> m_error;

        String_t m_invocationName;
        String_t m_installationDirectory;

        afl::data::StringList_t m_commandLine;

        std::map<String_t, String_t> m_environment;

        class StringListIterator : public CommandLine_t {
         public:
            StringListIterator(afl::base::Memory<const String_t> data)
                : m_data(data)
                { }
            bool getNextElement(String_t& ele)
                {
                    if (const String_t* p = m_data.eat()) {
                        ele = *p;
                        return true;
                    } else {
                        return false;
                    }
                }
         private:
            afl::base::Memory<const String_t> m_data;
        };
    };
}


/** Test obtaining help. */
void
TestServerMailinMailInApplication::testHelp()
{
    // Create environment
    EnvironmentMock env;

    // - add command line
    afl::data::StringList_t list;
    list.push_back("--help");
    env.setCommandLine(list);

    // - add output
    afl::base::Ref<afl::io::InternalStream> out = *new afl::io::InternalStream();
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
    EnvironmentMock env;

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
    env.setChannelStream(afl::sys::Environment::Input, *new afl::io::ConstMemoryStream(afl::string::toBytes(INPUT)));

    // - capture output
    afl::base::Ref<afl::io::InternalStream> out = *new afl::io::InternalStream();
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

