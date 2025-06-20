/**
  *  \file test/util/processrunnerapplettest.cpp
  *  \brief Test for util::ProcessRunnerApplet
  */

#include "util/processrunnerapplet.hpp"

#include "afl/io/internalstream.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/sys/internalenvironment.hpp"
#include "afl/test/testrunner.hpp"
#include "util/io.hpp"

using afl::io::InternalStream;
using afl::io::NullFileSystem;
using afl::sys::Environment;
using afl::sys::InternalEnvironment;

#ifdef TARGET_OS_POSIX
AFL_TEST("util.ProcessRunnerApplet", a)
{
    NullFileSystem fs;
    InternalEnvironment env;
    afl::base::Ptr<InternalStream> out = new InternalStream();
    env.setChannelStream(Environment::Output, out);
    env.setChannelStream(Environment::Error, out);

    afl::data::StringList_t args;
    args.push_back("app");
    args.push_back("echo");
    args.push_back("hi");
    env.setCommandLine(args);

    int n = util::Applet::Runner("", env, fs)
        .addNew("app", "", new util::ProcessRunnerApplet())
        .run();

    String_t result = util::normalizeLinefeeds(out->getContent());
    a.checkEqual("01. run", n, 0);
    a.checkEqual("02. result", result, "Output: <<hi\n>>\nExit code: 0\n");
}
#endif
