/**
  *  \file u/t_util_atomtable.cpp
  *  \brief Test for util::AtomTable
  */

#include "util/atomtable.hpp"

#include "t_util.hpp"

void
TestUtilAtomTable::testAtom()
{
    // ex UtilAtomTestSuite::testAtom
    util::AtomTable testee;

    TS_ASSERT(!testee.isAtom(util::AtomTable::NULL_ATOM));
    TS_ASSERT(!testee.isAtom(util::AtomTable::ATOM_LOWER_BOUND));

    TS_ASSERT(!testee.getAtomFromStringNC("foo"));
    TS_ASSERT(!testee.getAtomFromStringNC("bar"));

    // now add some atoms
    util::Atom_t foo = testee.getAtomFromString("foo");
    util::Atom_t bar = testee.getAtomFromString("bar");
    TS_ASSERT(testee.isAtom(foo));
    TS_ASSERT(testee.isAtom(bar));
    TS_ASSERT_DIFFERS(foo, bar);
    TS_ASSERT_EQUALS(testee.getAtomFromString("foo"), foo);
    TS_ASSERT_EQUALS(testee.getAtomFromString("bar"), bar);
    TS_ASSERT_EQUALS(testee.getStringFromAtom(foo), "foo");
    TS_ASSERT_EQUALS(testee.getStringFromAtom(bar), "bar");
    TS_ASSERT_EQUALS(testee.getAtomFromStringNC("foo"), foo);
    TS_ASSERT_DIFFERS(testee.getAtomFromString("FOO"), foo);

    TS_ASSERT_EQUALS(testee.getAtomFromString(String_t("foo")), foo);
    TS_ASSERT_EQUALS(testee.getAtomFromString(String_t("bar")), bar);
}

