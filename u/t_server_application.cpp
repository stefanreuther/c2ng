/**
  *  \file u/t_server_application.cpp
  *  \brief Test for server::Application
  */

#include "server/application.hpp"

#include "t_server.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullnetworkstack.hpp"

namespace {
    class NullCommandLine : public afl::base::Enumerator<String_t> {
     public:
        bool getNextElement(String_t&)
            { return false; }
    };
    class TestEnvironment : public afl::sys::Environment {
     public:
        virtual afl::base::Ref<CommandLine_t> getCommandLine()
            { return *new NullCommandLine(); }
        virtual String_t getInvocationName()
            { return "TestEnvironment"; }
        virtual String_t getEnvironmentVariable(const String_t& /*name*/)
            { return String_t(); }
        virtual String_t getSettingsDirectoryName(const String_t& /*appName*/)
            { return "/settings"; }
        virtual String_t getInstallationDirectoryName()
            { return "/install"; }
        virtual afl::base::Ref<afl::io::TextWriter> attachTextWriter(Channel /*ch*/)
            { throw std::runtime_error("attachTextWriter unsupported"); }
        virtual afl::base::Ref<afl::io::TextReader> attachTextReader(Channel /*ch*/)
            { throw std::runtime_error("attachTextReader unsupported"); }
        virtual afl::base::Ref<afl::io::Stream> attachStream(Channel /*ch*/)
            { throw std::runtime_error("attachStream unsupported"); }
    };
}

/** Test simple application. */
void
TestServerApplication::testSimple()
{
    // The application:
    class Tester : public server::Application {
     public:
        Tester(const String_t& logName, afl::sys::Environment& env, afl::io::FileSystem& fs, afl::net::NetworkStack& net)
            : Application(logName, env, fs, net)
            { }
        virtual void serverMain()
            { exit(99); }
        virtual bool handleConfiguration(const String_t& /*key*/, const String_t& /*value*/)
            { return false; }
        virtual bool handleCommandLineOption(const String_t& /*option*/, afl::sys::CommandLineParser& /*parser*/)
            { return false; }
        virtual String_t getApplicationName() const
            { return String_t(); }
        virtual String_t getCommandLineOptionHelp() const
            { return String_t(); }
    };

    // Environment and instantiation:
    TestEnvironment env;
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    Tester t("test", env, fs, net);

    // Verify accessors
    TS_ASSERT_EQUALS(&t.fileSystem(), &fs);
    TS_ASSERT_EQUALS(&t.networkStack(), &net);

    // Run and verify result
    TS_ASSERT_EQUALS(t.run(), 99);
}

