/**
  *  \file u/t_server_configurationhandler.cpp
  *  \brief Test for server::ConfigurationHandler
  */

#include <stdexcept>
#include <map>
#include "server/configurationhandler.hpp"

#include "t_server.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "afl/string/posixfilenames.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"

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

    /* A FileSystem that provides single files. */
    class TestFileSystem : public afl::io::FileSystem {
     public:
        TestFileSystem()
            : m_fileNames(),
              m_files()
            { }
        virtual afl::base::Ref<afl::io::Stream> openFile(FileName_t fileName, OpenMode mode)
            {
                if (mode != OpenRead) {
                    throw afl::except::FileProblemException(fileName, "open for modify not supported");
                }
                FileMap_t::iterator it = m_files.find(getCanonicalPathName(fileName));
                if (it == m_files.end()) {
                    throw afl::except::FileProblemException(fileName, "not found");
                }
                return it->second->createChild();
            }
        virtual afl::base::Ref<afl::io::Directory> openDirectory(FileName_t dirName)
            { throw afl::except::FileProblemException(dirName, "openDirectory not supported"); }
        virtual afl::base::Ref<afl::io::Directory> openRootDirectory()
            { throw std::runtime_error("openRootDirectory not supported"); }
        virtual bool isAbsolutePathName(FileName_t path)
            { return m_fileNames.isAbsolutePathName(path); }
        virtual bool isPathSeparator(char c)
            { return m_fileNames.isPathSeparator(c); }
        virtual FileName_t makePathName(FileName_t path, FileName_t name)
            { return m_fileNames.makePathName(path, name); }
        virtual FileName_t getCanonicalPathName(FileName_t name)
            { return m_fileNames.getCanonicalPathName(name); }
        virtual FileName_t getAbsolutePathName(FileName_t name)
            { return getCanonicalPathName(makePathName(getWorkingDirectoryName(), name)); }
        virtual FileName_t getFileName(FileName_t name)
            { return m_fileNames.getFileName(name); }
        virtual FileName_t getDirectoryName(FileName_t name)
            { return m_fileNames.getDirectoryName(name); }
        virtual FileName_t getWorkingDirectoryName()
            { return "/"; }

        void addFile(String_t absoluteName, afl::base::Ref<afl::io::Stream> file)
            { m_files[absoluteName] = file.asPtr(); }
     private:
        typedef std::map<String_t, afl::base::Ptr<afl::io::Stream> > FileMap_t;
        afl::string::PosixFileNames m_fileNames;
        FileMap_t m_files;
    };

    /* An Environment that just provides environment variables */
    class TestEnvironment : public afl::sys::Environment {
     public:
        virtual afl::base::Ref<CommandLine_t> getCommandLine()
            { throw std::runtime_error("getCommandLine unsupported"); }
        virtual String_t getInvocationName()
            { return "TestEnvironment"; }
        virtual String_t getEnvironmentVariable(const String_t& name)
            { return m_environment[name]; }
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

        void setEnvironmentVariable(String_t name, String_t value)
            { m_environment[name] = value; }
     private:
        std::map<String_t, String_t> m_environment;
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
        TestCommandLineParser p("public");
        TS_ASSERT(testee.handleCommandLineOption("Dg.public", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 1U);
    }

    // Handle a "-D" option with a recognized secret option
    {
        TestCommandLineParser p("secret");
        TS_ASSERT(testee.handleCommandLineOption("Dg.key", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 2U);
    }

    // Handle another "-D" option with a recognized secret option
    {
        TestCommandLineParser p("secret");
        TS_ASSERT(testee.handleCommandLineOption("DG.OTHER.KEY", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 3U);
    }

    // Handle a "-D" option with a not-recognized option
    {
        TestCommandLineParser p("x");
        TS_ASSERT_THROWS(testee.handleCommandLineOption("Dy", p), std::runtime_error);
    }

    TS_ASSERT_EQUALS(testee.getTotal(), "G.PUBLIC,G.KEY,G.OTHER.KEY");
}

/** Test loading configuration from file. */
void
TestServerConfigurationHandler::testFile()
{
    TestLogListener log;
    TestEnvironment env;
    TestFileSystem fs;
    env.setEnvironmentVariable("C2CONFIG", "/the/file.txt");
    fs.addFile("/the/file.txt", *new afl::io::ConstMemoryStream(afl::string::toBytes("# comment\n"
                                                                                     "g.public = public value\n"
                                                                                     "\n"
                                                                                     "other.thing = whatever\n"
                                                                                     "G.KEY = secret\n")));

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
    TestEnvironment env;
    TestFileSystem fs;

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
    TestEnvironment env;
    TestFileSystem fs;
    env.setEnvironmentVariable("C2CONFIG", "/a.txt");
    fs.addFile("/a.txt", *new afl::io::ConstMemoryStream(afl::string::toBytes("g.public.a=public value\n"
                                                                              "g.override=other\n")));
    fs.addFile("/b.txt", *new afl::io::ConstMemoryStream(afl::string::toBytes("g.public.b=public value\n"
                                                                              "g.override=other\n")));

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
        TestCommandLineParser p("value");
        TS_ASSERT(testee.handleCommandLineOption("Dg.Override", p));
        TS_ASSERT_EQUALS(log.getNumMessages(), 1U);
    }

    // Finally, the file
    testee.loadConfigurationFile(env, fs);

    // Verify
    TS_ASSERT_EQUALS(log.getNumMessages(), 2U);
    TS_ASSERT_EQUALS(testee.getTotal(), "G.OVERRIDE,G.PUBLIC.B");
}

