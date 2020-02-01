/**
  *  \file u/t_util_atomtable.cpp
  *  \brief Test for util::AtomTable
  */

#include "util/atomtable.hpp"

#include "t_util.hpp"
#include "afl/string/format.hpp"

namespace {
    String_t toString(size_t i)
    {
        return afl::string::Format("x%d", i);
    }
}

/** Simple test. */
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
    TS_ASSERT_EQUALS(testee.getAtomFromString(String_t("foo")), foo);
    TS_ASSERT_EQUALS(testee.getAtomFromString("bar"), bar);
    TS_ASSERT_EQUALS(testee.getStringFromAtom(foo), "foo");
    TS_ASSERT_EQUALS(testee.getStringFromAtom(bar), "bar");
    TS_ASSERT_EQUALS(testee.getAtomFromStringNC("foo"), foo);
    TS_ASSERT_EQUALS(testee.getAtomFromStringNC(String_t("foo")), foo);
    TS_ASSERT_DIFFERS(testee.getAtomFromString("FOO"), foo);

    TS_ASSERT_EQUALS(testee.getAtomFromString(String_t("foo")), foo);
    TS_ASSERT_EQUALS(testee.getAtomFromString(String_t("bar")), bar);
}

/** Test many atoms.
    This exercises hash-bucket overflow. */
void
TestUtilAtomTable::testManyAtoms()
{
    util::AtomTable testee;
    std::vector<util::Atom_t> atoms;

    // Create 10000 atoms
    for (size_t i = 0; i < 10000; ++i) {
        atoms.push_back(testee.getAtomFromString(toString(i)));
    }

    // Verify both directions
    for (size_t i = 0; i < 10000; ++i) {
        TS_ASSERT_EQUALS(atoms[i], testee.getAtomFromString(toString(i)));
        TS_ASSERT_EQUALS(testee.getStringFromAtom(atoms[i]), toString(i));
    }
}

