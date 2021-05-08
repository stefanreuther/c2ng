/**
  *  \file u/t_util_string.cpp
  *  \brief Test for util::String
  */

#include "util/string.hpp"

#include "t_util.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test util::stringMatch. */
void
TestUtilString::testStringMatch()
{
    TS_ASSERT(util::stringMatch("ENglish", "english"));
    TS_ASSERT(util::stringMatch("ENglish", "en"));
    TS_ASSERT(util::stringMatch("ENglish", "eng"));
    TS_ASSERT(util::stringMatch("ENglish", "ENGLISH"));
    TS_ASSERT(!util::stringMatch("ENglish", "e"));

    TS_ASSERT(util::stringMatch("ENGLISH", "english"));
    TS_ASSERT(!util::stringMatch("ENGLISH", "englis"));
    TS_ASSERT(!util::stringMatch("ENGLISH", "en"));
}

/** Test util::parseRange. */
void
TestUtilString::testParseRange()
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
        TSM_ASSERT(success_cases[i].value, util::parseRange(success_cases[i].value, min, max, pos));
        TSM_ASSERT_EQUALS(success_cases[i].value, success_cases[i].min, min);
        TSM_ASSERT_EQUALS(success_cases[i].value, success_cases[i].max, max);
    }

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
        TSM_ASSERT(failure_cases[i].value, !util::parseRange(failure_cases[i].value, min, max, pos));
        TSM_ASSERT_EQUALS(failure_cases[i].value, failure_cases[i].pos, pos);
    }
}

/** Test util::parsePlayerCharacter. */
void
TestUtilString::testParsePlayer()
{
    int id;
    TS_ASSERT(util::parsePlayerCharacter('0', id));
    TS_ASSERT_EQUALS(id, 0);

    TS_ASSERT(util::parsePlayerCharacter('1', id));
    TS_ASSERT_EQUALS(id, 1);

    TS_ASSERT(util::parsePlayerCharacter('2', id));
    TS_ASSERT_EQUALS(id, 2);

    TS_ASSERT(util::parsePlayerCharacter('3', id));
    TS_ASSERT_EQUALS(id, 3);

    TS_ASSERT(util::parsePlayerCharacter('4', id));
    TS_ASSERT_EQUALS(id, 4);

    TS_ASSERT(util::parsePlayerCharacter('5', id));
    TS_ASSERT_EQUALS(id, 5);

    TS_ASSERT(util::parsePlayerCharacter('6', id));
    TS_ASSERT_EQUALS(id, 6);

    TS_ASSERT(util::parsePlayerCharacter('7', id));
    TS_ASSERT_EQUALS(id, 7);

    TS_ASSERT(util::parsePlayerCharacter('8', id));
    TS_ASSERT_EQUALS(id, 8);

    TS_ASSERT(util::parsePlayerCharacter('9', id));
    TS_ASSERT_EQUALS(id, 9);

    TS_ASSERT(util::parsePlayerCharacter('a', id));
    TS_ASSERT_EQUALS(id, 10);

    TS_ASSERT(util::parsePlayerCharacter('A', id));
    TS_ASSERT_EQUALS(id, 10);

    TS_ASSERT(util::parsePlayerCharacter('b', id));
    TS_ASSERT_EQUALS(id, 11);

    TS_ASSERT(util::parsePlayerCharacter('B', id));
    TS_ASSERT_EQUALS(id, 11);

    TS_ASSERT(util::parsePlayerCharacter('c', id));
    TS_ASSERT_EQUALS(id, 12);

    TS_ASSERT(util::parsePlayerCharacter('C', id));
    TS_ASSERT_EQUALS(id, 12);

    TS_ASSERT(util::parsePlayerCharacter('Q', id));
    TS_ASSERT_EQUALS(id, 26);

    TS_ASSERT(!util::parsePlayerCharacter(' ', id));

    TS_ASSERT(util::parsePlayerCharacter('X', id));
    TS_ASSERT_EQUALS(id, 33);
}

/** Test util::formatOptions. */
void
TestUtilString::testFormatOptions()
{
    // Trivial cases
    TS_ASSERT_EQUALS(util::formatOptions(""),          "");
    TS_ASSERT_EQUALS(util::formatOptions("-a\tfoo\n"), "  -a   foo\n");

    // Not-so-trivial cases
    TS_ASSERT_EQUALS(util::formatOptions("-a\tfoo\n"
                                         "-foo\tbar\n"
                                         "-bar\tbaz\n"
                                         "-help\thelp!\n"),
                     "  -a      foo\n"
                     "  -foo    bar\n"
                     "  -bar    baz\n"
                     "  -help   help!\n");
    TS_ASSERT_EQUALS(util::formatOptions("Heading:\n"
                                         "-option\tinfo\n"
                                         "\n"
                                         "Another heading:\n"
                                         "-more\toption\n"),
                     "Heading:\n"
                     "  -option   info\n"
                     "\n"
                     "Another heading:\n"
                     "  -more     option\n");

    TS_ASSERT_EQUALS(util::formatOptions("-foo\twhoops, forgot the newline"),
                     "  -foo   whoops, forgot the newline");

    TS_ASSERT_EQUALS(util::formatOptions("-foo\tfirst line\n\tsecond line\n"),
                     "  -foo   first line\n"
                     "         second line\n");
}

/** Test util::formatName. */
void
TestUtilString::testFormatName()
{
    TS_ASSERT_EQUALS(util::formatName("FOO"), "Foo");
    TS_ASSERT_EQUALS(util::formatName("FOO.BAR"), "Foo.Bar");
    TS_ASSERT_EQUALS(util::formatName("LOC.X"), "Loc.X");
    TS_ASSERT_EQUALS(util::formatName("CC$FOO"), "Cc$Foo");
    TS_ASSERT_EQUALS(util::formatName("AA3BB"), "Aa3Bb");
}

/** Test util::encodeMimeHeader. */
void
TestUtilString::testEncodeMimeHeader()
{
    TS_ASSERT_EQUALS(util::encodeMimeHeader("hi mom", "UTF-8"), "hi mom");

    // No word wrapping for unencoded stuff!
    const char LOREM[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Duis sem velit, ultrices et, fermentum auctor, rhoncus ut, ligula. Phasellus at purus sed purus cursus iaculis. Suspendisse fermentum. Pellentesque et arcu.";
    TS_ASSERT_EQUALS(util::encodeMimeHeader(LOREM, "us-ascii"), LOREM);
                                            
    // Single unicode characters
    TS_ASSERT_EQUALS(util::encodeMimeHeader("die bl\xc3\xb6""den \xc3\xb6sen", "UTF-8"), "die =?UTF-8?B?YmzDtmRlbg==?= =?UTF-8?B?w7ZzZW4=?=");

    // Many unicode characters
    TS_ASSERT_EQUALS(util::encodeMimeHeader("\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6"
                                            "\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6\xc3\xb6",
                                            "UTF-8"),
                     "=?UTF-8?B?w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bDtsO2w7bD?=\r\n"
                     " =?UTF-8?B?tsO2w7bDtsO2w7bDtsO2w7bDtg==?=");
}

/** Test parseBooleanValue(). */
void
TestUtilString::testParseBoolean()
{
    bool result;
    TS_ASSERT(util::parseBooleanValue("yes",   result)); TS_ASSERT(result);
    TS_ASSERT(util::parseBooleanValue("YES",   result)); TS_ASSERT(result);
    TS_ASSERT(util::parseBooleanValue("y",     result)); TS_ASSERT(result);
    TS_ASSERT(util::parseBooleanValue("true",  result)); TS_ASSERT(result);
    TS_ASSERT(util::parseBooleanValue("1",     result)); TS_ASSERT(result);
    TS_ASSERT(util::parseBooleanValue("0001",  result)); TS_ASSERT(result);
    TS_ASSERT(util::parseBooleanValue(" 1 ",   result)); TS_ASSERT(result);

    TS_ASSERT(util::parseBooleanValue("no",    result)); TS_ASSERT(!result);
    TS_ASSERT(util::parseBooleanValue("NO",    result)); TS_ASSERT(!result);
    TS_ASSERT(util::parseBooleanValue("n",     result)); TS_ASSERT(!result);
    TS_ASSERT(util::parseBooleanValue("false", result)); TS_ASSERT(!result);
    TS_ASSERT(util::parseBooleanValue("0",     result)); TS_ASSERT(!result);
    TS_ASSERT(util::parseBooleanValue("00000", result)); TS_ASSERT(!result);
    TS_ASSERT(util::parseBooleanValue("  0 ",  result)); TS_ASSERT(!result);

    TS_ASSERT(!util::parseBooleanValue("-1",   result));
    TS_ASSERT(!util::parseBooleanValue("none", result));
    TS_ASSERT(!util::parseBooleanValue("1000", result));
    TS_ASSERT(!util::parseBooleanValue("",     result));
    TS_ASSERT(!util::parseBooleanValue(" ",    result));
}

/** Test encodeHtml(). */
void
TestUtilString::testEncodeHtml()
{
    TS_ASSERT_EQUALS(util::encodeHtml("", false), "");
    TS_ASSERT_EQUALS(util::encodeHtml("", true),  "");

    TS_ASSERT_EQUALS(util::encodeHtml("hi mom", false), "hi mom");
    TS_ASSERT_EQUALS(util::encodeHtml("hi mom", true),  "hi mom");

    TS_ASSERT_EQUALS(util::encodeHtml("vector<int>& a", false), "vector&lt;int&gt;&amp; a");
    TS_ASSERT_EQUALS(util::encodeHtml("vector<int>& a", true),  "vector&lt;int&gt;&amp; a");

    TS_ASSERT_EQUALS(util::encodeHtml("say \"Qapla'\"", false), "say &quot;Qapla&#39;&quot;");
    TS_ASSERT_EQUALS(util::encodeHtml("say \"Qapla'\"", true),  "say &quot;Qapla&#39;&quot;");

    TS_ASSERT_EQUALS(util::encodeHtml("\xc3\xb6\xE2\x9C\x97X", false), "&#246;&#10007;X");
    TS_ASSERT_EQUALS(util::encodeHtml("\xc3\xb6\xE2\x9C\x97X", true),  "\xc3\xb6\xE2\x9C\x97X");
}

/** Test addTrailingCharacter / removeTrailingCharacter. */
void
TestUtilString::testTrailing()
{
    // Add
    String_t s;
    util::addTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, ",");
    util::addTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, ",");
    s += 'a';
    util::addTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, ",a,");

    // Remove
    util::removeTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, ",a");
    util::removeTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, ",a");

    s = ",";
    util::removeTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, "");
    util::removeTrailingCharacter(s, ',');
    TS_ASSERT_EQUALS(s, "");
}

/** Test strCollate. */
void
TestUtilString::testCollate()
{
    using util::strCollate;

    TS_ASSERT_EQUALS(strCollate("", ""), 0);
    TS_ASSERT_EQUALS(strCollate("a10b", "a10b"), 0);

    TS_ASSERT(strCollate("1", "2") < 0);
    TS_ASSERT(strCollate("10", "2") > 0);
    TS_ASSERT(strCollate("0010", "002") > 0);
    TS_ASSERT(strCollate("001", "1") < 0);
    TS_ASSERT(strCollate("0010", "000002") > 0);
    TS_ASSERT(strCollate("a0070", "a000070") > 0);
    TS_ASSERT(strCollate("1.5", "1.10") < 0);
    TS_ASSERT(strCollate("a", "A") > 0);
    TS_ASSERT(strCollate("a1", "A5") < 0);
    TS_ASSERT(strCollate("gen1.dat", "gen10.dat") < 0);
    TS_ASSERT(strCollate("gen2.dat", "gen10.dat") < 0);
    TS_ASSERT(strCollate("bla", "blah") < 0);
    TS_ASSERT(strCollate("bar", "baz") < 0);

    TS_ASSERT(strCollate("2", "1") > 0);
    TS_ASSERT(strCollate("2", "10") < 0);
    TS_ASSERT(strCollate("002", "0010") < 0);
    TS_ASSERT(strCollate("1", "001") > 0);
    TS_ASSERT(strCollate("000002", "0010") < 0);
    TS_ASSERT(strCollate("a000070", "a0070") < 0);
    TS_ASSERT(strCollate("1.10", "1.5") > 0);
    TS_ASSERT(strCollate("A", "a") < 0);
    TS_ASSERT(strCollate("A5", "a1") > 0);
    TS_ASSERT(strCollate("gen10.dat", "gen1.dat") > 0);
    TS_ASSERT(strCollate("gen10.dat", "gen2.dat") > 0);
    TS_ASSERT(strCollate("blah", "bla") > 0);
    TS_ASSERT(strCollate("baz", "bar") > 0);
}

/** Test formatAge. */
void
TestUtilString::testFormatAge()
{
    using util::formatAge;
    afl::string::NullTranslator tx;

    TS_ASSERT_EQUALS(formatAge(100, 90, tx), "10 turns ago");
    TS_ASSERT_EQUALS(formatAge(100, 99, tx), "previous turn");
    TS_ASSERT_EQUALS(formatAge(100, 100, tx), "current turn");
    TS_ASSERT_EQUALS(formatAge(100, 777, tx), "turn 777");
}

