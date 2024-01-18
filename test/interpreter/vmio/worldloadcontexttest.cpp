/**
  *  \file test/interpreter/vmio/worldloadcontexttest.cpp
  *  \brief Test for interpreter::vmio::WorldLoadContext
  */

#include "interpreter/vmio/worldloadcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/vmio/nullloadcontext.hpp"
#include "interpreter/world.hpp"

AFL_TEST("interpreter.vmio.WorldLoadContext", a)
{
    // A test that actually loads a process from a VM file is in TestInterpreterVmioObjectLoader::testLoadProcess().
    // Therefore, just a plain coverage/continuity test.
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::ProcessList list;
    interpreter::vmio::NullLoadContext ctx;
    interpreter::vmio::WorldLoadContext testee(ctx, list, world);

    a.checkEqual("01. loadBCO",            testee.loadBCO(1),            ctx.loadBCO(1));
    a.checkEqual("02. loadArray",          testee.loadArray(1),          ctx.loadArray(1));
    a.checkEqual("03. loadHash",           testee.loadHash(1),           ctx.loadHash(1));
    a.checkEqual("04. loadStructureValue", testee.loadStructureValue(1), ctx.loadStructureValue(1));
    a.checkEqual("05. loadStructureType",  testee.loadStructureType(1),  ctx.loadStructureType(1));
 }
