/**
  *  \file test/util/processrunnertest.cpp
  *  \brief Test for util::ProcessRunner
  */

#include "util/processrunner.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("util.ProcessRunner", a)
{
    util::ProcessRunner testee;
#ifdef TARGET_OS_POSIX
    util::ProcessRunner::Command cmd;
    cmd.command.push_back("echo");
    cmd.command.push_back("hi");

    String_t result;
    int n = testee.run(cmd, result);
    a.checkEqual("01. run", n, 0);
    a.checkEqual("02. result", result, "hi\n");
#endif
}
