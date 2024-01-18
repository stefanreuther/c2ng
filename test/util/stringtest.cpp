/**
  *  \file test/util/stringtest.cpp
  *  \brief Test for util::String
  */

#include "util/string.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test util::stringMatch. */
AFL_TEST("util.String:stringMatch", a)
{
    a.check("01", util::stringMatch("ENglish", "english"));
    a.check("02", util::stringMatch("ENglish", "en"));
    a.check("03", util::stringMatch("ENglish", "eng"));
    a.check("04", util::stringMatch("ENglish", "ENGLISH"));
    a.check("05", !util::stringMatch("ENglish", "e"));

    a.check("11", util::stringMatch("ENGLISH", "english"));
    a.check("12", !util::stringMatch("ENGLISH", "englis"));
    a.check("13", !util::stringMatch("ENGLISH", "en"));
}

/** Test util::parseRange. */
AFL_TEST("util.String:parseRange:success", a)
{
    // ex UtilMiscTestSuite::testParseRange
    int min, max;
    size_t pos;

    // Success cases
    static const struct {
        const char* value;
        int min, max;
    } success_cases[] = {
        // standard cases
        { "42", 42, 42 },
        { "42-", 42, 100 },
        { "23-42", 23, 42 },
        { "23-142", 23, 142 },

        // various spaces
        { "  42", 42, 42 },
        { "  42-", 42, 100 },
        { "  23-42", 23, 42 },
        { "  23-142", 23, 142 },
        { "42  ", 42, 42 },
        { "42-  ", 42, 100 },
        { "23-42  ", 23, 42 },
        { "23-142  ", 23, 142 },
        { "42  -", 42, 100 },
        { "23  -42", 23, 42 },
        { "23  -142", 23, 142 },
        { "23  -  42", 23, 42 },
        { "23  -  142", 23, 142 },
        { "42  -  ", 42, 100 },
        { "23  -42  ", 23, 42 },
        { "23  -142  ", 23, 142 },
        { "23  -  42  ", 23, 42 },
        { "23  -  142  ", 23, 142 },
        { "1--2", 1, -2 }
    };

    for (size_t i = 0; i < sizeof(success_cases)/sizeof(success_cases[0]); ++i) {
        min = 0, max = 100;
        a(success_cases[i].value).check("parseRange", util::parseRange(success_cases[i].value, min, max, pos));
        a(success_cases[i].value).checkEqual("min", success_cases[i].min, min);
        a(success_cases[i].value).checkEqual("max", success_cases[i].max, max);
    }
}

AFL_TEST("util.String:parseRange:failure", a)
{
    int min, max;
    size_t pos;

    // Failure cases
    static const struct {
        const char* value;
        String_t::size_type pos;
    } failure_cases[] = {
        // standard failures
        { "", 0 },
        { "x", 0 },
        { "-", 0 },
        { "-2", 0 },
        { "   x", 0 /* was 3, now 0 because string is entirely invalid */ },
        { "   -x", 0 /* was 4, now 0 because string is entirely invalid */ },
        // { "   -2x", 5 },

        // standard cases
        { "42x", 2 },
        { "42-x", 3 },
        { "23-42x", 5 },

        // various spaces
        { "  42x", 4 },
        { "  42-x", 5 },
        { "  23-42x", 7 },
        { "42  x", 4 },
        { "42-  x", 5 },
        { "23-42  x", 7 },
        { "42  -x", 5 },
        { "23  -42x", 7 },
        { "23  -  42x", 9 },
        { "42  -  x", 7 },
        { "23  -42  x", 9 },
        { "23  -  42  x", 11 },
    };

    for (size_t i = 0; i < sizeof(failure_cases)/sizeof(failure_cases[0]); ++i) {
        min = 0, max = 100;
        a(failure_cases[i].value).check("parseRange", !util::parseRange(failure_cases[i].value, min, max, pos));
        a(failure_cases[i].value).checkEqual("pos", failure_cases[i].pos, pos);
    }
}

/** Test util::parsePlayerCharacter. */
AFL_TEST("util.String:parsePlayerCharacter", a)
{
    int id;
    a.check("01", util::parsePlayerCharacter('0', id));
    a.checkEqual("02", id, 0);

    a.check("11", util::parsePlayerCharacter('1', id));
    a.checkEqual("12", id, 1);

    a.check("21", util::parsePlayerCharacter('2', id));
    a.checkEqual("22", id, 2);

    a.check("31", util::parsePlayerCharacter('3', id));
    a.checkEqual("32", id, 3);

    a.check("41", util::parsePlayerCharacter('4', id));
    a.checkEqual("42", id, 4);

    a.check("51", util::parsePlayerCharacter('5', id));
    a.checkEqual("52", id, 5);

    a.check("61", util::parsePlayerCharacter('6', id));
    a.checkEqual("62", id, 6);

    a.check("71", util::parsePlayerCharacter('7', id));
    a.checkEqual("72", id, 7);

    a.check("81", util::parsePlayerCharacter('8', id));
    a.checkEqual("82", id, 8);

    a.check("91", util::parsePlayerCharacter('9', id));
    a.checkEqual("92", id, 9);

    a.check("101", util::parsePlayerCharacter('a', id));
    a.checkEqual("102", id, 10);

    a.check("111", util::parsePlayerCharacter('A', id));
    a.checkEqual("112", id, 10);

    a.check("121", util::parsePlayerCharacter('b', id));
    a.checkEqual("122", id, 11);

    a.check("131", util::parsePlayerCharacter('B', id));
    a.checkEqual("132", id, 11);

    a.check("141", util::parsePlayerCharacter('c', id));
    a.checkEqual("142", id, 12);

    a.check("151", util::parsePlayerCharacter('C', id));
    a.checkEqual("152", id, 12);

    a.check("161", util::parsePlayerCharacter('Q', id));
    a.checkEqual("162", id, 26);

    a.check("171", !util::parsePlayerCharacter(' ', id));

    a.check("181", util::parsePlayerCharacter('X', id));
    a.checkEqual("182", id, 33);
}

/** Test util::formatOptions. */
AFL_TEST("util.String:formatOptions", a)
{
    // Trivial cases
    a.checkEqual("01", util::formatOptions(""),          "");
    a.checkEqual("02", util::formatOptions("-a\tfoo\n"), "  -a   foo\n");

    // Not-so-trivial cases
    a.checkEqual("11", util::formatOptions("-a\tfoo\n"
                                           "-foo\tbar\n"
                                           "-bar\tbaz\n"
                                           "-help\thelp!\n"),
                 "  -a      foo\n"
                 "  -foo    bar\n"
                 "  -bar    baz\n"
                 "  -help   help!\n");
    a.checkEqual("12", util::formatOptions("Heading:\n"
                                           "-option\tinfo\n"
                                           "\n"
                                           "Another heading:\n"
                                           "-more\toption\n"),
                 "Heading:\n"
                 "  -option   info\n"
                 "\n"
                 "Another heading:\n"
                 "  -more     option\n");

    a.checkEqual("21", util::formatOptions("-foo\twhoops, forgot the newline"),
                 "  -foo   whoops, forgot the newline");

    a.checkEqual("31", util::formatOptions("-foo\tfirst line\n\tsecond line\n"),
                 "  -foo   first line\n"
                 "         second line\n");
}

/** Test util::formatName. */
AFL_TEST("util.String:formatName", a)
{
    a.checkEqual("01", util::formatName("FOO"), "Foo");
    a.checkEqual("02", util::formatName("FOO.BAR"), "Foo.Bar");
    a.checkEqual("03", util::formatName("LOC.X"), "Loc.X");
    a.checkEqual("04", util::formatName("CC$FOO"), "Cc$Foo");
    a.checkEqual("05", util::formatName("AA3BB"), "Aa3Bb");
}

/** Test util::encodeMimeHeader. */
AFL_TEST("util.String:encodeMimeHeader", a)
{
    a.checkEqual("01", util::encodeMimeHeader("hi mom", "UTF-8"), "hi mom");

    // No word wrapping for unencoded stuff!
    const char LOREM[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula. Phasellus at purus sed purus cursus iaculis. Suspendisse fermentum. Pellentesque et arcu.";
    a.checkEqual("11", util::encodeMimeHeader(LOREM, "us-ascii"), LOREM);

    // Single unicode characters
    a.checkEqual("21", util::encodeMimeHeader("die bl\xc3\xb6""den \xc3\xb6sen", "UTF-8"), "die =?UTF-8?B?YmzDtmRlbg==?= =?UTF-8?B?w7ZzZW4=?=");

    // Many unicode characters
    a.checkEqual("31", util::encodeMimeHeader("\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6"
                                              "\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6",
                                              "UTF-8"),
                 "=?UTF-8?B?w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bD?=\r\n"
                 " =?UTF-8?B?tsO2w7bDtsO2w7bDtsO2w7bDtg==?=");
}

/** Test parseBooleanValue(). */
AFL_TEST("util.String:parseBooleanValue", a)
{
    bool result;
    a.check("01", util::parseBooleanValue("yes",   result)); a.check("01", result);
    a.check("02", util::parseBooleanValue("YES",   result)); a.check("02", result);
    a.check("03", util::parseBooleanValue("y",     result)); a.check("03", result);
    a.check("04", util::parseBooleanValue("true",  result)); a.check("04", result);
    a.check("05", util::parseBooleanValue("1",     result)); a.check("05", result);
    a.check("06", util::parseBooleanValue("0001",  result)); a.check("06", result);
    a.check("07", util::parseBooleanValue(" 1 ",   result)); a.check("07", result);

    a.check("11", util::parseBooleanValue("no",    result)); a.check("11", !result);
    a.check("12", util::parseBooleanValue("NO",    result)); a.check("12", !result);
    a.check("13", util::parseBooleanValue("n",     result)); a.check("13", !result);
    a.check("14", util::parseBooleanValue("false", result)); a.check("14", !result);
    a.check("15", util::parseBooleanValue("0",     result)); a.check("15", !result);
    a.check("16", util::parseBooleanValue("00000", result)); a.check("16", !result);
    a.check("17", util::parseBooleanValue("  0 ",  result)); a.check("17", !result);

    a.check("21", !util::parseBooleanValue("-1",   result));
    a.check("22", !util::parseBooleanValue("none", result));
    a.check("23", !util::parseBooleanValue("1000", result));
    a.check("24", !util::parseBooleanValue("",     result));
    a.check("25", !util::parseBooleanValue(" ",    result));
}

/** Test encodeHtml(). */
AFL_TEST("util.String:encodeHtml", a)
{
    a.checkEqual("01", util::encodeHtml("", false), "");
    a.checkEqual("02", util::encodeHtml("", true),  "");

    a.checkEqual("11", util::encodeHtml("hi mom", false), "hi mom");
    a.checkEqual("12", util::encodeHtml("hi mom", true),  "hi mom");

    a.checkEqual("21", util::encodeHtml("vector<int>& a", false), "vector&lt;int&gt;&amp; a");
    a.checkEqual("22", util::encodeHtml("vector<int>& a", true),  "vector&lt;int&gt;&amp; a");

    a.checkEqual("31", util::encodeHtml("say \"Qapla'\"", false), "say &quot;Qapla&#39;&quot;");
    a.checkEqual("32", util::encodeHtml("say \"Qapla'\"", true),  "say &quot;Qapla&#39;&quot;");

    a.checkEqual("41", util::encodeHtml("\xc3\xb6\xE2\x9C\x97X", false), "&#246;&#10007;X");
    a.checkEqual("42", util::encodeHtml("\xc3\xb6\xE2\x9C\x97X", true),  "\xc3\xb6\xE2\x9C\x97X");
}

/** Test addTrailingCharacter / removeTrailingCharacter. */
AFL_TEST("util.String:trailing", a)
{
    // Add
    String_t s;
    util::addTrailingCharacter(s, ',');
    a.checkEqual("01", s, ",");
    util::addTrailingCharacter(s, ',');
    a.checkEqual("02", s, ",");
    s += 'a';
    util::addTrailingCharacter(s, ',');
    a.checkEqual("03", s, ",a,");

    // Remove
    util::removeTrailingCharacter(s, ',');
    a.checkEqual("11", s, ",a");
    util::removeTrailingCharacter(s, ',');
    a.checkEqual("12", s, ",a");

    s = ",";
    util::removeTrailingCharacter(s, ',');
    a.checkEqual("21", s, "");
    util::removeTrailingCharacter(s, ',');
    a.checkEqual("22", s, "");
}

/** Test strCollate. */
AFL_TEST("util.String:strCollate", a)
{
    using util::strCollate;

    a.checkEqual("01", strCollate("", ""), 0);
    a.checkEqual("02", strCollate("a10b", "a10b"), 0);

    a.check("11", strCollate("1", "2") < 0);
    a.check("12", strCollate("10", "2") > 0);
    a.check("13", strCollate("0010", "002") > 0);
    a.check("14", strCollate("001", "1") < 0);
    a.check("15", strCollate("0010", "000002") > 0);
    a.check("16", strCollate("a0070", "a000070") > 0);
    a.check("17", strCollate("1.5", "1.10") < 0);
    a.check("18", strCollate("a", "A") > 0);
    a.check("19", strCollate("a1", "A5") < 0);
    a.check("20", strCollate("gen1.dat", "gen10.dat") < 0);
    a.check("21", strCollate("gen2.dat", "gen10.dat") < 0);
    a.check("22", strCollate("bla", "blah") < 0);
    a.check("23", strCollate("bar", "baz") < 0);

    a.check("31", strCollate("2", "1") > 0);
    a.check("32", strCollate("2", "10") < 0);
    a.check("33", strCollate("002", "0010") < 0);
    a.check("34", strCollate("1", "001") > 0);
    a.check("35", strCollate("000002", "0010") < 0);
    a.check("36", strCollate("a000070", "a0070") < 0);
    a.check("37", strCollate("1.10", "1.5") > 0);
    a.check("38", strCollate("A", "a") < 0);
    a.check("39", strCollate("A5", "a1") > 0);
    a.check("40", strCollate("gen10.dat", "gen1.dat") > 0);
    a.check("41", strCollate("gen10.dat", "gen2.dat") > 0);
    a.check("42", strCollate("blah", "bla") > 0);
    a.check("43", strCollate("baz", "bar") > 0);
}

/** Test formatAge. */
AFL_TEST("util.String:formatAge", a)
{
    using util::formatAge;
    afl::string::NullTranslator tx;

    a.checkEqual("01", formatAge(100, 90, tx), "10 turns ago");
    a.checkEqual("02", formatAge(100, 99, tx), "previous turn");
    a.checkEqual("03", formatAge(100, 100, tx), "current turn");
    a.checkEqual("04", formatAge(100, 777, tx), "turn 777");
}

/** Test strStartsWith. */
AFL_TEST("util.String:strStartsWith", a)
{
    using util::strStartsWith;

    // Long-lived string
    String_t s = "foobar";
    a.check("01", strStartsWith(s, "foo")    == s.c_str() + 3);
    a.check("02", strStartsWith(s, "foobar") == s.c_str() + 6);
    a.check("03", strStartsWith(s, "")       == s.c_str());
    a.checkNull("04", strStartsWith(s, "bar")   );
    a.checkNull("05", strStartsWith(s, "foobarx"));

    // Short-lived string
    a.checkEqual("11", String_t(strStartsWith("foobar", "foo")), "bar");
    a.checkEqual("12", String_t(strStartsWith("foobar", "")),    "foobar");
    a.checkNull("13", strStartsWith("foobar", "bar"));
}

/** Test parseZoomLevel. */
AFL_TEST("util.String:parseZoomLevel", a)
{
    using util::parseZoomLevel;
    int mul = 99, div = 99;

    // Error cases (leave output unmodified)
    a.checkEqual("01", parseZoomLevel("", mul, div), false);
    a.checkEqual("02", parseZoomLevel(":", mul, div), false);
    a.checkEqual("03", parseZoomLevel("/", mul, div), false);
    a.checkEqual("04", parseZoomLevel("4/", mul, div), false);
    a.checkEqual("05", parseZoomLevel("/4", mul, div), false);
    a.checkEqual("06", parseZoomLevel("0/0", mul, div), false);
    a.checkEqual("07", parseZoomLevel("-2/-3", mul, div), false);
    a.checkEqual("08", mul, 99);
    a.checkEqual("09", div, 99);

    // Success cases
    a.checkEqual("11", parseZoomLevel("1", mul, div), true);
    a.checkEqual("12", mul, 1);
    a.checkEqual("13", div, 1);

    a.checkEqual("21", parseZoomLevel("   4  ", mul, div), true);
    a.checkEqual("22", mul, 4);
    a.checkEqual("23", div, 1);

    a.checkEqual("31", parseZoomLevel("2:3", mul, div), true);
    a.checkEqual("32", mul, 2);
    a.checkEqual("33", div, 3);

    a.checkEqual("41", parseZoomLevel(" 5 / 9 ", mul, div), true);
    a.checkEqual("42", mul, 5);
    a.checkEqual("43", div, 9);
}

/** Test formatZoomLevel. */
AFL_TEST("util.String:formatZoomLevel", a)
{
    using util::formatZoomLevel;
    a.checkEqual("01", formatZoomLevel(1, 1), "1");
    a.checkEqual("02", formatZoomLevel(4, 4), "4/4");
    a.checkEqual("03", formatZoomLevel(1, 2), "1/2");
}
