/**
  *  \file u/t_interpreter_vmio_nullloadcontext.cpp
  *  \brief Test for interpreter::vmio::NullLoadContext
  */

#include "interpreter/vmio/nullloadcontext.hpp"

#include "t_interpreter_vmio.hpp"
#include "afl/io/nullstream.hpp"

/** Test NullLoadContext. */
void
TestInterpreterVmioNullLoadContext::testIt()
{
    interpreter::vmio::NullLoadContext testee;

    // Data
    TS_ASSERT(testee.loadBCO(0) == 0);
    TS_ASSERT(testee.loadBCO(99) == 0);
    TS_ASSERT(testee.loadArray(0) == 0);
    TS_ASSERT(testee.loadArray(99) == 0);
    TS_ASSERT(testee.loadHash(0) == 0);
    TS_ASSERT(testee.loadHash(99) == 0);
    TS_ASSERT(testee.loadStructureValue(0) == 0);
    TS_ASSERT(testee.loadStructureValue(99) == 0);
    TS_ASSERT(testee.loadStructureType(0) == 0);
    TS_ASSERT(testee.loadStructureType(99) == 0);

    // Processes
    {
        interpreter::TagNode node;
        afl::io::NullStream in;
        node.tag = node.Tag_Ship;
        node.value = 42;
        TS_ASSERT(testee.loadContext(node, in) == 0);
    }
    TS_ASSERT(testee.loadMutex("foo", "bar", 0) == 0);
    TS_ASSERT(testee.createProcess() == 0);
}

