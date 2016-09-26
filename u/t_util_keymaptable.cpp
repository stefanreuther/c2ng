/**
  *  \file u/t_util_keymaptable.cpp
  *  \brief Test for util::KeymapTable
  */

#include <stdexcept>
#include "util/keymaptable.hpp"

#include "t_util.hpp"

void
TestUtilKeymapTable::testKeymapTable()
{
    // ex IntKeymapTestSuite::testKeymap (part)
    util::KeymapTable testee;

    // Create TESTKEYMAP
    TS_ASSERT_EQUALS(testee.getKeymapByName("TESTKEYMAP"), util::KeymapRef_t(0));
    util::KeymapRef_t a = testee.createKeymap("TESTKEYMAP");
    TS_ASSERT_EQUALS(testee.getKeymapByName("TESTKEYMAP"), a);
    TS_ASSERT_THROWS(testee.createKeymap("TESTKEYMAP"), std::runtime_error);

    // Create TESTCHILD
    util::KeymapRef_t b = testee.createKeymap("TESTCHILD");
    TS_ASSERT_EQUALS(testee.getKeymapByName("TESTCHILD"), b);

    TS_ASSERT_DIFFERS(a, b);
    TS_ASSERT_EQUALS(a->getName(), "TESTKEYMAP");
    TS_ASSERT_EQUALS(b->getName(), "TESTCHILD");
}

