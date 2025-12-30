/**
  *  \file test/util/characternamelisttest.cpp
  *  \brief Test for util::CharacterNameList
  */

#include "util/characternamelist.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"

namespace {
    void prepareSearch(util::CharacterNameList& list)
    {
        const char* FILE =
            "0020 SPACE\n"
            "0045 LATIN CAPITAL LETTER E\n"
            "0065 LATIN SMALL LETTER E\n"
            "00D0 LATIN CAPITAL LETTER ETH\n"
            "00A0 NO-BREAK SPACE\n"
            "2003 EM SPACE\n";
        afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));
        list.loadNames(ms);
    }

    void prepareGenerate(util::CharacterNameList& list)
    {
        const char* NAME_FILE =
            "0041 LATIN CAPITAL LETTER A\n"
            "00C4 LATIN CAPITAL LETTER A WITH DIAERESIS\n"
            "0308 COMBINING DIAERESIS (Dialytika)\n"
            "0391 GREEK CAPITAL LETTER ALPHA\n"
            "0410 CYRILLIC CAPITAL LETTER A\n";
        afl::io::ConstMemoryStream names(afl::string::toBytes(NAME_FILE));
        list.loadNames(names);

        const char* ALIAS_FILE =
            "CYRILLIC ? A                        = LATIN ? A\n"
            "GREEK CAPITAL LETTER ALPHA          = LATIN ? A\n";
        afl::io::ConstMemoryStream alias(afl::string::toBytes(ALIAS_FILE));
        list.loadAliases(alias);
    }

    class GeneratorMock : public util::CharacterNameList::Generator, public afl::test::CallReceiver {
     public:
        GeneratorMock(afl::test::Assert a)
            : CallReceiver(a)
            { }
        bool check(afl::base::Memory<const afl::charset::Unichar_t> set)
            {
                while (const afl::charset::Unichar_t* ch = set.eat()) {
                    checkCall(afl::string::Format("check(%04X)", *ch));
                }
                checkCall("end()");
                return consumeReturnValue<bool>();
            }
    };
}

/** Interface test. */
AFL_TEST_NOARG("util.CharacterNameList:Generator-interface")
{
    class Tester : public util::CharacterNameList::Generator {
     public:
        virtual bool check(afl::base::Memory<const afl::charset::Unichar_t> /*set*/)
            { return false; }
    };
    Tester t;
}

/** Initial status. */
AFL_TEST("util.CharacterNameList:init", a)
{
    util::CharacterNameList testee;
    a.checkEqual("01", testee.getCharacterName(0x001B), "U+001B");
    a.checkEqual("02", testee.getCharacterName(0x0020), "U+0020");
    a.checkEqual("03", testee.getCharacterName(0x0061), "U+0061 a");
    a.checkEqual("04", testee.getCharacterName(0x00D0), "U+00D0");
    a.checkEqual("05", testee.getCharacterName(0xCDEF), "U+CDEF");
    a.checkEqual("06", testee.getCharacterName(0xE142), "U+E142");
}

/** loadNames, getCharacterName, findCharacterByName. */
AFL_TEST("util.CharacterNameList:loadNames", a)
{
    const char* FILE =
        "001B\t<control>\n"
        "\t= ESCAPE\n"
        "0020\tSPACE\n"
        "\t* sometimes considered a control code\n"
        "\t* other space characters: 2000-200A\n"
        "00D0\tLATIN CAPITAL LETTER ETH (Icelandic)\n"
        "\tx (latin small letter eth - 00F0)\n";
    afl::io::ConstMemoryStream ms(afl::string::toBytes(FILE));

    util::CharacterNameList testee;
    testee.addDefault();
    testee.loadNames(ms);

    a.checkEqual("01", testee.getCharacterName(0x001B), "U+001B ESCAPE");
    a.checkEqual("02", testee.getCharacterName(0x0020), "U+0020 SPACE");
    a.checkEqual("03", testee.getCharacterName(0x0061), "U+0061 a");
    a.checkEqual("04", testee.getCharacterName(0x00D0), "U+00D0 LATIN CAPITAL LETTER ETH");
    a.checkEqual("05", testee.getCharacterName(0xCDEF), "U+CDEF");
    a.checkEqual("06", testee.getCharacterName(0xE142), "U+E142 [PCC2] ORNAMENT LEFT");

    a.checkEqual("11", testee.findCharacterByName("SPACE"), 0x0020U);
    a.checkEqual("12", testee.findCharacterByName("OTHER"), 0U);
}

/** searchCharactersByName: "". */
AFL_TEST("util.CharacterNameList:searchCharactersByName:empty", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("", 0xFFFF);
    a.checkEqual("size", list.size(), 0U);
}

/** searchCharactersByName: "e". */
AFL_TEST("util.CharacterNameList:searchCharactersByName:e", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("e", 0xFFFF);
    a.checkEqual("size", list.size(), 6U);
    a.checkEqual("index 0", list[0], 0x0065U);
    a.checkEqual("index 1", list[1], 0x0045U);
    a.checkEqual("index 2", list[2], 0x00D0U);
    a.checkEqual("index 3", list[3], 0x2003U);
    a.checkEqual("index 4", list[4], 0x0020U);
    a.checkEqual("index 5", list[5], 0x00A0U);
}

/** searchCharactersByName: "e", limited search. */
AFL_TEST("util.CharacterNameList:searchCharactersByName:e:limit", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("e", 0x1000);
    a.checkEqual("size", list.size(), 5U);
    a.checkEqual("index 0", list[0], 0x0065U);
    a.checkEqual("index 1", list[1], 0x0045U);
    a.checkEqual("index 2", list[2], 0x00D0U);
    a.checkEqual("index 3", list[3], 0x0020U);
    a.checkEqual("index 4", list[4], 0x00A0U);
}

/** searchCharactersByName: "et" (letter "eth", and "letter"). */
AFL_TEST("util.CharacterNameList:searchCharactersByName:et", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("et", 0xFFFF);
    a.checkEqual("size", list.size(), 3U);
    a.checkEqual("index 0", list[0], 0x00D0U);
    a.checkEqual("index 1", list[1], 0x0045U);
    a.checkEqual("index 2", list[2], 0x0065U);
}

/** searchCharactersByName: "space". */
AFL_TEST("util.CharacterNameList:searchCharactersByName:space", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("space", 0xFFFF);
    a.checkEqual("size", list.size(), 3U);
    a.checkEqual("index 0", list[0], 0x0020U);
    a.checkEqual("index 1", list[1], 0x00A0U);
    a.checkEqual("index 2", list[2], 0x2003U);
}

/** searchCharactersByName: "u+300". */
AFL_TEST("util.CharacterNameList:searchCharactersByName:u+300", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("u+300", 0xFFFF);
    a.checkEqual("size", list.size(), 1U);
    a.checkEqual("index 0", list[0], 0x0300U);
}

/** searchCharactersByName: "300". */
AFL_TEST("util.CharacterNameList:searchCharactersByName:300", a)
{
    util::CharacterNameList testee;
    prepareSearch(testee);

    util::CharacterNameList::CharacterList_t list = testee.searchCharactersByName("300", 0xFFFF);
    a.checkEqual("size", list.size(), 1U);
    a.checkEqual("index 0", list[0], 0x0300U);
}

/** generate: A umlaut. */
AFL_TEST("util.CharacterNameList:generateCharacter:auml", a)
{
    util::CharacterNameList testee;
    prepareGenerate(testee);

    GeneratorMock mock(a);
    mock.expectCall("check(0041)");
    mock.expectCall("check(0308)");
    mock.expectCall("end()");
    mock.provideReturnValue(true);
    a.check("generate result", testee.generateCharacter(0x00C4, mock));
    mock.checkFinish();
}

/** generate: A umlaut, but generator returns false. */
AFL_TEST("util.CharacterNameList:generateCharacter:auml:false", a)
{
    util::CharacterNameList testee;
    prepareGenerate(testee);

    GeneratorMock mock(a);
    mock.expectCall("check(0041)");
    mock.expectCall("check(0308)");
    mock.expectCall("end()");
    mock.provideReturnValue(false);
    a.check("generate result", !testee.generateCharacter(0x00C4, mock));
    mock.checkFinish();
}

/** generate: cyrillic a. */
AFL_TEST("util.CharacterNameList:generateCharacter:cyrillic", a)
{
    util::CharacterNameList testee;
    prepareGenerate(testee);

    GeneratorMock mock(a);
    mock.expectCall("check(0041)");
    mock.expectCall("end()");
    mock.provideReturnValue(true);
    a.check("generate result", testee.generateCharacter(0x0410, mock));
    mock.checkFinish();
}

/** generate: unprepared. */
AFL_TEST("util.CharacterNameList:generateCharacter:unprepared", a)
{
    util::CharacterNameList testee;

    GeneratorMock mock(a);
    a.check("generate result", !testee.generateCharacter(0x0410, mock));
    mock.checkFinish();
}
