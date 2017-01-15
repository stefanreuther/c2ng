/**
  *  \file u/t_util_keystring.cpp
  *  \brief Test for util::KeyString
  */

#include "util/keystring.hpp"

#include "t_util.hpp"

/** Simple test. */
void
TestUtilKeyString::testIt()
{
    util::KeyString testee("Yes");
    TS_ASSERT_EQUALS(testee.getKey(), util::Key_t('y'));
    TS_ASSERT_EQUALS(testee.getString(), "Yes");

    util::KeyString test2(" foo ");
    TS_ASSERT_EQUALS(test2.getKey(), util::Key_t(0));
    TS_ASSERT_EQUALS(test2.getString(), " foo ");

    util::KeyString test3(String_t("No"));
    TS_ASSERT_EQUALS(test3.getKey(), util::Key_t('n'));
    TS_ASSERT_EQUALS(test3.getString(), "No");
}

