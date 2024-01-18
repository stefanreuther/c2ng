/**
  *  \file test/util/syntax/keywordtabletest.cpp
  *  \brief Test for util::syntax::KeywordTable
  */

#include "util/syntax/keywordtable.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"

/** Simple add/get test. */
AFL_TEST("util.syntax.KeywordTable:basics", a)
{
    util::syntax::KeywordTable testee;

    // Initial state
    a.checkNull("01", testee.get("foo"));

    // Store and retrieve
    testee.add("foo", "bar");
    a.checkNonNull("11", testee.get("foo"));
    a.checkEqual("12", *testee.get("foo"), "bar");

    // Case-insensitive access
    a.checkNonNull("21", testee.get("Foo"));
    a.checkEqual("22", *testee.get("Foo"), "bar");
    a.checkNonNull("23", testee.get("FOO"));
    a.checkEqual("24", *testee.get("FOO"), "bar");
    a.checkNonNull("25", testee.get("foO"));
    a.checkEqual("26", *testee.get("foO"), "bar");

    // Case-insensitive replacement
    testee.add("FOO", "what?");
    a.checkNonNull("31", testee.get("Foo"));
    a.checkEqual("32", *testee.get("Foo"), "what?");
}

/*
 *  Test load errors.
 */

// Sanity check
AFL_TEST("util.syntax.KeywordTable:load:success", a)
{
    afl::io::ConstMemoryStream ms(afl::base::Nothing);
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 0U);
}

// Syntax error on one line
AFL_TEST("util.syntax.KeywordTable:load:error:syntax", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("foo"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}

// Two syntax errors (proves that parsing proceeds)
AFL_TEST("util.syntax.KeywordTable:load:error:two", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("foo\nbar"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 2U);
}

// Empty key
AFL_TEST("util.syntax.KeywordTable:load:error:empty-key", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("=x"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}

// Bad block
AFL_TEST("util.syntax.KeywordTable:load:error:open-block", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("{"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}

// Bad block
AFL_TEST("util.syntax.KeywordTable:load:error:open-block-syntax", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("{x"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("etNumMessages", c.getNumMessages(), 1U);
}

// Bad block closer
AFL_TEST("util.syntax.KeywordTable:load:error:close-block-syntax", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("}x"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}

// Bad block closer
AFL_TEST("util.syntax.KeywordTable:load:error:close-block-syntax:2", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("x}"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}

// Badly-placed block closer (missing opener)
AFL_TEST("util.syntax.KeywordTable:load:error:missing-opener", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("}"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}

// Badly-placed block closer (missing opener)
AFL_TEST("util.syntax.KeywordTable:load:error:missing-opener:2", a)
{
    util::syntax::KeywordTable tab;
    afl::io::ConstMemoryStream ms(afl::string::toBytes("x {\na=b\n}\n}"));
    afl::test::LogListener c;
    tab.load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);

    const String_t* p = tab.get("x.a");
    a.check("get", p);
    a.checkEqual("value", *p, "b");
}

// Bad reference
AFL_TEST("util.syntax.KeywordTable:load:error:bad-reference", a)
{
    afl::io::ConstMemoryStream ms(afl::string::toBytes("a = 1\nb = $a\nx = $y\n"));
    afl::test::LogListener c;
    util::syntax::KeywordTable().load(ms, c);
    a.checkEqual("getNumMessages", c.getNumMessages(), 1U);
}


/*
 *  Test load success.
 */
AFL_TEST("util.syntax.KeywordTable:load:success:complex", a)
{
    // A simple test file
    afl::io::ConstMemoryStream ms(afl::string::toBytes("; header\n"
                                                       "; a = foo\n"
                                                       "b = bar\n"
                                                       "c = baz\n"
                                                       "c.x = iks\n"
                                                       "D = $c\n"
                                                       "d.x=$C.X\n"
                                                       "e {\n"
                                                       "a = eee\n"
                                                       "b = fff\n"
                                                       "c = $b\n"
                                                       "}\n"
                                                       "f = 1\n"
                                                       "c {\n"
                                                       "  x = iks again\n"
                                                       "  d.e.f {\n"
                                                       "    g = gg\n"
                                                       "  }\n"
                                                       "}\n"));

    // Parse it. Must be silent (no messages)
    afl::test::LogListener c;
    util::syntax::KeywordTable testee;
    testee.load(ms, c);
    a.checkEqual("01. getNumMessages", c.getNumMessages(), 0U);

    // Verify content
    a.checkNull("11", testee.get("a"));
    a.checkNull("12", testee.get("; a"));

    a.checkNonNull("21", testee.get("b"));
    a.checkEqual("22", *testee.get("b"), "bar");

    a.checkNonNull("31", testee.get("c"));
    a.checkEqual("32", *testee.get("c"), "baz");

    a.checkNonNull("41", testee.get("c.x"));
    a.checkEqual("42", *testee.get("c.x"), "iks again");  // overwritten by subsequent assignment

    a.checkNonNull("51", testee.get("d"));
    a.checkEqual("52", *testee.get("d"), "baz");

    a.checkNonNull("61", testee.get("d.x"));
    a.checkEqual("62", *testee.get("d.x"), "iks");  // copied from original value before it is overwritten

    a.checkNull("71", testee.get("e"));

    a.checkNonNull("81", testee.get("e.a"));
    a.checkEqual("82", *testee.get("e.a"), "eee");

    a.checkNonNull("91", testee.get("e.b"));
    a.checkEqual("92", *testee.get("e.b"), "fff");

    a.checkNonNull("101", testee.get("e.c"));
    a.checkEqual("102", *testee.get("e.c"), "bar");

    a.checkNonNull("111", testee.get("f"));
    a.checkEqual("112", *testee.get("f"), "1");

    a.checkNonNull("121", testee.get("c.d.e.f.g"));
    a.checkEqual("122", *testee.get("c.d.e.f.g"), "gg");
}
