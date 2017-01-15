/**
  *  \file u/t_util_string.cpp
  *  \brief Test for util::String
  */

#include "util/string.hpp"

#include "t_util.hpp"

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
        { "   x", 3 },
        { "   -x", 4 },
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
TestUtilstring::testFormatName()
{
    TS_ASSERT_EQUALS(util::formatName("FOO"), "Foo");
    TS_ASSERT_EQUALS(util::formatName("FOO.BAR"), "Foo.Bar");
    TS_ASSERT_EQUALS(util::formatName("LOC.X"), "Loc.X");
    TS_ASSERT_EQUALS(util::formatName("CC$FOO"), "Cc$Foo");
    TS_ASSERT_EQUALS(util::formatName("AA3BB"), "Aa3Bb");
}
