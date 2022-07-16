/**
  *  \file u/t_util_syntax_keywordtable.cpp
  *  \brief Test for util::syntax::KeywordTable
  */

#include "util/syntax/keywordtable.hpp"

#include "t_util_syntax.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/sys/loglistener.hpp"
#include "afl/test/loglistener.hpp"

/** Simple add/get test. */
void
TestUtilSyntaxKeywordTable::testAccess()
{
    util::syntax::KeywordTable testee;

    // Initial state
    TS_ASSERT(testee.get("foo") == 0);

    // Store and retrieve
    testee.add("foo", "bar");
    TS_ASSERT(testee.get("foo") != 0);
    TS_ASSERT_EQUALS(*testee.get("foo"), "bar");

    // Case-insensitive access
    TS_ASSERT(testee.get("Foo") != 0);
    TS_ASSERT_EQUALS(*testee.get("Foo"), "bar");
    TS_ASSERT(testee.get("FOO") != 0);
    TS_ASSERT_EQUALS(*testee.get("FOO"), "bar");
    TS_ASSERT(testee.get("foO") != 0);
    TS_ASSERT_EQUALS(*testee.get("foO"), "bar");

    // Case-insensitive replacement
    testee.add("FOO", "what?");
    TS_ASSERT(testee.get("Foo") != 0);
    TS_ASSERT_EQUALS(*testee.get("Foo"), "what?");
}

/** Test load errors. */
void
TestUtilSyntaxKeywordTable::testLoadErrors()
{
    // Sanity check
    {
        afl::io::ConstMemoryStream ms(afl::base::Nothing);
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 0U);
    }

    // Syntax error on one line
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("foo"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Two syntax errors (proves that parsing proceeds)
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("foo\nbar"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 2U);
    }

    // Empty key
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("=x"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Bad block
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("{"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Bad block
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("{x"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Bad block closer
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("}x"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Bad block closer
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("x}"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Badly-placed block closer (missing opener)
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("}"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }

    // Badly-placed block closer (missing opener)
    {
        util::syntax::KeywordTable tab;
        afl::io::ConstMemoryStream ms(afl::string::toBytes("x {\na=b\n}\n}"));
        afl::test::LogListener c;
        tab.load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);

        const String_t* p = tab.get("x.a");
        TS_ASSERT(p);
        TS_ASSERT_EQUALS(*p, "b");
    }

    // Bad reference
    {
        afl::io::ConstMemoryStream ms(afl::string::toBytes("a = 1\nb = $a\nx = $y\n"));
        afl::test::LogListener c;
        util::syntax::KeywordTable().load(ms, c);
        TS_ASSERT_EQUALS(c.getNumMessages(), 1U);
    }
}

/** Test load success. */
void
TestUtilSyntaxKeywordTable::testLoad()
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
    TS_ASSERT_EQUALS(c.getNumMessages(), 0U);

    // Verify content
    TS_ASSERT(testee.get("a") == 0);
    TS_ASSERT(testee.get("; a") == 0);

    TS_ASSERT(testee.get("b") != 0);
    TS_ASSERT_EQUALS(*testee.get("b"), "bar");

    TS_ASSERT(testee.get("c") != 0);
    TS_ASSERT_EQUALS(*testee.get("c"), "baz");

    TS_ASSERT(testee.get("c.x") != 0);
    TS_ASSERT_EQUALS(*testee.get("c.x"), "iks again");  // overwritten by subsequent assignment

    TS_ASSERT(testee.get("d") != 0);
    TS_ASSERT_EQUALS(*testee.get("d"), "baz");

    TS_ASSERT(testee.get("d.x") != 0);
    TS_ASSERT_EQUALS(*testee.get("d.x"), "iks");  // copied from original value before it is overwritten

    TS_ASSERT(testee.get("e") == 0);

    TS_ASSERT(testee.get("e.a") != 0);
    TS_ASSERT_EQUALS(*testee.get("e.a"), "eee");

    TS_ASSERT(testee.get("e.b") != 0);
    TS_ASSERT_EQUALS(*testee.get("e.b"), "fff");

    TS_ASSERT(testee.get("e.c") != 0);
    TS_ASSERT_EQUALS(*testee.get("e.c"), "bar");

    TS_ASSERT(testee.get("f") != 0);
    TS_ASSERT_EQUALS(*testee.get("f"), "1");

    TS_ASSERT(testee.get("c.d.e.f.g") != 0);
    TS_ASSERT_EQUALS(*testee.get("c.d.e.f.g"), "gg");
}

