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
        TS_ASSERT_EQUALS(p.getPosition(), 0U);
    }

    // Regular case
    {
        util::StringParser p("13a");
        TS_ASSERT_EQUALS(p.getRemainder(), "13a");
        TS_ASSERT(p.parseInt(n));
        TS_ASSERT_EQUALS(p.getPosition(), 2U);
        TS_ASSERT(p.parseString("a"));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT_EQUALS(n, 13);
        TS_ASSERT_EQUALS(p.getRemainder(), "");
        TS_ASSERT_EQUALS(p.getPosition(), 3U);
    }

    // consumeCharacter
    {
        util::StringParser p("13a");
        TS_ASSERT_EQUALS(p.getRemainder(), "13a");
        TS_ASSERT(p.consumeCharacter());
        TS_ASSERT(p.parseInt(n));
        TS_ASSERT_EQUALS(p.getPosition(), 2U);
        TS_ASSERT(p.parseString("a"));
        TS_ASSERT(p.parseEnd());
        TS_ASSERT(!p.consumeCharacter());
        TS_ASSERT_EQUALS(n, 3);
        TS_ASSERT_EQUALS(p.getRemainder(), "");
        TS_ASSERT_EQUALS(p.getPosition(), 3U);
    }

    // parseCharacter
    {
        util::StringParser p("xyz");
        TS_ASSERT( p.parseCharacter('x'));
        TS_ASSERT(!p.parseCharacter('a'));
        TS_ASSERT( p.parseCharacter('y'));
        TS_ASSERT_EQUALS(p.getRemainder(), "z");
        TS_ASSERT( p.parseCharacter('z'));
        TS_ASSERT(!p.parseCharacter('z'));
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

        TS_ASSERT(p.parseCharacter(':'));

        TS_ASSERT(p.parseDelim(":", tmp));
        TS_ASSERT_EQUALS(tmp, "xyz");
        TS_ASSERT(p.parseEnd());
    }

    // parseDelimGreedy behaves like parseDelim
    {
        String_t tmp;
        util::StringParser p("abc:xyz");
        TS_ASSERT(p.parseDelimGreedy(":", tmp));
        TS_ASSERT_EQUALS(tmp, "abc");

        TS_ASSERT(p.parseDelimGreedy(":", tmp));   // we did not skip the ':' yet
        TS_ASSERT_EQUALS(tmp, "");
        TS_ASSERT_EQUALS(p.getRemainder(), ":xyz");

        TS_ASSERT(p.parseCharacter(':'));

        TS_ASSERT(p.parseDelimGreedy(":", tmp));
        TS_ASSERT_EQUALS(tmp, "xyz");
        TS_ASSERT(p.parseEnd());
    }

    // parseDelim with multiple delimiters
    {
        String_t tmp;
        util::StringParser p("a.b:c:d.e");
        TS_ASSERT(p.parseDelim(":.", tmp));
        TS_ASSERT_EQUALS(tmp, "a");

        TS_ASSERT(p.parseDelim(":.", tmp));   // we did not skip the '.' yet
        TS_ASSERT_EQUALS(tmp, "");
        TS_ASSERT_EQUALS(p.getRemainder(), ".b:c:d.e");
    }

    // parseDelimGreedy with multiple delimiters
    {
        String_t tmp;
        util::StringParser p("a.b:c:d.e");
        TS_ASSERT(p.parseDelimGreedy(":.", tmp));
        TS_ASSERT_EQUALS(tmp, "a.b:c:d");

        TS_ASSERT(p.parseDelimGreedy(":.", tmp));   // we did not skip the '.' yet
        TS_ASSERT_EQUALS(tmp, "");
        TS_ASSERT_EQUALS(p.getRemainder(), ".e");
    }

    // Numbers
    {
        util::StringParser p("1 -1 +1 99 -99 +99");
        int a = 0, b = 0, c = 0;
        int64_t d = 0, e = 0, f = 0;
        TS_ASSERT(p.parseInt(a));
        TS_ASSERT(p.parseInt(b));
        TS_ASSERT(p.parseInt(c));
        TS_ASSERT(p.parseInt64(d));
        TS_ASSERT(p.parseInt64(e));
        TS_ASSERT(p.parseInt64(f));
        TS_ASSERT_EQUALS(a, 1);
        TS_ASSERT_EQUALS(b, -1);
        TS_ASSERT_EQUALS(c, 1);
        TS_ASSERT_EQUALS(d, 99);
        TS_ASSERT_EQUALS(e, -99);
        TS_ASSERT_EQUALS(f, 99);
    }
}
