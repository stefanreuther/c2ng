/**
  *  \file u/t_util_stringparser.cpp
  *  \brief Test for util::StringParser
  */

#include "util/stringparser.hpp"

#include "t_util.hpp"

/** Some tests. */
void
TestUtilStringParser::testIt()
{
    // Trivial case
    int n = 0;
    {
        util::StringParser p("");
        TS_ASSERT(p.parseString(""));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT(!p.parseString("x"));
        TS_ASSERT(!p.parseInt(n));
        TS_ASSERT_EQUALS(p.getRemainder(), "");
    }

    // Regular case
    {
        util::StringParser p("13a");
        TS_ASSERT_EQUALS(p.getRemainder(), "13a");
        TS_ASSERT(p.parseInt(n));
        TS_ASSERT(p.parseString("a"));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(n, 13);
        TS_ASSERT_EQUALS(p.getRemainder(), "");
    }

    // parseChar
    {
        util::StringParser p("xyz");
        TS_ASSERT( p.parseChar('x'));
        TS_ASSERT(!p.parseChar('a'));
        TS_ASSERT( p.parseChar('y'));
        TS_ASSERT_EQUALS(p.getRemainder(), "z");
        TS_ASSERT( p.parseChar('z'));
        TS_ASSERT(!p.parseChar('z'));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(p.getRemainder(), "");
    }

    // parseDelim
    {
        String_t tmp;
        util::StringParser p("abc:xyz");
        TS_ASSERT(p.parseDelim(":", tmp));
        TS_ASSERT_EQUALS(tmp, "abc");

        TS_ASSERT(p.parseDelim(":", tmp));   // we did not skip the ':' yet
        TS_ASSERT_EQUALS(tmp, "");
        TS_ASSERT_EQUALS(p.getRemainder(), ":xyz");

        TS_ASSERT(p.parseChar(':'));

        TS_ASSERT(p.parseDelim(":", tmp));
        TS_ASSERT_EQUALS(tmp, "xyz");
        TS_ASSERT(p.parseEnd());
    }
}
