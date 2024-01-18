/**
  *  \file test/interpreter/vmio/nullloadcontexttest.cpp
  *  \brief Test for interpreter::vmio::NullLoadContext
  */

#include "interpreter/vmio/nullloadcontext.hpp"

#include "afl/io/nullstream.hpp"
#include "afl/test/testrunner.hpp"

/** Test NullLoadContext. */
AFL_TEST("interpreter.vmio.NullLoadContext", a)
{
    interpreter::vmio::NullLoadContext testee;

    // Data
    a.checkNull("01. loadBCO",            testee.loadBCO(0));
    a.checkNull("02. loadBCO",            testee.loadBCO(99));
    a.checkNull("03. loadArray",          testee.loadArray(0));
    a.checkNull("04. loadArray",          testee.loadArray(99));
    a.checkNull("05. loadHash",           testee.loadHash(0));
    a.checkNull("06. loadHash",           testee.loadHash(99));
    a.checkNull("07. loadStructureValue", testee.loadStructureValue(0));
    a.checkNull("08. loadStructureValue", testee.loadStructureValue(99));
    a.checkNull("09. loadStructureType",  testee.loadStructureType(0));
    a.checkNull("10. loadStructureType",  testee.loadStructureType(99));

    // Processes
    {
        interpreter::TagNode node;
        afl::io::NullStream in;
        node.tag = node.Tag_Ship;
        node.value = 42;
        a.checkNull("11. loadContext", testee.loadContext(node, in));
    }
    a.checkNull("12. loadMutex", testee.loadMutex("foo", "bar"));
    a.checkNull("13. createProcess", testee.createProcess());
}
