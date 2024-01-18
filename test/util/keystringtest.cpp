/**
  *  \file test/util/keystringtest.cpp
  *  \brief Test for util::KeyString
  */

#include "util/keystring.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("util.KeyString", a)
{
    util::KeyString testee("Yes");
    a.checkEqual("01. getKey", testee.getKey(), util::Key_t('y'));
    a.checkEqual("02. getString", testee.getString(), "Yes");

    util::KeyString test2(" foo ");
    a.checkEqual("11. getKey", test2.getKey(), util::Key_t(0));
    a.checkEqual("12. getString", test2.getString(), " foo ");

    util::KeyString test3(String_t("No"));
    a.checkEqual("21. getKey", test3.getKey(), util::Key_t('n'));
    a.checkEqual("22. getString", test3.getString(), "No");

    util::KeyString test4("ha", util::Key_F7);
    a.checkEqual("31. getKey", test4.getKey(), util::Key_F7);
    a.checkEqual("32. getString", test4.getString(), "ha");
}
