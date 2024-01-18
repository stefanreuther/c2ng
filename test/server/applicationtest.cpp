/**
  *  \file test/server/applicationtest.cpp
  *  \brief Test for server::Application
  */

#include "server/application.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/net/nullnetworkstack.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"

/** Test simple application. */
AFL_TEST("server.Application", a)
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
    afl::sys::InternalEnvironment env;
    afl::io::NullFileSystem fs;
    afl::net::NullNetworkStack net;
    Tester t("test", env, fs, net);

    // Verify accessors
    a.checkEqual("01. fileSystem", &t.fileSystem(), &fs);
    a.checkEqual("02. networkStack", &t.networkStack(), &net);

    // Run and verify result
    a.checkEqual("11. run", t.run(), 99);
}
