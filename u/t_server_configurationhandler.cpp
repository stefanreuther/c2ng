/**
  *  \file u/t_server_configurationhandler.cpp
  *  \brief Test for server::ConfigurationHandler
  */

#include <stdexcept>
#include <map>
#include "server/configurationhandler.hpp"

#include "t_server.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/loglistener.hpp"

namespace {
    const char LOG_NAME[] = "test.log";

    /* A LogListener that counts messages and validates that
       - censoring works
       - the correct channel is used */
    class TestLogListener : public afl::sys::LogListener {
     public:
        TestLogListener()
            : m_numMessages(0)
            { }
        virtual void handleMessage(const Message& msg)
            {
                ++m_numMessages;
                TS_ASSERT_EQUALS(msg.m_channel, LOG_NAME);
                TS_ASSERT_EQUALS(msg.m_message.find("secret"), String_t::npos);
            }
        size_t getNumMessages() const
            { return m_numMessages; }
     private:
        size_t m_numMessages;
    };

    /* A ConfigurationHandler that accepts all options starting with "g",
       and collects them in a list. */
    class TestConfigHandler : public server::ConfigurationHandler {
     public:
        TestConfigHandler(TestLogListener& log)
            : ConfigurationHandler(log, LOG_NAME),
              m_total()
            { }
        virtual bool handleConfiguration(const String_t& key, const String_t& /*value*/)
            {
                if (key.substr(0, 1) == "G") {
                    if (!m_total.empty()) {
                        m_total += ",";
                    }
                    m_total += key;
                    return true;
                } else {
                    return false;
                }
            }
        String_t getTotal() const
            { return m_total; }
     private:
        String_t m_total;
    };

    /* A CommandLineParser that supplies a hardcoded command-line option. */
    class TestCommandLineParser : public afl::sys::CommandLineParser {
     public:
        TestCommandLineParser(String_t value)
            : m_value(value)
            { }
        virtual bool getNext(bool& /*option*/, String_t& /*text*/)
            {
                TS_FAIL("getNext unexpected");
                return false;
            }
        virtual bool getParameter(String_t& value)
            {
                value = m_value;
                return true;
            }
        virtual Flags_t getFlags()
            { return Flags_t(); }
     private:
        String_t m_value;
    };
}


/** Test command-line options. */
void
TestServerConfigurationHandler::testCommandLine()
{
    TestLogListener log;
    TestConfigHandler testee(log);

    TS_ASSERT_EQUALS(log.getNumMessages(), 0U);

    // Handle a nonexistant option
    {
        TestCommandLineParser p("");
        TS_ASSERT(!testee.handleCommandLineOption("x", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 0U);
    }

    // Handle a "-D" option with a recognized option
    {
        TestCommandLineParser p("g.public=public");
        TS_ASSERT(testee.handleCommandLineOption("D", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 1U);
    }

    // Handle a "-D" option with a recognized secret option
    {
        TestCommandLineParser p("g.key=secret");
        TS_ASSERT(testee.handleCommandLineOption("D", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 2U);
    }

    // Handle another "-D" option with a recognized secret option
    {
        TestCommandLineParser p("G.OTHER.KEY=secret");
        TS_ASSERT(testee.handleCommandLineOption("D", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 3U);
    }

    // Handle a "-D" option with a not-recognized option
    {
        TestCommandLineParser p("y=x");
        TS_ASSERT_THROWS(testee.handleCommandLineOption("D", p), std::runtime_error);
    }

    TS_ASSERT_EQUALS(testee.getTotal(), "G.PUBLIC,G.KEY,G.OTHER.KEY");
}

/** Test loading configuration from file. */
void
TestServerConfigurationHandler::testFile()
{
    TestLogListener log;
    afl::sys::InternalEnvironment env;
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/the");
    fs.openFile("/the/file.txt", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("# comment\n"
                                                                                              "g.public = public value\n"
                                                                                              "\n"
                                                                                              "other.thing = whatever\n"
                                                                                              "G.KEY = secret\n"));
    env.setEnvironmentVariable("C2CONFIG", "/the/file.txt");

    // Test
    TestConfigHandler testee(log);
    TS_ASSERT_EQUALS(log.getNumMessages(), 0U);
    testee.loadConfigurationFile(env, fs);

    // Verify
    TS_ASSERT_EQUALS(log.getNumMessages(), 2U);               // 2 values logged
    TS_ASSERT_EQUALS(testee.getTotal(), "G.PUBLIC,G.KEY");
}

/** Test loading configuration from file, file does not exist. */
void
TestServerConfigurationHandler::testFileOverride()
{
    TestLogListener log;
    afl::sys::InternalEnvironment env;
    afl::io::InternalFileSystem fs;

    // Test
    TestConfigHandler testee(log);
    TS_ASSERT_EQUALS(log.getNumMessages(), 0U);
    testee.loadConfigurationFile(env, fs);

    // Verify
    TS_ASSERT_EQUALS(log.getNumMessages(), 1U);               // message reporting use of defaults
}

/** Test loading configuration from file, with command-line override. */
void
TestServerConfigurationHandler::testNoFile()
{
    TestLogListener log;
    afl::sys::InternalEnvironment env;
    afl::io::InternalFileSystem fs;
    env.setEnvironmentVariable("C2CONFIG", "/a.txt");
    fs.openFile("/a.txt", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("g.public.a=public value\n"
                                                                                       "g.override=other\n"));
    fs.openFile("/b.txt", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("g.public.b=public value\n"
                                                                                       "g.override=other\n"));

    // Test
    TestConfigHandler testee(log);
    TS_ASSERT_EQUALS(log.getNumMessages(), 0U);

    // Handle a "--config" option
    {
        TestCommandLineParser p("/b.txt");
        TS_ASSERT(testee.handleCommandLineOption("config", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 0U);
    }

    // Handle a "-D" option
    {
        TestCommandLineParser p("g.Override=value");
        TS_ASSERT(testee.handleCommandLineOption("D", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 1U);
    }

    // Finally, the file
    testee.loadConfigurationFile(env, fs);

    // Verify
    TS_ASSERT_EQUALS(log.getNumMessages(), 2U);
    TS_ASSERT_EQUALS(testee.getTotal(), "G.OVERRIDE,G.PUBLIC.B");
}

