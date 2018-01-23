/**
  *  \file u/t_util_processrunner.cpp
  *  \brief Test for util::ProcessRunner
  */

#include "util/processrunner.hpp"

#include "t_util.hpp"

void
TestUtilProcessRunner::testIt()
{
    util::ProcessRunner testee;
#ifdef TARGET_OS_POSIX
    util::ProcessRunner::Command cmd;
    cmd.command.push_back("echo");
    cmd.command.push_back("hi");

    String_t result;
    int n = testee.run(cmd, result);
    TS_ASSERT_EQUALS(n, 0);
    TS_ASSERT_EQUALS(result, "hi\n");
#endif
}

