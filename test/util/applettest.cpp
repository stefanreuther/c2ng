/**
  *  \file test/util/applettest.cpp
  *  \brief Test for util::Applet
  */

#include "util/applet.hpp"

#include "afl/io/internalfilesystem.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

using afl::data::StringList_t;
using afl::io::InternalFileSystem;
using afl::io::InternalStream;
using afl::sys::InternalEnvironment;
using util::Applet;

namespace {
    class FakeEnvironment : public InternalEnvironment {
     public:
        FakeEnvironment()
            : m_stream(*new InternalStream())
            {
                setChannelStream(Output, m_stream.asPtr());
                setChannelStream(Error, m_stream.asPtr());
            }
        afl::base::ConstBytes_t getOutput()
            { return m_stream->getContent(); }
     private:
        afl::base::Ref<InternalStream> m_stream;
    };

    class TestApplet : public util::Applet {
     public:
        TestApplet(String_t name, int exit)
            : m_name(name), m_exit(exit)
            { }
        virtual int run(util::Application& app, afl::sys::Environment::CommandLine_t& cmdl)
            {
                app.standardOutput().writeLine(m_name);
                String_t it;
                while (cmdl.getNextElement(it)) {
                    app.standardOutput().writeLine(it);
                }
                return m_exit;
            }
     private:
        String_t m_name;
        int m_exit;
    };
}

/** Test help. */
AFL_TEST("util.Applet.Runner:help", a)
{
    InternalFileSystem fs;
    FakeEnvironment env;
    StringList_t args;
    args.push_back("--help");
    env.setCommandLine(args);

    int exit = Applet::Runner("runner-name", env, fs)
        .addNew("applet-one", "Info 1", new TestApplet("marker-one", 1))
        .addNew("applet-two", "Info 2", new TestApplet("marker-two", 2))
        .run();

    a.checkEqual("01. exit", exit, 0);

    String_t output = afl::string::fromBytes(env.getOutput());
    a.checkContains("11. name", output, "runner-name");
    a.checkContains("12. name", output, "applet-one");
    a.checkContains("13. name", output, "applet-two");
    a.checkContains("14. info", output, "Info 1");
    a.checkContains("15. info", output, "Info 2");
}

/** Test execution of an applet. */
AFL_TEST("util.Applet.Runner:run", a)
{
    InternalFileSystem fs;
    FakeEnvironment env;
    StringList_t args;
    args.push_back("applet-two");
    args.push_back("arg1");
    args.push_back("arg2");
    env.setCommandLine(args);

    int exit = Applet::Runner("runner-name", env, fs)
        .addNew("applet-one", "Info 1", new TestApplet("marker-one", 41))
        .addNew("applet-two", "Info 2", new TestApplet("marker-two", 42))
        .run();

    a.checkEqual("01. exit", exit, 42);

    String_t output = util::normalizeLinefeeds(env.getOutput());
    a.checkEqual("11. output", output, "marker-two\narg1\narg2\n");
}

/** Test execution error: bad name. */
AFL_TEST("util.Applet.Runner:run:bad-name", a)
{
    InternalFileSystem fs;
    FakeEnvironment env;
    StringList_t args;
    args.push_back("applet-three");
    env.setCommandLine(args);

    int exit = Applet::Runner("runner-name", env, fs)
        .addNew("applet-one", "Info 1", new TestApplet("marker-one", 41))
        .addNew("applet-two", "Info 2", new TestApplet("marker-two", 42))
        .run();

    a.checkEqual("01. exit", exit, 1);

    String_t output = afl::string::fromBytes(env.getOutput());
    a.checkDifferent("11. output", output, "");
}

/** Test execution error: no name. */
AFL_TEST("util.Applet.Runner:run:no-name", a)
{
    InternalFileSystem fs;
    FakeEnvironment env;
    StringList_t args;

    int exit = Applet::Runner("runner-name", env, fs)
        .addNew("applet-one", "Info 1", new TestApplet("marker-one", 41))
        .addNew("applet-two", "Info 2", new TestApplet("marker-two", 42))
        .run();

    a.checkEqual("01. exit", exit, 1);

    String_t output = afl::string::fromBytes(env.getOutput());
    a.checkDifferent("11. output", output, "");
}
