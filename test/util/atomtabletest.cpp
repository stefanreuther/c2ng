/**
  *  \file test/util/atomtabletest.cpp
  *  \brief Test for util::AtomTable
  */

#include "util/atomtable.hpp"

#include "afl/string/format.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    String_t toString(size_t i)
    {
        return afl::string::Format("x%d", i);
    }
}

/** Simple test. */
AFL_TEST("util.AtomTable:basics", a)
{
    // ex UtilAtomTestSuite::testAtom
    util::AtomTable testee;

    a.check("01", !testee.isAtom(util::AtomTable::NULL_ATOM));
    a.check("02", !testee.isAtom(util::AtomTable::ATOM_LOWER_BOUND));

    a.check("11", !testee.getAtomFromStringNC("foo"));
    a.check("12", !testee.getAtomFromStringNC("bar"));

    // now add some atoms
    util::Atom_t foo = testee.getAtomFromString("foo");
    util::Atom_t bar = testee.getAtomFromString("bar");
    a.check("21", testee.isAtom(foo));
    a.check("22", testee.isAtom(bar));
    a.checkDifferent("23", foo, bar);
    a.checkEqual("24", testee.getAtomFromString("foo"), foo);
    a.checkEqual("25", testee.getAtomFromString(String_t("foo")), foo);
    a.checkEqual("26", testee.getAtomFromString("bar"), bar);
    a.checkEqual("27", testee.getStringFromAtom(foo), "foo");
    a.checkEqual("28", testee.getStringFromAtom(bar), "bar");
    a.checkEqual("29", testee.getAtomFromStringNC("foo"), foo);
    a.checkEqual("30", testee.getAtomFromStringNC(String_t("foo")), foo);
    a.checkDifferent("31", testee.getAtomFromString("FOO"), foo);

    a.checkEqual("41", testee.getAtomFromString(String_t("foo")), foo);
    a.checkEqual("42", testee.getAtomFromString(String_t("bar")), bar);
}

/** Test many atoms.
    This exercises hash-bucket overflow. */
AFL_TEST("util.AtomTable:many-atoms", a)
{
    util::AtomTable testee;
    std::vector<util::Atom_t> atoms;

    // Create 10000 atoms
    for (size_t i = 0; i < 10000; ++i) {
        atoms.push_back(testee.getAtomFromString(toString(i)));
    }

    // Verify both directions
    for (size_t i = 0; i < 10000; ++i) {
        a.checkEqual("01", atoms[i], testee.getAtomFromString(toString(i)));
        a.checkEqual("02", testee.getStringFromAtom(atoms[i]), toString(i));
    }
}
