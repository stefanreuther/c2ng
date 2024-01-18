/**
  *  \file test/util/stringparsertest.cpp
  *  \brief Test for util::StringParser
  */

#include "util/stringparser.hpp"
#include "afl/test/testrunner.hpp"

// Trivial case
AFL_TEST("util.StringParser:empty", a)
{
    int n = 0;
    util::StringParser p("");
    a.check     ("01. parseString",  p.parseString(""));
    a.check     ("02. parseEnd",     p.parseEnd());
    a.check     ("03. parseString", !p.parseString("x"));
    a.check     ("04. parseInt",    !p.parseInt(n));
    a.checkEqual("05. getRemainder", p.getRemainder(), "");
    a.checkEqual("06. getPosition",  p.getPosition(), 0U);
}

// Regular case
AFL_TEST("util.StringParser:normal", a)
{
    int n = 0;
    util::StringParser p("13a");
    a.checkEqual("01. getRemainder", p.getRemainder(), "13a");
    a.check     ("02. parseInt",     p.parseInt(n));
    a.checkEqual("03. getPosition",  p.getPosition(), 2U);
    a.check     ("04. parseString",  p.parseString("a"));
    a.check     ("05. parseEnd",     p.parseEnd());
    a.checkEqual("06. value",        n, 13);
    a.checkEqual("07. getRemainder", p.getRemainder(), "");
    a.checkEqual("08. getPosition",  p.getPosition(), 3U);
}

// consumeCharacter
AFL_TEST("util.StringParser:consumeCharacter", a)
{
    int n = 0;
    util::StringParser p("13a");
    a.checkEqual("01. getRemainder",      p.getRemainder(), "13a");
    a.check     ("02. consumeCharacter",  p.consumeCharacter());
    a.check     ("03. parseInt",          p.parseInt(n));
    a.checkEqual("04. getPosition",       p.getPosition(), 2U);
    a.check     ("05. parseString",       p.parseString("a"));
    a.check     ("06. parseEnd",          p.parseEnd());
    a.check     ("07. consumeCharacter", !p.consumeCharacter());
    a.checkEqual("08. value",             n, 3);
    a.checkEqual("09. getRemainder",      p.getRemainder(), "");
    a.checkEqual("10. getPosition",       p.getPosition(), 3U);
}

// parseCharacter
AFL_TEST("util.StringParser:parseCharacter", a)
{
    util::StringParser p("xyz");
    a.check     ("01. parseCharacter",  p.parseCharacter('x'));
    a.check     ("02. parseCharacter", !p.parseCharacter('a'));
    a.check     ("03. parseCharacter",  p.parseCharacter('y'));
    a.checkEqual("04. getRemainder",    p.getRemainder(), "z");
    a.check     ("05. parseCharacter",  p.parseCharacter('z'));
    a.check     ("06. parseCharacter", !p.parseCharacter('z'));
    a.check     ("07. parseEnd",        p.parseEnd());
    a.checkEqual("08. getRemainder",    p.getRemainder(), "");
}

// parseDelim
AFL_TEST("util.StringParser:parseDelim", a)
{
    String_t tmp;
    util::StringParser p("abc:xyz");
    a.check     ("01. parseDelim",     p.parseDelim(":", tmp));
    a.checkEqual("02. value",          tmp, "abc");

    a.check     ("11. parseDelim",     p.parseDelim(":", tmp));   // we did not skip the ':' yet
    a.checkEqual("12. value",          tmp, "");
    a.checkEqual("13. getRemainder",   p.getRemainder(), ":xyz");

    a.check     ("21. parseCharacter", p.parseCharacter(':'));

    a.check     ("31. parseDelim",     p.parseDelim(":", tmp));
    a.checkEqual("32. value",          tmp, "xyz");
    a.check     ("33. parseEnd",       p.parseEnd());
}

// parseDelimGreedy behaves like parseDelim
AFL_TEST("util.StringParser:parseDelimGreedy", a)
{
    String_t tmp;
    util::StringParser p("abc:xyz");
    a.check     ("01. parseDelimGreedy", p.parseDelimGreedy(":", tmp));
    a.checkEqual("02. value",            tmp, "abc");

    a.check     ("11. parseDelimGreedy", p.parseDelimGreedy(":", tmp));   // we did not skip the ':' yet
    a.checkEqual("12. value",            tmp, "");
    a.checkEqual("13. getRemainder",     p.getRemainder(), ":xyz");

    a.check     ("21. parseCharacter",   p.parseCharacter(':'));

    a.check     ("31. parseDelimGreedy", p.parseDelimGreedy(":", tmp));
    a.checkEqual("32. value",            tmp, "xyz");
    a.check     ("33. parseEnd",         p.parseEnd());
}

// parseDelim with multiple delimiters
AFL_TEST("util.StringParser:parseDelim:multiple-delimiters", a)
{
    String_t tmp;
    util::StringParser p("a.b:c:d.e");
    a.check     ("01. parseDelim",   p.parseDelim(":.", tmp));
    a.checkEqual("02. value",        tmp, "a");

    a.check     ("11. parseDelim",   p.parseDelim(":.", tmp));   // we did not skip the '.' yet
    a.checkEqual("12. value",        tmp, "");
    a.checkEqual("13. getRemainder", p.getRemainder(), ".b:c:d.e");
}

// parseDelimGreedy with multiple delimiters
AFL_TEST("util.StringParser:parseDelimGreedy:multiple-delimiters", a)
{
    String_t tmp;
    util::StringParser p("a.b:c:d.e");
    a.check     ("01. parseDelimGreedy", p.parseDelimGreedy(":.", tmp));
    a.checkEqual("02. value",            tmp, "a.b:c:d");

    a.check     ("11. parseDelimGreedy", p.parseDelimGreedy(":.", tmp));   // we did not skip the '.' yet
    a.checkEqual("12. value",            tmp, "");
    a.checkEqual("13. getRemainder",     p.getRemainder(), ".e");
}

// Numbers
AFL_TEST("util.StringParser:parseInt", a)
{
    util::StringParser p("1 -1 +1 99 -99 +99");
    int ia = 0, ib = 0, ic = 0;
    int64_t id = 0, ie = 0, jf = 0;
    a.check("01. parseInt",   p.parseInt(ia));
    a.check("02. parseInt",   p.parseInt(ib));
    a.check("03. parseInt",   p.parseInt(ic));
    a.check("04. parseInt64", p.parseInt64(id));
    a.check("05. parseInt64", p.parseInt64(ie));
    a.check("06. parseInt64", p.parseInt64(jf));

    a.checkEqual("11. value", ia, 1);
    a.checkEqual("12. value", ib, -1);
    a.checkEqual("13. value", ic, 1);
    a.checkEqual("14. value", id, 99);
    a.checkEqual("15. value", ie, -99);
    a.checkEqual("16. value", jf, 99);
}

// Case-insensitivity
AFL_TEST("util.StringParser:parseCaseInsensitiveString", a)
{
    util::StringParser p("hello!");
    a.check("01. parseString",               !p.parseString("hElLo"));
    a.check("02. parseCaseInsensitiveString", p.parseCaseInsensitiveString("hElLo"));
    a.check("03. parseCharacter",             p.parseCharacter('!'));
    a.check("04. parseEnd",                   p.parseEnd());
}

// Case-insensitivity
AFL_TEST("util.StringParser:parseCaseInsensitiveString:mismatch", a)
{
    util::StringParser p("hello!");
    a.check("01. parseCaseInsensitiveString", !p.parseCaseInsensitiveString("hAlLo"));
    a.check("02. parseString",                 p.parseString("hello"));
    a.check("03. parseCharacter",              p.parseCharacter('!'));
    a.check("04. parseEnd",                    p.parseEnd());
}
