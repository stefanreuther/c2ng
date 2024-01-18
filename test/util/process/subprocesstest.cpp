/**
  *  \file test/util/process/subprocesstest.cpp
  *  \brief Test for util::process::Subprocess
  */

#include "util/process/subprocess.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("util.process.Subprocess")
{
    class Tester : public util::process::Subprocess {
     public:
        virtual bool isActive() const
            { return false; }
        virtual uint32_t getProcessId() const
            { return 0; }
        virtual bool start(const String_t& /*path*/, afl::base::Memory<const String_t> /*args*/)
            { return false; }
        virtual bool stop()
            { return false; }
        virtual bool writeLine(const String_t& /*line*/)
            { return false; }
        virtual bool readLine(String_t& /*result*/)
            { return false; }
        virtual String_t getStatus() const
            { return String_t(); }
    };
    Tester t;
}
