/**
  *  \file u/t_util_process_subprocess.cpp
  *  \brief Test for util::process::Subprocess
  */

#include "util/process/subprocess.hpp"

#include "t_util_process.hpp"

/** Interface test. */
void
TestUtilProcessSubprocess::testInterface()
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

