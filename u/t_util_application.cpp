/**
  *  \file u/t_util_application.cpp
  *  \brief Test for util::Application
  */

#include "util/application.hpp"

#include "t_util.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/string.hpp"
#include "afl/sys/internalenvironment.hpp"

/** Test initialisation with an uncooperative environment.
    The uncooperative throws exceptions instead of attaching channels.
    Application initialisation must succeed anyway. */
void
TestUtilApplication::testInit()
{
    // Environment
    afl::sys::InternalEnvironment env;
    afl::io::NullFileSystem fs;

    // Application descendant
    class Tester : public util::Application {
     public:
        Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
            : Application(env, fs)
            { }
        virtual void appMain()
            {
                // Test all methods. Just verifies that they are callable without error.
                environment();
                fileSystem();
                translator();
                log();
                consoleLogger();
                standardOutput();
                errorOutput();

                // Test that we can write despite uncooperative environment.
                standardOutput().writeLine("hi");
            }
    };
    Tester t(env, fs);

    int n = t.run();
    TS_ASSERT_EQUALS(n, 0);
}

/** Interface test. */
void
TestUtilApplication::testExit()
{
    // Environment
    class FakeEnvironment : public afl::sys::InternalEnvironment {
     public:
        FakeEnvironment()
            : m_stream(*new afl::io::InternalStream())
            {
                setChannelStream(Output, m_stream.asPtr());
                setChannelStream(Error, m_stream.asPtr());
            }
        afl::base::ConstBytes_t getOutput()
            { return m_stream->getContent(); }
     private:
        afl::base::Ref<afl::io::InternalStream> m_stream;
    };

    // Regular exit
    {
        FakeEnvironment env;
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { }
        };

        // Regular exit produces error 0
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 0);

        // We didn't write anything, so output must be empty
        TS_ASSERT_EQUALS(env.getOutput().size(), 0U);
    }

    // Exit with error code
    {
        FakeEnvironment env;
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { exit(42); }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 42);
        TS_ASSERT_EQUALS(env.getOutput().size(), 0U);
    }

    // Exit with exception
    {
        FakeEnvironment env;
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { throw std::runtime_error("hi mom"); }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 1);
        TS_ASSERT_DIFFERS(env.getOutput().size(), 0U);
        TS_ASSERT_DIFFERS(afl::string::fromBytes(env.getOutput()).find("hi mom"), String_t::npos);
    }

    // Exit with nonstandard exception
    {
        FakeEnvironment env;
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { throw "whatever"; }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 1);
        TS_ASSERT_DIFFERS(env.getOutput().size(), 0U);
    }

    // Exit with errorExit
    {
        FakeEnvironment env;
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { errorExit("broken"); }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 1);
        TS_ASSERT_DIFFERS(env.getOutput().size(), 0U);
        TS_ASSERT_DIFFERS(afl::string::fromBytes(env.getOutput()).find("broken"), String_t::npos);
    }

    // Write partial line; must arrive completely.
    {
        FakeEnvironment env;
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { standardOutput().writeText("ok"); }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 0);
        TS_ASSERT_EQUALS(env.getOutput().size(), 2U);
        TS_ASSERT_SAME_DATA(env.getOutput().unsafeData(), "ok", 2);
    }
}

/** Test write error on stdout.
    This emulates problems with standard output, e.g. EPIPE.
    This should not generate a user-face exception, but should properly be reported on stderr. */
void
TestUtilApplication::testWriteError()
{
    // Environment
    class FailStream : public afl::io::Stream {
     public:
        virtual size_t read(Bytes_t /*m*/)
            { throw afl::except::FileProblemException(*this, "read"); }
        virtual size_t write(ConstBytes_t /*m*/)
            { throw afl::except::FileProblemException(*this, "write"); }
        virtual void flush()
            { throw afl::except::FileProblemException(*this, "flush"); }
        virtual void setPos(FileSize_t /*pos*/)
            { throw afl::except::FileProblemException(*this, "setPos"); }
        virtual FileSize_t getPos()
            { return 0; }
        virtual FileSize_t getSize()
            { return 0; }
        virtual uint32_t getCapabilities()
            { return CanWrite; }
        virtual String_t getName()
            { return "FailStream"; }
        virtual afl::base::Ref<afl::io::Stream> createChild()
            { return *this; }
        virtual afl::base::Ptr<afl::io::FileMapping> createFileMapping(FileSize_t /*limit*/)
            { throw afl::except::FileProblemException(*this, "createFileMapping"); }
    };

    // Output fails, error succeeds: must return errorlevel 1 and an error message
    {
        afl::base::Ref<afl::io::InternalStream> err(*new afl::io::InternalStream());
        afl::sys::InternalEnvironment env;
        env.setChannelStream(afl::sys::Environment::Output, new FailStream());
        env.setChannelStream(afl::sys::Environment::Error, err.asPtr());
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { standardOutput().writeLine("hi there"); }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 1);
        TS_ASSERT_DIFFERS(err->getContent().size(), 0U);
        TS_ASSERT_DIFFERS(afl::string::fromBytes(err->getContent()).find("FailStream"), String_t::npos);
    }

    // Output and error fail: must return errorlevel 1
    {
        afl::sys::InternalEnvironment env;
        env.setChannelStream(afl::sys::Environment::Output, new FailStream());
        env.setChannelStream(afl::sys::Environment::Error, new FailStream());
        afl::io::NullFileSystem fs;

        class Tester : public util::Application {
         public:
            Tester(afl::sys::Environment& env, afl::io::FileSystem& fs)
                : Application(env, fs)
                { }
            virtual void appMain()
                { standardOutput().writeLine("hi there"); }
        };
        TS_ASSERT_EQUALS(Tester(env, fs).run(), 1);
    }
}

