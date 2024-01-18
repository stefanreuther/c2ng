/**
  *  \file test/client/si/requestlink2test.cpp
  *  \brief Test for client::si::RequestLink2
  */

#include "client/si/requestlink2.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

AFL_TEST("client.si.RequestLink2:create:no-result", a)
{
    client::si::RequestLink2 testee(125, false);
    uint32_t pid;
    a.check("01. getProcessId", testee.getProcessId(pid));
    a.checkEqual("02. pid", pid, 125U);
    a.checkEqual("03. isWantResult", testee.isWantResult(), false);
}

AFL_TEST("client.si.RequestLink2:create:want-result", a)
{
    client::si::RequestLink2 testee(999999999, true);
    uint32_t pid;
    a.check("01. getProcessId", testee.getProcessId(pid));
    a.checkEqual("02. pid", pid, 999999999U);
    a.checkEqual("03. isWantResult", testee.isWantResult(), true);
}

AFL_TEST("client.si.RequestLink2:create:default", a)
{
    client::si::RequestLink2 testee;
    uint32_t pid;
    a.check("getProcessId", !testee.getProcessId(pid));
}

/** Test conversion from RequestLink1. */
AFL_TEST("client.si.RequestLink2:convert", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World w(log, tx, fs);
    interpreter::Process p(w, "TestClientSiRequestLink2", 99);
    client::si::RequestLink1 t(p, true);

    client::si::RequestLink2 testee(t);
    uint32_t pid;
    a.check("01. getProcessId", testee.getProcessId(pid));
    a.checkEqual("02. pid", pid, 99U);
    a.checkEqual("03. isWantResult", testee.isWantResult(), true);
}
