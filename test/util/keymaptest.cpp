/**
  *  \file test/util/keymaptest.cpp
  *  \brief Test for util::KeyMap
  */

#include "util/keymap.hpp"

#include "afl/test/testrunner.hpp"
#include "util/keymapinformation.hpp"

/** General keymap test. */
AFL_TEST("util.KeyMap:basics", a)
{
    // ex IntKeymapTestSuite::testKeymap (part)
    util::Keymap ka("TESTKEYMAP");
    util::Keymap kb("TESTCHILD");

    // Check parents
    a.check("01. hasParent", !ka.hasParent(kb));
    a.check("02. hasParent", !kb.hasParent(ka));
    a.check("03. hasParent", ka.hasParent(ka));
    a.check("04. hasParent", kb.hasParent(kb));
    AFL_CHECK_SUCCEEDS(a("05. addParent"), kb.addParent(ka));
    a.check("06. hasParent", !ka.hasParent(kb));
    a.check("07. hasParent", kb.hasParent(ka));
    AFL_CHECK_THROWS(a("08. addParent"), kb.addParent(ka), std::runtime_error);
    AFL_CHECK_THROWS(a("09. addParent"), ka.addParent(kb), std::runtime_error);
    AFL_CHECK_THROWS(a("10. addParent"), ka.addParent(ka), std::runtime_error);
    AFL_CHECK_THROWS(a("11. addParent"), kb.addParent(kb), std::runtime_error);

    a.checkEqual("21. getNumDirectParents", ka.getNumDirectParents(), 0U);
    a.checkEqual("22. getNumDirectParents", kb.getNumDirectParents(), 1U);

    a.checkEqual("31. getDirectParent", kb.getDirectParent(0), &ka);
    a.checkEqual("32. getDirectParent", kb.getDirectParent(1), (util::Keymap*) 0);

    // Check keys
    ka.addKey(1, 2, 3);
    ka.addKey(4, 5, 6);
    kb.addKey(1, 4, 5);
    kb.addKey(7, 8, 9);
    a.checkEqual("41. lookupCommand", ka.lookupCommand(1), 2U);
    a.checkEqual("42. lookupCommand", ka.lookupCommand(4), 5U);
    a.checkEqual("43. lookupCommand", ka.lookupCommand(7), 0U);
    a.checkEqual("44. lookupCommand", ka.lookupCommand(99), 0U);
    a.checkEqual("45. lookupCommand", kb.lookupCommand(1), 4U);
    a.checkEqual("46. lookupCommand", kb.lookupCommand(4), 5U);
    a.checkEqual("47. lookupCommand", kb.lookupCommand(7), 8U);
    a.checkEqual("48. lookupCommand", kb.lookupCommand(99), 0U);

    // Look up, asking for place of definition
    util::KeymapRef_t where;
    kb.lookupCommand(1, where);
    a.checkEqual("51. lookupCommand", where, &kb);
    kb.lookupCommand(4, where);
    a.checkEqual("52. lookupCommand", where, &ka);

    // Look up conditions
    a.checkEqual("61. lookupCondition", ka.lookupCondition(1), 3U);
    a.checkEqual("62. lookupCondition", ka.lookupCondition(4), 6U);
    a.checkEqual("63. lookupCondition", ka.lookupCondition(7), 0U);
    a.checkEqual("64. lookupCondition", ka.lookupCondition(99), 0U);
    a.checkEqual("65. lookupCondition", kb.lookupCondition(1), 5U);
    a.checkEqual("66. lookupCondition", kb.lookupCondition(4), 6U);
    a.checkEqual("67. lookupCondition", kb.lookupCondition(7), 9U);
    a.checkEqual("68. lookupCondition", kb.lookupCondition(99), 0U);
}

/** Test change tracking. */
AFL_TEST("util.KeyMap:change", a)
{
    util::Keymap ka("TEST");
    a.check("01. isChanged", !ka.isChanged());

    ka.addKey(1, 2, 3);
    a.check("11. isChanged", ka.isChanged());
    ka.markChanged(false);

    ka.addKey(1, 2, 3);
    a.check("21. isChanged", !ka.isChanged());

    ka.addKey(1, 2, 4);
    a.check("31. isChanged", ka.isChanged());
    ka.markChanged(false);
}

/** Test describe(). */
AFL_TEST("util.KeyMap:describe", a)
{
    util::Keymap ka("A");
    util::Keymap ka1("A1");
    util::Keymap ka1b("A1B");
    util::Keymap ka2("A2");
    ka.addParent(ka1);
    ka1.addParent(ka1b);
    ka.addParent(ka2);

    // Describe A with big limit
    {
        util::KeymapInformation info;
        ka.describe(info, 99);

        a.checkEqual("01. size", info.size(), 4U);

        size_t level;
        String_t name;
        a.checkEqual("11", info.get(0, level, name), true);
        a.checkEqual("12", level, 0U);
        a.checkEqual("13", name, "A");

        a.checkEqual("21", info.get(1, level, name), true);
        a.checkEqual("22", level, 1U);
        a.checkEqual("23", name, "A1");

        a.checkEqual("31", info.get(2, level, name), true);
        a.checkEqual("32", level, 2U);
        a.checkEqual("33", name, "A1B");

        a.checkEqual("41", info.get(3, level, name), true);
        a.checkEqual("42", level, 1U);
        a.checkEqual("43", name, "A2");
    }

    // Describe A with low limit
    {
        util::KeymapInformation info;
        ka.describe(info, 1);

        a.checkEqual("51. size", info.size(), 4U);

        size_t level;
        String_t name;
        a.checkEqual("61", info.get(0, level, name), true);
        a.checkEqual("62", level, 0U);
        a.checkEqual("63", name, "A");

        a.checkEqual("71", info.get(1, level, name), true);
        a.checkEqual("72", level, 1U);
        a.checkEqual("73", name, "A1");

        a.checkEqual("81", info.get(2, level, name), true);
        a.checkEqual("82", level, 2U);     // placeholder
        a.checkEqual("83", name, "");

        a.checkEqual("91", info.get(3, level, name), true);
        a.checkEqual("92", level, 1U);
        a.checkEqual("93", name, "A2");
    }

    // Describe A with very low limit
    {
        util::KeymapInformation info;
        ka.describe(info, 0);

        a.checkEqual("101. size", info.size(), 2U);

        size_t level;
        String_t name;
        a.checkEqual("111", info.get(0, level, name), true);
        a.checkEqual("112", level, 0U);
        a.checkEqual("113", name, "A");

        a.checkEqual("121", info.get(1, level, name), true);
        a.checkEqual("122", level, 1U);     // placeholder
        a.checkEqual("123", name, "");
    }
}

/** Test describe() with multiple inheritance. */
AFL_TEST("util.KeyMap:describe:multiple-inheritance", a)
{
    util::Keymap base("BASE"), left("LEFT"), right("RIGHT"), common("COMMON");
    common.addParent(left);
    common.addParent(right);
    left.addParent(base);
    right.addParent(base);

    // Describe COMMON
    util::KeymapInformation info;
    common.describe(info, 99);

    a.checkEqual("01. size", info.size(), 4U);

    size_t level;
    String_t name;
    a.checkEqual("11", info.get(0, level, name), true);
    a.checkEqual("12", level, 0U);
    a.checkEqual("13", name, "COMMON");

    a.checkEqual("21", info.get(1, level, name), true);
    a.checkEqual("22", level, 1U);
    a.checkEqual("23", name, "LEFT");

    a.checkEqual("31", info.get(2, level, name), true);
    a.checkEqual("32", level, 2U);
    a.checkEqual("33", name, "BASE");

    a.checkEqual("41", info.get(3, level, name), true);
    a.checkEqual("42", level, 1U);
    a.checkEqual("43", name, "RIGHT");
}
