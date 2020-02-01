/**
  *  \file u/t_util_vector.cpp
  *  \brief Test for util::Vector
  */

#include "util/vector.hpp"

#include "t_util.hpp"

/** Test 0-based vector. */
void
TestUtilVector::testIt()
{
    // Create empty vector and verify it
    util::Vector<char,int> testee;
    TS_ASSERT(testee.at(0) == 0);
    TS_ASSERT_EQUALS(testee.size(), 0);
    TS_ASSERT_EQUALS(testee.get(0), '\0');
    TS_ASSERT_EQUALS(testee.get(99), '\0');
    TS_ASSERT_EQUALS(testee.empty(), true);

    // Set one element and verify
    testee.set(2, 'x');
    TS_ASSERT_EQUALS(testee.size(), 3);
    TS_ASSERT_EQUALS(testee.get(0), '\0');
    TS_ASSERT_EQUALS(testee.get(2), 'x');
    TS_ASSERT_EQUALS(testee.get(99), '\0');
    TS_ASSERT_EQUALS(testee.empty(), false);

    TS_ASSERT(testee.at(0) != 0);
    TS_ASSERT(testee.at(1) != 0);
    TS_ASSERT(testee.at(2) != 0);
    TS_ASSERT(testee.at(3) == 0);
    TS_ASSERT_EQUALS(*testee.at(2), 'x');

    // Clear and verify
    testee.clear();
    TS_ASSERT(testee.at(0) == 0);
    TS_ASSERT_EQUALS(testee.size(), 0);
    TS_ASSERT_EQUALS(testee.get(0), '\0');
    TS_ASSERT_EQUALS(testee.get(99), '\0');
    TS_ASSERT_EQUALS(testee.empty(), true);
}

/** Test 1-based vector. */
void
TestUtilVector::test1Based()
{
    // Create empty vector and verify it
    util::Vector<char,int> testee(1);
    TS_ASSERT(testee.at(0) == 0);
    TS_ASSERT_EQUALS(testee.size(), 1);
    TS_ASSERT_EQUALS(testee.get(0), '\0');
    TS_ASSERT_EQUALS(testee.get(99), '\0');
    TS_ASSERT_EQUALS(testee.empty(), true);

    // Set one element and verify
    testee.set(2, 'x');
    TS_ASSERT_EQUALS(testee.size(), 3);
    TS_ASSERT_EQUALS(testee.get(0), '\0');
    TS_ASSERT_EQUALS(testee.get(2), 'x');
    TS_ASSERT_EQUALS(testee.get(99), '\0');
    TS_ASSERT_EQUALS(testee.empty(), false);

    TS_ASSERT(testee.at(0) == 0);
    TS_ASSERT(testee.at(1) != 0);
    TS_ASSERT(testee.at(2) != 0);
    TS_ASSERT(testee.at(3) == 0);
    TS_ASSERT_EQUALS(*testee.at(2), 'x');
}

