/**
  *  \file test/client/si/requestlink1test.cpp
  *  \brief Test for client::si::RequestLink1
  */

#include "client/si/requestlink1.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

AFL_TEST("client.si.RequestLink1", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World w(log, tx, fs);
    interpreter::Process p(w, "TestClientSiRequestLink1", 99);
    client::si::RequestLink1 testee(p, true);

    a.checkEqual("01. getProcess", &testee.getProcess(), &p);
    a.check("02. isWantResult", testee.isWantResult());
}
