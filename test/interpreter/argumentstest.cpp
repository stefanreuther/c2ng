/**
  *  \file test/interpreter/argumentstest.cpp
  *  \brief Test for interpreter::Arguments
  */

#include "interpreter/arguments.hpp"

#include "afl/data/segment.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "util/atomtable.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("interpreter.Arguments:basics", a)
{
    // Prepare a segment
    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBack(0);
    seg.pushBackString("x");
    a.checkEqual("01. size", seg.size(), 3U);

    // Testee
    interpreter::Arguments testee(seg, 0, 3);
    a.checkEqual("11. getNumArgs", testee.getNumArgs(), 3U);

    AFL_CHECK_SUCCEEDS(a("21. checkArgumentCount"),        testee.checkArgumentCount(3));
    AFL_CHECK_SUCCEEDS(a("22. checkArgumentCountAtLeast"), testee.checkArgumentCountAtLeast(3));
    AFL_CHECK_SUCCEEDS(a("23. checkArgumentCountAtLeast"), testee.checkArgumentCountAtLeast(2));
    AFL_CHECK_SUCCEEDS(a("24. checkArgumentCount"),        testee.checkArgumentCount(0, 3));
    AFL_CHECK_SUCCEEDS(a("25. checkArgumentCount"),        testee.checkArgumentCount(3, 4));

    AFL_CHECK_THROWS(a("31. checkArgumentCount"),        testee.checkArgumentCount(2), interpreter::Error);
    AFL_CHECK_THROWS(a("32. checkArgumentCount"),        testee.checkArgumentCount(4), interpreter::Error);
    AFL_CHECK_THROWS(a("33. checkArgumentCountAtLeast"), testee.checkArgumentCountAtLeast(4), interpreter::Error);
    AFL_CHECK_THROWS(a("34. checkArgumentCount"),        testee.checkArgumentCount(0, 2), interpreter::Error);
    AFL_CHECK_THROWS(a("35. checkArgumentCount"),        testee.checkArgumentCount(4, 5), interpreter::Error);

    // Consume args
    afl::data::Value* p1 = testee.getNext();
    afl::data::Value* p2 = testee.getNext();
    a.checkEqual("41. getNumArgs", testee.getNumArgs(), 1U);
    a.checkEqual("42. getNext", p1, seg[0]);
    a.checkEqual("43. getNext", p2, seg[1]);

    afl::data::Value* p3 = testee.getNext();
    afl::data::Value* p4 = testee.getNext();
    a.checkEqual("51. getNumArgs", testee.getNumArgs(), 0U);
    a.checkEqual("52. getNext", p3, seg[2]);
    a.checkNull("53. getNext", p4);
}

/** Test checkArgumentCount(). */
AFL_TEST("interpreter.Arguments:checkArgumentCount", a)
{
    AFL_CHECK_SUCCEEDS(a("0/0/0"), interpreter::checkArgumentCount(0, 0, 0));

    AFL_CHECK_THROWS  (a("1/0/0"), interpreter::checkArgumentCount(1, 0, 0), interpreter::Error);
    AFL_CHECK_SUCCEEDS(a("1/0/1"), interpreter::checkArgumentCount(1, 0, 1));
    AFL_CHECK_SUCCEEDS(a("1/1/1"), interpreter::checkArgumentCount(1, 1, 1));

    // These are the Argument test-cases:
    AFL_CHECK_SUCCEEDS(a("3/3/3"), interpreter::checkArgumentCount(3, 3, 3));
    AFL_CHECK_SUCCEEDS(a("3/2/3"), interpreter::checkArgumentCount(3, 2, 3));
    AFL_CHECK_SUCCEEDS(a("3/0/3"), interpreter::checkArgumentCount(3, 0, 3));
    AFL_CHECK_SUCCEEDS(a("3/3/4"), interpreter::checkArgumentCount(3, 3, 4));

    AFL_CHECK_THROWS(a("3/2/2"), interpreter::checkArgumentCount(3, 2, 2), interpreter::Error);
    AFL_CHECK_THROWS(a("3/4/4"), interpreter::checkArgumentCount(3, 4, 4), interpreter::Error);
    AFL_CHECK_THROWS(a("3/4/3"), interpreter::checkArgumentCount(3, 4, 3), interpreter::Error);
    AFL_CHECK_THROWS(a("3/0/2"), interpreter::checkArgumentCount(3, 0, 2), interpreter::Error);
    AFL_CHECK_THROWS(a("3/4/5"), interpreter::checkArgumentCount(3, 4, 5), interpreter::Error);
}

/*
 *  checkIntegerArg()
 */

// Null
AFL_TEST("interpreter.Arguments:checkIntegerArg:null", a)
{
    int32_t iv = 0;
    a.check("unlimited", !interpreter::checkIntegerArg(iv, 0));
    a.check("range",     !interpreter::checkIntegerArg(iv, 0, 1, 10));
}

// Integer
AFL_TEST("interpreter.Arguments:checkIntegerArg:int", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
    a.check("checkIntegerArg", interpreter::checkIntegerArg(iv, p.get()));
    a.checkEqual("value", iv, 3);
}

AFL_TEST("interpreter.Arguments:checkIntegerArg:int:in-range", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
    a.check("checkIntegerArg", interpreter::checkIntegerArg(iv, p.get(), 1, 10));
    a.checkEqual("value", iv, 3);
}

AFL_TEST("interpreter.Arguments:checkIntegerArg:int:out-of-range", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
    AFL_CHECK_THROWS(a, interpreter::checkIntegerArg(iv, p.get(), 0, 2), interpreter::Error);
}

// String
AFL_TEST("interpreter.Arguments:checkIntegerArg::str", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("hi"));
    AFL_CHECK_THROWS(a("unlimited"), interpreter::checkIntegerArg(iv, p.get()),        interpreter::Error);
    AFL_CHECK_THROWS(a("range"),     interpreter::checkIntegerArg(iv, p.get(), 1, 10), interpreter::Error);
}

// String: no implicit destringification!
AFL_TEST("interpreter.Arguments:checkIntegerArg:str:2", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("7"));
    AFL_CHECK_THROWS(a("unlimited"), interpreter::checkIntegerArg(iv, p.get()),        interpreter::Error);
    AFL_CHECK_THROWS(a("range"),     interpreter::checkIntegerArg(iv, p.get(), 1, 10), interpreter::Error);
}

// Bool
AFL_TEST("interpreter.Arguments:checkIntegerArg:bool", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
    a.check("checkIntegerArg", interpreter::checkIntegerArg(iv, p.get()));
    a.checkEqual("value", iv, 1);
}

AFL_TEST("interpreter.Arguments:checkIntegerArg:bool:in-range", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
    a.check("checkIntegerArg", interpreter::checkIntegerArg(iv, p.get(), 1, 10));
    a.checkEqual("value", iv, 1);
}

AFL_TEST("interpreter.Arguments:checkIntegerArg:bool:out-of-range", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
    AFL_CHECK_THROWS(a, interpreter::checkIntegerArg(iv, p.get(), 2, 5), interpreter::Error);
}

// Float
AFL_TEST("interpreter.Arguments:checkIntegerArg:float", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
    a.check("checkIntegerArg", interpreter::checkIntegerArg(iv, p.get()));
    a.checkEqual("value", iv, 16);
}

AFL_TEST("interpreter.Arguments:checkIntegerArg:float:in-range", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
    a.check("checkIntegerArg", interpreter::checkIntegerArg(iv, p.get(), 1, 16));
    a.checkEqual("value", iv, 16);
}

AFL_TEST("interpreter.Arguments:checkIntegerArg:float:out-of-range", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
    AFL_CHECK_THROWS(a, interpreter::checkIntegerArg(iv, p.get(), 0, 5), interpreter::Error);
}

// Float overflow
AFL_TEST("interpreter.Arguments:checkIntegerArg:float:overflow", a)
{
    int32_t iv = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0E20));
    AFL_CHECK_THROWS(a("unlimited"), interpreter::checkIntegerArg(iv, p.get()),        interpreter::Error);
    AFL_CHECK_THROWS(a("range"),     interpreter::checkIntegerArg(iv, p.get(), 1, 10), interpreter::Error);
}


/*
 *  checkBooleanArg()
 */

// Null
AFL_TEST("interpreter.Arguments:checkBooleanArg:null", a)
{
    bool bv = false;
    a.check("checkBooleanArg", !interpreter::checkBooleanArg(bv, 0));
}

// Integer
AFL_TEST("interpreter.Arguments:checkBooleanArg:int:true", a)
{
    bool bv = false;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, true);
}

AFL_TEST("interpreter.Arguments:checkBooleanArg:int:false", a)
{
    bool bv = true;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(0));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, false);
}

// String
AFL_TEST("interpreter.Arguments:checkBooleanArg:str:true", a)
{
    bool bv = false;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("hi"));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, true);
}

AFL_TEST("interpreter.Arguments:checkBooleanArg:str:false", a)
{
    bool bv = false;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue(""));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, false);
}

// Bool
AFL_TEST("interpreter.Arguments:checkBooleanArg:bool:true", a)
{
    bool bv = false;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, true);
}

AFL_TEST("interpreter.Arguments:checkBooleanArg:bool:false", a)
{
    bool bv = true;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(0));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, false);
}

// Float
AFL_TEST("interpreter.Arguments:checkBooleanArg", a)
{
    bool bv = false;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
    a.check("checkBooleanArg", interpreter::checkBooleanArg(bv, p.get()));
    a.checkEqual("value", bv, true);
}


/*
 *  checkStringArg()
 */

// Null
AFL_TEST("interpreter.Arguments:checkStringArg:null", a)
{
    String_t sv;
    a.check("checkStringArg", !interpreter::checkStringArg(sv, 0));
}

// Integer
AFL_TEST("interpreter.Arguments:checkStringArg:int", a)
{
    String_t sv;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
    a.check("checkStringArg", interpreter::checkStringArg(sv, p.get()));
    a.checkEqual("value", sv, "3");
}

// String
AFL_TEST("interpreter.Arguments:checkStringArg:str", a)
{
    String_t sv;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("hi"));
    a.check("checkStringArg", interpreter::checkStringArg(sv, p.get()));
    a.checkEqual("value", sv, "hi");
}

// Bool
AFL_TEST("interpreter.Arguments:checkStringArg:bool", a)
{
    String_t sv;
    std::auto_ptr<afl::data::Value> p(interpreter::makeBooleanValue(1));
    a.check("checkStringArg", interpreter::checkStringArg(sv, p.get()));
    a.checkEqual("value", sv, "YES");
}

// Float
AFL_TEST("interpreter.Arguments:checkStringArg:float", a)
{
    String_t sv;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(16.25));
    a.check("checkStringArg", interpreter::checkStringArg(sv, p.get()));
    a.checkEqual("value", sv, "16.25");
}

// Huge float
AFL_TEST("interpreter.Arguments:checkStringArg:float:large", a)
{
    String_t sv;
    std::auto_ptr<afl::data::Value> p(interpreter::makeFloatValue(1.0E20));
    a.check("checkStringArg", interpreter::checkStringArg(sv, p.get()));
    a.checkEqual("value", sv, "100000000000000000000");
}

/*
 *  checkFlagArg
 */

// Null
AFL_TEST("interpreter.Arguments:checkFlagArg:null", a)
{
    int32_t flags = 9, value = 0;
    a.check("checkFlagArg", !interpreter::checkFlagArg(flags, &value, 0, "XYZ"));
    a.checkEqual("flags", flags, 9);    // on 'false' return, other outputs are unchanged
}

// Integer
AFL_TEST("interpreter.Arguments:checkFlagArg:int", a)
{
    int32_t flags = 0, value = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(3));
    a.check("checkFlagArg", interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
    a.checkEqual("flags", flags, 0);
    a.checkEqual("value", value, 3);

    // Fails if no value requested: integer should go in value slot
    AFL_CHECK_THROWS(a("checkFlagArg(null)"), interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
}

// String
AFL_TEST("interpreter.Arguments:checkFlagArg:str", a)
{
    int32_t flags = 64, value = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("XY7"));
    a.check("checkFlagArg", interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
    a.checkEqual("flags", flags, 3);
    a.checkEqual("value", value, 7);

    // Fails if no value requested: value specified in string
    AFL_CHECK_THROWS(a("checkFlagArg(null)"), interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
}

// String, flags at back
AFL_TEST("interpreter.Arguments:checkFlagArg:str:flags-at-end", a)
{
    int32_t flags = 0, value = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("9XY"));
    a.check("checkFlagArg", interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
    a.checkEqual("flags", flags, 3);
    a.checkEqual("value", value, 9);

    // Fails if no value requested: value specified in string
    AFL_CHECK_THROWS(a("checkFlagArg(null)"), interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
}

// String, just flags
AFL_TEST("interpreter.Arguments:checkFlagArg:str:just-flags", a)
{
    int32_t flags = 0, value = 77;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("XZ"));
    a.check("checkFlagArg", interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
    a.checkEqual("flags", flags, 5);
    a.checkEqual("value", value, 77);      // unchanged because not specified!

    // Succeeds if no value requested
    flags = 0;
    a.check("checkFlagArg", interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"));
    a.checkEqual("flags", flags, 5);
}

// String, value in the middle
AFL_TEST("interpreter.Arguments:checkFlagArg:str:middle-value", a)
{
    int32_t flags = 0, value = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("X3Z"));
    a.check("checkFlagArg", interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"));
    a.checkEqual("flags", flags, 5);
    a.checkEqual("value", value, 3);

    // Fails if no value requested: value specified in string
    AFL_CHECK_THROWS(a("checkFlagArg(null)"), interpreter::checkFlagArg(flags, 0, p.get(), "XYZ"), interpreter::Error);
}

// String, multiple numbers (bad syntax)
AFL_TEST("interpreter.Arguments:checkFlagArg:str:error:too-many-values", a)
{
    int32_t flags = 0, value = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("X3Z5"));
    AFL_CHECK_THROWS(a("checkFlagArg"),       interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"), interpreter::Error);
    AFL_CHECK_THROWS(a("checkFlagArg(null)"), interpreter::checkFlagArg(flags, 0,      p.get(), "XYZ"), interpreter::Error);
}

// String, bad flags
AFL_TEST("interpreter.Arguments:checkFlagArg:str:error:bad-flags", a)
{
    int32_t flags = 0, value = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("XA"));
    AFL_CHECK_THROWS(a("checkFlagArg"),       interpreter::checkFlagArg(flags, &value, p.get(), "XYZ"), interpreter::Error);
    AFL_CHECK_THROWS(a("checkFlagArg(null)"), interpreter::checkFlagArg(flags, 0,      p.get(), "XYZ"), interpreter::Error);
}

/*
 *  checkCommandAtomArg
 */

// Null
AFL_TEST("interpreter.Arguments:checkCommandAtomArg:null", a)
{
    util::AtomTable tab;
    util::Atom_t result = 0;
    a.check("checkCommandAtomArg", !interpreter::checkCommandAtomArg(result, 0, tab));
}

// Integer
AFL_TEST("interpreter.Arguments:checkCommandAtomArg:int", a)
{
    util::AtomTable tab;
    util::Atom_t result = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeIntegerValue(333));
    a.check("checkCommandAtomArg", interpreter::checkCommandAtomArg(result, p.get(), tab));
    a.checkEqual("result", result, util::Atom_t(333));
}

// String
AFL_TEST("interpreter.Arguments:checkCommandAtomArg:str:existing-atom", a)
{
    util::AtomTable tab;
    util::Atom_t at = tab.getAtomFromString("foo");
    util::Atom_t result = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("foo"));
    a.check("checkCommandAtomArg", interpreter::checkCommandAtomArg(result, p.get(), tab));
    a.checkEqual("result", result, at);
}

// String (new atom)
AFL_TEST("interpreter.Arguments:checkCommandAtomArg:str:new-atom", a)
{
    util::AtomTable tab;
    util::Atom_t at = tab.getAtomFromString("foo");
    util::Atom_t result = 0;
    std::auto_ptr<afl::data::Value> p(interpreter::makeStringValue("bar"));
    a.check("checkCommandAtomArg", interpreter::checkCommandAtomArg(result, p.get(), tab));
    a.checkDifferent("result", result, at);
    a.checkEqual("result", result, tab.getAtomFromString("bar"));
}
