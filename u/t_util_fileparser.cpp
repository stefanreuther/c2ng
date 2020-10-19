/**
  *  \file u/t_util_fileparser.cpp
  *  \brief Test for util::FileParser
  */

#include <vector>
#include "util/fileparser.hpp"

#include "t_util.hpp"
#include "afl/string/format.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/test/assert.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"

namespace {
    using afl::string::Format;
    class TesterMock : public util::FileParser, public afl::test::CallReceiver {
     public:
        TesterMock(const afl::test::Assert& a)
            : FileParser("#"),
              CallReceiver(a)
            { }
        virtual void handleLine(const String_t& fileName, int lineNr, String_t line)
            { checkCall(Format("handleLine(%s,%d,%s)", fileName, lineNr, line)); }
        virtual void handleIgnoredLine(const String_t& fileName, int lineNr, String_t line)
            { checkCall(Format("handleIgnoredLine(%s,%d,%s)", fileName, lineNr, line)); }
    };
}

/** Interface test. */
void
TestUtilFileParser::testInterface()
{
    class Tester : public util::FileParser {
     public:
        Tester()
            : FileParser(";")
            { }
        virtual void handleLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
        virtual void handleIgnoredLine(const String_t& /*fileName*/, int /*lineNr*/, String_t /*line*/)
            { }
    };
    Tester t;
}

/** Simple functionality test. */
void
TestUtilFileParser::testIt()
{
    TesterMock t("testIt");
    t.expectCall("handleLine(<memory>,1,first)");
    t.expectCall("handleIgnoredLine(<memory>,2,   #comment)");
    t.expectCall("handleIgnoredLine(<memory>,3,# another comment)");
    t.expectCall("handleLine(<memory>,4,not # comment)");
    t.expectCall("handleIgnoredLine(<memory>,5,)");
    t.expectCall("handleLine(<memory>,6,final)");

    afl::io::ConstMemoryStream ms(afl::string::toBytes("first\n"
                                                       "   #comment\n"
                                                       "# another comment\n"
                                                       "not # comment\n"
                                                       "\n"
                                                       "final"));
    TS_ASSERT_EQUALS(ms.getName(), "<memory>");  // not contractual, but embedded in above expectations
    t.parseFile(ms);
    t.checkFinish();
}

/** Functionality test using charset. */
void
TestUtilFileParser::testCharset()
{
    TesterMock t("testCharset");
    t.expectCall("handleLine(<memory>,1,x\xc3\x97y)");
    t.setCharsetNew(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));

    afl::io::ConstMemoryStream ms(afl::string::toBytes("x\xd7y\n"));
    t.parseFile(ms);
    t.checkFinish();
}

/** Test trimComments(). */
void
TestUtilFileParser::testTrimComments()
{
    TesterMock t("testTrimComments");

    {
        String_t s = "a#b#c";
        t.trimComments(s);
        TS_ASSERT_EQUALS(s, "a");
    }

    {
        String_t s = "foo # bar";
        t.trimComments(s);
        TS_ASSERT_EQUALS(s, "foo");
    }

    {
        String_t s = "    \t   # hi!";
        t.trimComments(s);
        TS_ASSERT_EQUALS(s, "");
    }
}
