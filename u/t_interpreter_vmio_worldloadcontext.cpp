/**
  *  \file u/t_interpreter_vmio_worldloadcontext.cpp
  *  \brief Test for interpreter::vmio::WorldLoadContext
  */

#include "interpreter/vmio/worldloadcontext.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/vmio/nullloadcontext.hpp"
#include "interpreter/world.hpp"

void
TestInterpreterVmioWorldLoadContext::testIt()
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

    TS_ASSERT_EQUALS(testee.loadBCO(1),            ctx.loadBCO(1));
    TS_ASSERT_EQUALS(testee.loadArray(1),          ctx.loadArray(1));
    TS_ASSERT_EQUALS(testee.loadHash(1),           ctx.loadHash(1));
    TS_ASSERT_EQUALS(testee.loadStructureValue(1), ctx.loadStructureValue(1));
    TS_ASSERT_EQUALS(testee.loadStructureType(1),  ctx.loadStructureType(1));
    TS_ASSERT_EQUALS(testee.loadBCO(1),            ctx.loadBCO(1));
}

