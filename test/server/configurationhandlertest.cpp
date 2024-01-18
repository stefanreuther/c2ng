/**
  *  \file test/server/configurationhandlertest.cpp
  *  \brief Test for server::ConfigurationHandler
  */

#include "server/configurationhandler.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/test/testrunner.hpp"
#include <map>
#include <stdexcept>

namespace {
    const char LOG_NAME[] = "test.log";

    /* A LogListener that counts messages and validates that
       - censoring works
       - the correct channel is used */
    class TestLogListener : public afl::sys::LogListener {
     public:
        TestLogListener(afl::test::Assert a)
            : m_assert(a), m_numMessages(0)
            { }
        virtual void handleMessage(const Message& msg)
            {
                ++m_numMessages;
                m_assert.checkEqual("01. channel", msg.m_channel, LOG_NAME);
                m_assert.checkEqual("02. message", msg.m_message.find("secret"), String_t::npos);
            }
        size_t getNumMessages() const
            { return m_numMessages; }
     private:
        afl::test::Assert m_assert;
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
                throw std::runtime_error("getNext unexpected");
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
AFL_TEST("server.ConfigurationHandler:command-line", a)
{
    TestLogListener log(a);
    TestConfigHandler testee(log);

    a.checkEqual("01. getNumMessages", log.getNumMessages(), 0U);

    // Handle a nonexistant option
    {
        TestCommandLineParser p("");
        a.check("11. handleCommandLineOption", !testee.handleCommandLineOption("x", p));
        a.checkEqual("12. getNumMessages", log.getNumMessages(), 0U);
    }

    // Handle a "-D" option with a recognized option
    {
        TestCommandLineParser p("g.public=public");
        a.check("21. handleCommandLineOption", testee.handleCommandLineOption("D", p));
        a.checkEqual("22. getNumMessages", log.getNumMessages(), 1U);
    }

    // Handle a "-D" option with a recognized secret option
    {
        TestCommandLineParser p("g.key=secret");
        a.check("31. handleCommandLineOption", testee.handleCommandLineOption("D", p));
        a.checkEqual("32. getNumMessages", log.getNumMessages(), 2U);
    }

    // Handle another "-D" option with a recognized secret option
    {
        TestCommandLineParser p("G.OTHER.KEY=secret");
        a.check("41. handleCommandLineOption", testee.handleCommandLineOption("D", p));
        a.checkEqual("42. getNumMessages", log.getNumMessages(), 3U);
    }

    // Handle a "-D" option with a not-recognized option
    {
        TestCommandLineParser p("y=x");
        AFL_CHECK_THROWS(a("51. handleCommandLineOption"), testee.handleCommandLineOption("D", p), std::runtime_error);
    }

    a.checkEqual("61. getTotal", testee.getTotal(), "G.PUBLIC,G.KEY,G.OTHER.KEY");
}

/** Test loading configuration from file. */
AFL_TEST("server.ConfigurationHandler:loadConfigurationFile", a)
{
    TestLogListener log(a);
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
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 0U);
    testee.loadConfigurationFile(env, fs);

    // Verify
    a.checkEqual("11. getNumMessages", log.getNumMessages(), 2U);               // 2 values logged
    a.checkEqual("12. getTotal", testee.getTotal(), "G.PUBLIC,G.KEY");
}

/** Test loading configuration from file, file does not exist. */
AFL_TEST("server.ConfigurationHandler:loadConfigurationFile:missing-file", a)
{
    TestLogListener log(a);
    afl::sys::InternalEnvironment env;
    afl::io::InternalFileSystem fs;

    // Test
    TestConfigHandler testee(log);
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 0U);
    testee.loadConfigurationFile(env, fs);

    // Verify
    a.checkEqual("11. getNumMessages", log.getNumMessages(), 1U);               // message reporting use of defaults
}

/** Test loading configuration from file, with command-line override. */
AFL_TEST("server.ConfigurationHandler:loadConfigurationFile:command-line", a)
{
    TestLogListener log(a);
    afl::sys::InternalEnvironment env;
    afl::io::InternalFileSystem fs;
    env.setEnvironmentVariable("C2CONFIG", "/a.txt");
    fs.openFile("/a.txt", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("g.public.a=public value\n"
                                                                                       "g.override=other\n"));
    fs.openFile("/b.txt", afl::io::FileSystem::Create)->fullWrite(afl::string::toBytes("g.public.b=public value\n"
                                                                                       "g.override=other\n"));

    // Test
    TestConfigHandler testee(log);
    a.checkEqual("01. getNumMessages", log.getNumMessages(), 0U);

    // Handle a "--config" option
    {
        TestCommandLineParser p("/b.txt");
        a.check("11. handleCommandLineOption", testee.handleCommandLineOption("config", p));
        a.checkEqual("12. getNumMessages", log.getNumMessages(), 0U);
    }

    // Handle a "-D" option
    {
        TestCommandLineParser p("g.Override=value");
        a.check("21. handleCommandLineOption", testee.handleCommandLineOption("D", p));
        a.checkEqual("22. getNumMessages", log.getNumMessages(), 1U);
    }

    // Finally, the file
    testee.loadConfigurationFile(env, fs);

    // Verify
    a.checkEqual("31. getNumMessages", log.getNumMessages(), 2U);
    a.checkEqual("32. getTotal", testee.getTotal(), "G.OVERRIDE,G.PUBLIC.B");
}
