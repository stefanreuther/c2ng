/**
  *  \file test/util/keymaptabletest.cpp
  *  \brief Test for util::KeyMapTable
  */

#include "util/keymaptable.hpp"

#include "afl/test/testrunner.hpp"
#include <stdexcept>

AFL_TEST("util.KeyMapTable", a)
{
    // ex IntKeymapTestSuite::testKeymap (part)
    util::KeymapTable testee;
    a.checkEqual("01. getNumKeymaps",    testee.getNumKeymaps(), 0U);
    a.checkEqual("02. getKeymapByIndex", testee.getKeymapByIndex(0), util::KeymapRef_t(0));

    // Create TESTKEYMAP
    a.checkEqual("11. getKeymapByName", testee.getKeymapByName("TESTKEYMAP"), util::KeymapRef_t(0));
    util::KeymapRef_t ra = testee.createKeymap("TESTKEYMAP");
    a.checkEqual("12. getKeymapByName", testee.getKeymapByName("TESTKEYMAP"), ra);
    AFL_CHECK_THROWS(a("13. createKeymap"), testee.createKeymap("TESTKEYMAP"), std::runtime_error);

    a.checkEqual("21. getNumKeymaps", testee.getNumKeymaps(), 1U);
    a.checkEqual("22. getKeymapByIndex", testee.getKeymapByIndex(0), ra);

    // Create TESTCHILD
    util::KeymapRef_t rb = testee.createKeymap("TESTCHILD");
    a.checkEqual("31. getKeymapByName", testee.getKeymapByName("TESTCHILD"), rb);

    a.check("41. different keymaps", ra != rb);
    a.checkEqual("42. getName", ra->getName(), "TESTKEYMAP");
    a.checkEqual("43. getName", rb->getName(), "TESTCHILD");
}
